/* Test 1: Floating-point intensive computation with nested loops */
#include <math.h>

volatile double input = 3.14159;
volatile int iterations = 1000;

double compute_pi_approximation(int n) {
    double sum = 0.0;
    double sign = 1.0;
    
    /* Complex loop with data dependencies */
    for (int i = 0; i < n; i++) {
        double term = sign / (2 * i + 1);
        
        /* Nested computation with branches */
        if (i % 2 == 0) {
            for (int j = 0; j < 5; j++) {
                term *= (1.0 + 0.1 * j);
            }
        } else {
            for (int j = 0; j < 3; j++) {
                term /= (1.0 + 0.05 * j);
            }
        }
        
        sum += term;
        sign = -sign;
        
        /* Additional arithmetic to create scheduling pressure */
        sum = sum * 0.999 + term * 0.001;
    }
    
    return 4.0 * sum;
}

/* Matrix multiplication with mixed operations */
void matrix_multiply(double A[4][4], double B[4][4], double C[4][4]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            C[i][j] = 0.0;
            for (int k = 0; k < 4; k++) {
                /* Complex expression with multiple operations */
                C[i][j] += A[i][k] * B[k][j] 
                          + sin(A[i][k] * 0.1) * cos(B[k][j] * 0.1);
            }
            /* Conditional operation */
            if (C[i][j] < 0) {
                C[i][j] = -C[i][j] * 0.5;
            }
        }
    }
}
