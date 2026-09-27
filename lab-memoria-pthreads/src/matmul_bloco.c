/*
 * Item 3 - Multiplicacao de matrizes com BLOCAGEM (Tiling).
 *
 * Calcula C = A * B (mesmo resultado do matmul_padrao), mas particiona as
 * matrizes em sub-blocos de dimensao BLK x BLK. A ideia e escolher BLK pequeno
 * o bastante para que os tres sub-blocos ativos (de A, B e C) caibam juntos no
 * cache L1d/L2:
 *
 *     3 * (BLK * BLK * sizeof(double)) <= capacidade da cache
 *
 * Enquanto a multiplicacao trabalha dentro de um bloco, os dados permanecem nos
 * niveis mais rapidos do cache, convertendo o que seriam faltas na DRAM em
 * acertos em L1/L2. Isso reduz drasticamente os LLd misses e acelera a
 * execucao (tipicamente varias vezes mais rapido).
 *
 * Detalhe de alto desempenho: a ordem dos lacos internos e i, k, j. Assim o
 * acesso mais interno a B[k*N + j] e a C[i*N + j] e contiguo por linha
 * (row-major), o que favorece a auto-vetorizacao SIMD (AVX/AVX2) do compilador.
 *
 * Uso: ./matmul_bloco N BLK
 *   N   = dimensao da matriz
 *   BLK = tamanho do bloco (tile). Ex.: 32, 64, 128.
 */

#include <stdio.h>
#include <stdlib.h>

#include "util.h"

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "Uso: %s N BLK\n", argv[0]);
        return EXIT_FAILURE;
    }

    size_t N   = (size_t)strtoul(argv[1], NULL, 10);
    size_t BLK = (size_t)strtoul(argv[2], NULL, 10);
    if (N == 0 || BLK == 0) {
        fprintf(stderr, "Erro: N e BLK devem ser inteiros positivos.\n");
        return EXIT_FAILURE;
    }

    double *A = alloc_matrix(N);
    double *B = alloc_matrix(N);
    double *C = alloc_matrix(N);

    init_matrices(A, B, N);
    zero_matrix(C, N);   /* C recebe somas parciais, entao precisa comecar zerada */

    double t0 = now_seconds();

    for (size_t ii = 0; ii < N; ii += BLK) {
        for (size_t jj = 0; jj < N; jj += BLK) {
            for (size_t kk = 0; kk < N; kk += BLK) {
                /* multiplicacao do sub-bloco BLK x BLK */
                size_t i_max = (ii + BLK < N) ? ii + BLK : N;
                size_t k_max = (kk + BLK < N) ? kk + BLK : N;
                size_t j_max = (jj + BLK < N) ? jj + BLK : N;

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

    double t1 = now_seconds();

    double tempo = t1 - t0;
    double gflops = (2.0 * (double)N * (double)N * (double)N) / (tempo * 1e9);

    printf("[Matmul Blocado] N: %5zu | Bloco B: %4zu | Tempo: %.4f s | GFLOPS: %.2f | checksum: %.4e\n",
           N, BLK, tempo, gflops, checksum(C, N));

    free(A);
    free(B);
    free(C);
    return EXIT_SUCCESS;
}
