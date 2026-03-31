/* test1.c - Floating-point intensive with modulo scheduling opportunities */
#include <math.h>

#define SIZE 1024

void compute_pi_approximation(double *result, int iterations) {
    volatile int seed = iterations; /* Prevent optimization */
    double sum = 0.0;
    
    /* Complex loop with data dependencies - good for modulo scheduling */
    for (int i = 0; i < iterations; i++) {
        double term = 1.0 / (2.0 * i + 1.0);
        if (i % 2 == 0) {
            sum += term;
        } else {
            sum -= term;
        }
        /* Cross-iteration dependency */
        result[i % SIZE] = sum * 4.0;
    }
    
    /* Nested loop with varying trip counts */
    for (int i = 0; i < iterations; i++) {
        double acc = result[i % SIZE];
        for (int j = 0; j < (i % 8) + 1; j++) {
            acc = sin(acc) + cos(acc);
        }
        result[i % SIZE] = acc;
    }
}

/* Matrix multiplication with complex access pattern */
void matrix_transform(double A[SIZE][SIZE], double B[SIZE][SIZE], int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                /* Complex expression with multiple operations */
                sum += A[i][k] * B[k][j] * sin(i * j * k * 0.001);
            }
            A[i][j] = sum + (i * j) * 0.01;
        }
    }
}
