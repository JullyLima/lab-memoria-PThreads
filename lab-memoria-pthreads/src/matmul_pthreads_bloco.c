/*
 * Item 4 - Multiplicacao PARALELA com Pthreads + BLOCAGEM (versao avancada).
 *
 * Combina as duas otimizacoes deste laboratorio:
 *   1. Paralelismo: as N linhas de C sao divididas em faixas contiguas entre
 *      T threads (mesma estrategia sem-mutex e sem-falsa-partilha do
 *      matmul_pthreads).
 *   2. Blocagem (tiling): DENTRO de sua faixa de linhas, cada thread aplica a
 *      tecnica de blocos BLK x BLK, mantendo os dados nos caches L1d/L2 e
 *      reduzindo o trafego para a DRAM.
 *
 * A sinergia esperada: o paralelismo divide o trabalho entre nucleos e a
 * blocagem reduz o quanto cada nucleo precisa buscar na DRAM. Como a largura
 * de banda da memoria e um recurso compartilhado, diminuir o trafego por
 * nucleo melhora a escalabilidade global (ganhos tendem a ser sinergicos, nao
 * apenas aditivos).
 *
 * Uso: ./matmul_pthreads_bloco N T BLK
 *   N   = dimensao da matriz
 *   T   = numero de threads
 *   BLK = tamanho do bloco (tile)
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
    size_t        BLK;
    size_t        linha_ini;  /* primeira linha (inclusive) desta thread */
    size_t        linha_fim;  /* ultima linha (exclusive) desta thread   */
} tarefa_t;

/*
 * Cada thread processa suas linhas [linha_ini, linha_fim) com blocagem.
 * O laco de blocos em ii fica restrito a faixa de linhas da thread; os blocos
 * em jj e kk percorrem toda a largura da matriz. A ordem interna e i, k, j
 * para manter o acesso mais interno contiguo por linha (favoravel a SIMD).
 */
static void *worker(void *arg)
{
    tarefa_t *t = (tarefa_t *)arg;
    const double *A = t->A;
    const double *B = t->B;
    double       *C = t->C;
    size_t        N = t->N;
    size_t        BLK = t->BLK;

    for (size_t ii = t->linha_ini; ii < t->linha_fim; ii += BLK) {
        size_t i_max = (ii + BLK < t->linha_fim) ? ii + BLK : t->linha_fim;
        for (size_t jj = 0; jj < N; jj += BLK) {
            size_t j_max = (jj + BLK < N) ? jj + BLK : N;
            for (size_t kk = 0; kk < N; kk += BLK) {
                size_t k_max = (kk + BLK < N) ? kk + BLK : N;
                for (size_t i = ii; i < i_max; i++) {
                    for (size_t k = kk; k < k_max; k++) {
                        double r = A[i * N + k];
                        for (size_t j = jj; j < j_max; j++) {
                            C[i * N + j] += r * B[k * N + j];
                        }
                    }
                }
            }
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 4) {
        fprintf(stderr, "Uso: %s N T BLK\n", argv[0]);
        return EXIT_FAILURE;
    }

    size_t N   = (size_t)strtoul(argv[1], NULL, 10);
    size_t T   = (size_t)strtoul(argv[2], NULL, 10);
    size_t BLK = (size_t)strtoul(argv[3], NULL, 10);
    if (N == 0 || T == 0 || BLK == 0) {
        fprintf(stderr, "Erro: N, T e BLK devem ser inteiros positivos.\n");
        return EXIT_FAILURE;
    }
    if (T > N) T = N;

    double *A = alloc_matrix(N);
    double *B = alloc_matrix(N);
    double *C = alloc_matrix(N);

    init_matrices(A, B, N);
    zero_matrix(C, N);   /* C acumula somas parciais na blocagem */

    pthread_t *threads = malloc(T * sizeof(pthread_t));
    tarefa_t  *tarefas = malloc(T * sizeof(tarefa_t));
    if (!threads || !tarefas) {
        fprintf(stderr, "Erro: falha ao alocar controle de threads.\n");
        return EXIT_FAILURE;
    }

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
        tarefas[th].BLK = BLK;
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

    printf("[Matmul Pthreads+Bloco]  N: %5zu | Threads: %3zu | Bloco B: %4zu | Tempo: %.4f s | GFLOPS: %.2f | checksum: %.4e\n",
           N, T, BLK, tempo, gflops, checksum(C, N));

    free(threads);
    free(tarefas);
    free(A);
    free(B);
    free(C);
    return EXIT_SUCCESS;
}
