/*
 * Item 4 - Multiplicacao de matrizes PARALELA com Pthreads.
 *
 * Divide o calculo da matriz resultado C entre T threads. A particao e feita
 * por FAIXAS CONTIGUAS DE LINHAS: cada thread fica responsavel por um intervalo
 * [linha_inicial, linha_final) de linhas de C, calculando-as por completo.
 *
 * Por que essa divisao e segura e eficiente:
 *   - Cada thread escreve em linhas DIFERENTES de C (posicoes mutuamente
 *     exclusivas), entao NAO ha necessidade de mutex/lock: o laco opera 100%
 *     livre de contencao.
 *   - Como cada linha de C ocupa N*8 bytes (muito maior que uma linha de cache
 *     de 64 bytes), a fronteira entre duas threads compartilha no maximo uma
 *     unica linha de cache. Isso praticamente elimina a FALSA PARTILHA
 *     (false sharing), em que nucleos diferentes disputariam a mesma linha de
 *     cache e forcariam o protocolo de coerencia (MESI) a invalida-la sem parar.
 *
 * Uso: ./matmul_pthreads N T
 *   N = dimensao da matriz
 *   T = numero de threads (ex.: 1, 2, 4, 8, 16)
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

typedef struct {
    const double *A;
    const double *B;
    double       *C;
    size_t        N;
    size_t        linha_ini;  /* primeira linha (inclusive) desta thread */
    size_t        linha_fim;  /* ultima linha (exclusive) desta thread   */
} tarefa_t;

/* Cada thread multiplica sua faixa de linhas: C[i][*] para i em [ini, fim). */
static void *worker(void *arg)
{
    tarefa_t *t = (tarefa_t *)arg;
    const double *A = t->A;
    const double *B = t->B;
    double       *C = t->C;
    size_t        N = t->N;

    for (size_t i = t->linha_ini; i < t->linha_fim; i++) {
        for (size_t j = 0; j < N; j++) {
            double soma = 0.0;
            for (size_t k = 0; k < N; k++) {
                soma += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = soma;
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "Uso: %s N T\n", argv[0]);
        return EXIT_FAILURE;
    }

    size_t N = (size_t)strtoul(argv[1], NULL, 10);
    size_t T = (size_t)strtoul(argv[2], NULL, 10);
    if (N == 0 || T == 0) {
        fprintf(stderr, "Erro: N e T devem ser inteiros positivos.\n");
        return EXIT_FAILURE;
    }
    if (T > N) T = N;  /* nao faz sentido ter mais threads que linhas */

    double *A = alloc_matrix(N);
    double *B = alloc_matrix(N);
    double *C = alloc_matrix(N);

    init_matrices(A, B, N);
    zero_matrix(C, N);

    pthread_t *threads = malloc(T * sizeof(pthread_t));
    tarefa_t  *tarefas = malloc(T * sizeof(tarefa_t));
    if (!threads || !tarefas) {
        fprintf(stderr, "Erro: falha ao alocar controle de threads.\n");
        return EXIT_FAILURE;
    }

    /* Distribui as N linhas entre T threads o mais uniformemente possivel.
     * As primeiras (N % T) threads recebem uma linha a mais. */
    size_t base  = N / T;
    size_t resto = N % T;

    double t0 = now_seconds();

    size_t linha = 0;
    for (size_t th = 0; th < T; th++) {
        size_t qtd = base + (th < resto ? 1 : 0);
        tarefas[th].A = A;
        tarefas[th].B = B;
        tarefas[th].C = C;
        tarefas[th].N = N;
        tarefas[th].linha_ini = linha;
        tarefas[th].linha_fim = linha + qtd;
        linha += qtd;
        pthread_create(&threads[th], NULL, worker, &tarefas[th]);
    }

    for (size_t th = 0; th < T; th++) {
        pthread_join(threads[th], NULL);
    }

    double t1 = now_seconds();

    double tempo = t1 - t0;
    double gflops = (2.0 * (double)N * (double)N * (double)N) / (tempo * 1e9);

    printf("[Matmul Pthreads]        N: %5zu | Threads: %3zu | Tempo: %.4f s | GFLOPS: %.2f | checksum: %.4e\n",
           N, T, tempo, gflops, checksum(C, N));

    free(threads);
    free(tarefas);
    free(A);
    free(B);
    free(C);
    return EXIT_SUCCESS;
}
