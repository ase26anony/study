/* Test 1: Floating-point intensive computation with nested loops */
#include <math.h>

volatile double input = 3.14159;
volatile int iterations = 1000;

double compute_pi_approximation(int n) {
    double sum = 0.0;
    double sign = 1.0;
    
    /* Create scheduling pressure with data-dependent floating point ops */
    for (int i = 0; i < n; i++) {
        double term = sign / (2.0 * i + 1.0);
        
        /* Mix operations to create varied instruction types */
        term = term * term * input;
        term = term / (i + 1.0);
        term = term + (term * 0.1);
        
        sum += term;
        sign = -sign;
        
        /* Add conditional to create control flow */
        if (i % 7 == 0) {
            sum = sum * 0.99;
        }
    }
    
    return 4.0 * sum;
}

/* Function with software pipelining opportunities */
void matrix_multiply_3x3(double A[3][3], double B[3][3], double C[3][3]) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            C[i][j] = 0.0;
            for (int k = 0; k < 3; k++) {
                /* Complex expression with multiple dependencies */
                C[i][j] += A[i][k] * B[k][j] + 
                           (A[i][k] * 0.5) * (B[k][j] * 0.5);
            }
            /* Additional operation to prevent simple optimization */
            C[i][j] = C[i][j] * (1.0 + (i * j * 0.01));
        }
    }
}
