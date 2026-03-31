/* Test 1: Floating-point intensive computation with nested loops */
#include <math.h>

volatile double input = 3.14159;

double compute_polynomial(int iterations) {
    double result = 0.0;
    double x = input;
    
    /* Nested loops create scheduling pressure */
    for (int i = 0; i < iterations; i++) {
        double term = 1.0;
        for (int j = 0; j < 8; j++) {
            term *= x;
            term /= (j + 1);
        }
        result += term;
        x = sin(x) + 0.5;
    }
    
    /* Mix of operations */
    for (int i = 0; i < iterations; i++) {
        result = result * 1.1 - 0.3;
        if (result > 100.0) {
            result = sqrt(result);
        } else {
            result = result * result;
        }
    }
    
    return result;
}

/* Function with software pipelining opportunities */
void matrix_multiply(double A[8][8], double B[8][8], double C[8][8]) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            C[i][j] = 0.0;
            for (int k = 0; k < 8; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}
