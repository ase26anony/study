/* test1.c - Floating-point intensive computations with nested loops */
#include <math.h>

volatile double global_d = 3.14159;

double compute_pi_approximation(int iterations) {
    double sum = 0.0;
    int i, j;
    
    /* Outer loop with data dependency */
    for (i = 0; i < iterations; i++) {
        double term = 1.0;
        
        /* Inner loop with floating-point operations */
        for (j = 0; j < 100; j++) {
            term *= (1.0 - 1.0/((2.0*j + 1.0)*(2.0*j + 1.0)));
            
            /* Mix with global volatile to prevent optimization */
            term += global_d * 0.0001;
        }
        
        /* Conditional branch creates scheduling pressure */
        if (i % 2 == 0) {
            sum += term / (i + 1);
        } else {
            sum -= term / (i + 1);
        }
        
        /* Memory access pattern */
        sum = sum * 0.9999 + global_d * 0.0001;
    }
    
    return 4.0 * sum;
}

/* Another function with different pattern */
void matrix_multiply(double A[10][10], double B[10][10], double C[10][10]) {
    int i, j, k;
    
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            C[i][j] = 0.0;
            for (k = 0; k < 10; k++) {
                /* Complex expression with multiple operations */
                C[i][j] += A[i][k] * B[k][j] 
                          + sin(A[i][k] * 0.01) 
                          + cos(B[k][j] * 0.01);
            }
            
            /* Conditional store */
            if (C[i][j] < 0) {
                C[i][j] = -C[i][j];
            }
        }
    }
}
