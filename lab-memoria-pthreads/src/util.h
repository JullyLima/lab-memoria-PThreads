#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

/*
 * Utilitarios comuns do laboratorio de hierarquia de memoria.
 *
 * Convencao de armazenamento: TODAS as matrizes sao alocadas como um bloco
 * contiguo de memoria (double* de N*N elementos) e indexadas como A[i*N + j].
 * Isso garante o layout Row-Major previsivel exigido pelo laboratorio e evita
 * o espalhamento de ponteiros pelo heap causado por matrizes double**.
 */

/* Retorna o tempo atual em segundos (relogio monotonico de alta precisao).
 * Usa clock_gettime(CLOCK_MONOTONIC) conforme exigido pelo enunciado. */
double now_seconds(void);

/* Aloca um bloco contiguo de n*n doubles. Aborta o programa se falhar. */
double *alloc_matrix(size_t n);

/* Inicializacao deterministica padronizada do laboratorio:
 *   A[i*N + j] = (double)(i + j)
 *   B[i*N + j] = (double)(i * j)
 * Qualquer um dos ponteiros pode ser NULL se aquela matriz nao for necessaria. */
void init_matrices(double *A, double *B, size_t n);

/* Zera uma matriz contigua de n*n doubles (usado para a matriz resultado C). */
void zero_matrix(double *C, size_t n);

/* Soma de verificacao simples de toda a matriz. Serve para comprovar a
 * integridade numerica: versoes diferentes do mesmo calculo devem produzir
 * exatamente o mesmo checksum. */
double checksum(const double *M, size_t n);

#endif /* UTIL_H */
