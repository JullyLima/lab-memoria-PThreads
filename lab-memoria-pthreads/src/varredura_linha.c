/*
 * Item 2 - Varredura em ORDEM DE LINHA (Row-Major).
 *
 * Inicializa uma matriz quadrada A de dimensao N x N e conta quantos
 * elementos pares ela contem, percorrendo a matriz com o laco externo em i
 * (linhas) e o laco interno em j (colunas).
 *
 * Como C armazena as matrizes linha por linha (row-major), este padrao acessa
 * posicoes de memoria contiguas: cada linha de cache carregada da DRAM traz
 * varios elementos vizinhos que serao usados nas iteracoes seguintes (alta
 * localidade espacial => muitos cache hits => execucao rapida).
 *
 * Uso: ./varredura_linha N
 *   N = dimensao da matriz quadrada (ex.: 512, 1024, 2048, 4096, 8192)
 */

#include <stdio.h>
#include <stdlib.h>

#include "util.h"

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Uso: %s N\n", argv[0]);
        return EXIT_FAILURE;
    }

    size_t N = (size_t)strtoul(argv[1], NULL, 10);
    if (N == 0) {
        fprintf(stderr, "Erro: N deve ser um inteiro positivo.\n");
        return EXIT_FAILURE;
    }

    double *A = alloc_matrix(N);
    init_matrices(A, NULL, N);

    double t0 = now_seconds();

    long pares = 0;
    for (size_t i = 0; i < N; i++) {         /* laco externo: linhas  */
        for (size_t j = 0; j < N; j++) {     /* laco interno: colunas */
            if ((int)A[i * N + j] % 2 == 0) {
                pares++;
            }
        }
    }

    double t1 = now_seconds();

    printf("[Varredura LINHA]   N: %5zu | Pares: %ld | Tempo: %.6f s\n",
           N, pares, t1 - t0);

    free(A);
    return EXIT_SUCCESS;
}
