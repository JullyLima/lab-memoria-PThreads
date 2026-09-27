/*
 * Item 2 - Varredura em ORDEM DE COLUNA (Column-Major).
 *
 * Faz exatamente a mesma tarefa do varredura_linha (contar elementos pares de
 * uma matriz N x N), mas invertendo a ordem dos lacos: laco externo em j
 * (colunas) e laco interno em i (linhas).
 *
 * Como C armazena as matrizes linha por linha (row-major), acessar A[i][j] e
 * depois A[i+1][j] provoca um salto de N*sizeof(double) bytes na memoria. Para
 * N grande, a linha de cache recem-carregada e descartada antes de ser
 * reaproveitada => uma falta de cache (cache miss) a cada iteracao => execucao
 * muito mais lenta. O resultado (contagem de pares) e identico ao da varredura
 * por linha; o que muda drasticamente e o TEMPO.
 *
 * Uso: ./varredura_coluna N
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
    for (size_t j = 0; j < N; j++) {         /* laco externo: colunas */
        for (size_t i = 0; i < N; i++) {     /* laco interno: linhas  */
            if ((int)A[i * N + j] % 2 == 0) {
                pares++;
            }
        }
    }

    double t1 = now_seconds();

    printf("[Varredura COLUNA]  N: %5zu | Pares: %ld | Tempo: %.6f s\n",
           N, pares, t1 - t0);

    free(A);
    return EXIT_SUCCESS;
}
