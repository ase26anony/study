/* test1.c - Floating-point intensive with nested loops */
#include <math.h>

volatile double input = 3.14159;

double compute_polynomial(int iterations) {
    double result = 0.0;
    double x = input;
    
    /* Create scheduling pressure with mixed operations */
    for (int i = 0; i < iterations; i++) {
        double term = x;
        
        /* Nested loop creates basic block boundaries */
        for (int j = 0; j < 8; j++) {
            term = term * x + (j * 0.1);
            
            /* Conditional creates branch for scheduler */
            if (term > 100.0) {
                term = term / 2.0;
            } else {
                term = term * 1.5;
            }
        }
        
        result += term;
        x = sin(x) + 0.5;
    }
    
    return result;
}

/* Matrix multiplication creates predictable patterns */
void matrix_multiply(double A[4][4], double B[4][4], double C[4][4]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            C[i][j] = 0.0;
            for (int k = 0; k < 4; k++) {
                /* Multiple dependencies create scheduling constraints */
                C[i][j] += A[i][k] * B[k][j];
                
                /* Additional operation to prevent simple optimization */
                if (C[i][j] < 0) {
                    C[i][j] = -C[i][j];
                }
            }
        }
    }
}
