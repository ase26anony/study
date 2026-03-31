/* test1.c - Floating-point intensive with modulo scheduling */
#include <math.h>

volatile double input = 3.14159;

double compute_polynomial(int iterations) {
    double result = 0.0;
    double x = input;
    
    /* Complex loop with data dependencies - good for software pipelining */
    for (int i = 0; i < iterations; i++) {
        /* Mixed FP operations creating long dependency chains */
        double term1 = x * x * x;
        double term2 = sin(x) * cos(x);
        double term3 = exp(-x * 0.1);
        
        result += term1 * 0.3 + term2 * 0.5 + term3 * 0.2;
        
        /* Create loop-carried dependency */
        x = fmod(result, 10.0) + 0.1;
        
        /* Conditional inside loop creates scheduling barriers */
        if (result > 1000.0) {
            result *= 0.99;
        }
    }
    
    return result;
}

/* Nested loops for more scheduling complexity */
void matrix_multiply(double A[10][10], double B[10][10], double C[10][10]) {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            C[i][j] = 0.0;
            for (int k = 0; k < 10; k++) {
                /* Complex expression with multiple dependencies */
                C[i][j] += A[i][k] * B[k][j] 
                          + sin(A[i][k]) * cos(B[k][j])
                          + 0.5 * (A[i][k] + B[k][j]);
            }
        }
    }
}
