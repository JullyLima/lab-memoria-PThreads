#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

double now_seconds(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

double *alloc_matrix(size_t n)
{
    double *m = (double *)malloc(n * n * sizeof(double));
    if (m == NULL) {
        fprintf(stderr, "Erro: falha ao alocar matriz %zux%zu (%zu bytes)\n",
                n, n, n * n * sizeof(double));
        exit(EXIT_FAILURE);
    }
    return m;
}

void init_matrices(double *A, double *B, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            if (A) A[i * n + j] = (double)(i + j);
            if (B) B[i * n + j] = (double)(i * j);
        }
    }
}

void zero_matrix(double *C, size_t n)
{
    for (size_t i = 0; i < n * n; i++) {
        C[i] = 0.0;
    }
}

double checksum(const double *M, size_t n)
{
    double s = 0.0;
    for (size_t i = 0; i < n * n; i++) {
        s += M[i];
    }
    return s;
}
