/*
 * Item 3 - Multiplicacao de matrizes CANONICA (sem blocagem).
 *
 * Calcula C = A * B para matrizes densas N x N usando a formulacao classica
 * de tres lacos aninhados (ordem i, j, k):
 *
 *     C[i][j] = soma_{k=0}^{N-1} A[i][k] * B[k][j]
 *
 * Problema de desempenho: no laco mais interno (sobre k),
 *   - A[i][k] e lido sequencialmente na linha i          (boa localidade),
 *   - C[i][j] fica fixo                                   (boa localidade),
 *   - MAS B[k][j] "pula" uma linha inteira a cada passo (B[0][j], B[1][j], ...),
 *     saltando N*sizeof(double) bytes na memoria. Para matrizes que nao cabem
 *     no cache, cada acesso a B[k][j] tende a gerar um cache miss e forca uma
 *     viagem lenta a DRAM. E dai que vem a maioria esmagadora das faltas.
 *
 * Uso: ./matmul_padrao N
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
    double *B = alloc_matrix(N);
    double *C = alloc_matrix(N);

    init_matrices(A, B, N);
    zero_matrix(C, N);

    double t0 = now_seconds();

    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
            double soma = 0.0;
            for (size_t k = 0; k < N; k++) {
                soma += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = soma;
        }
    }

    double t1 = now_seconds();

    double tempo = t1 - t0;
    /* 2*N^3 operacoes de ponto flutuante (uma multiplicacao + uma soma por termo) */
    double gflops = (2.0 * (double)N * (double)N * (double)N) / (tempo * 1e9);

    printf("[Matmul Padrao]  N: %5zu | Tempo: %.4f s | GFLOPS: %.2f | checksum: %.4e\n",
           N, tempo, gflops, checksum(C, N));

    free(A);
    free(B);
    free(C);
    return EXIT_SUCCESS;
}
