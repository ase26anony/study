/* Test 1: Floating-point intensive computation with nested loops */
#include <math.h>

volatile double global_seed = 3.14159;

double compute_pi_approximation(int iterations) {
    double sum = 0.0;
    double sign = 1.0;
    
    /* Create scheduling pressure with mixed operations */
    for (int i = 0; i < iterations; i++) {
        double term = sign / (2 * i + 1);
        
        /* Nested computation to create complex basic block */
        for (int j = 0; j < 4; j++) {
            term = term * (1.0 + global_seed / (j + 1));
            term = term / (1.0 + fabs(sin(term)));
        }
        
        sum += term;
        sign = -sign;
        
        /* Conditional to create control flow */
        if (i % 100 == 0) {
            sum = sum * 0.99;  /* Prevent optimization */
        }
    }
    
    return 4.0 * sum;
}

/* Matrix multiplication with software pipelining potential */
void matrix_multiply(int n, double A[n][n], double B[n][n], double C[n][n]) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            C[i][j] = 0.0;
            /* Innermost loop - target for modulo scheduling */
            for (int k = 0; k < n; k++) {
                C[i][j] += A[i][k] * B[k][j];
                /* Add dependency chain */
                C[i][j] = C[i][j] * (1.0 + 0.0001 * (i + j + k));
            }
        }
    }
}
