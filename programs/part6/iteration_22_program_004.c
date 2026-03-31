/* test1.c - Floating-point intensive computations */
#include <math.h>
#include <stdlib.h>

volatile double global_seed = 0.12345;

double compute_pi_approximation(int iterations) {
    double sum = 0.0;
    double sign = 1.0;
    
    /* Create scheduling pressure with mixed operations */
    for (int i = 0; i < iterations; i++) {
        double term = sign / (2 * i + 1);
        sum += term;
        sign = -sign;
        
        /* Additional computations to create dependencies */
        if (i % 7 == 0) {
            sum = sum * 1.0001 - 0.0001;
        }
    }
    
    return 4.0 * sum;
}

void matrix_multiply_3d(double A[8][8][8], double B[8][8][8], double C[8][8][8]) {
    /* Nested loops create complex scheduling scenarios */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                C[i][j][k] = 0.0;
                for (int l = 0; l < 8; l++) {
                    /* Mixed operations with dependencies */
                    double temp = A[i][j][l] * B[l][j][k];
                    C[i][j][k] += temp;
                    
                    /* Conditional creates branch scheduling needs */
                    if (temp > 100.0) {
                        C[i][j][k] *= 0.99;
                    }
                }
                /* Additional operation to prevent optimization */
                C[i][j][k] += global_seed * 0.001;
            }
        }
    }
}

double complex_math_operations(int n) {
    double result = 0.0;
    double x = 1.0;
    
    /* Loop with varying operations */
    for (int i = 1; i <= n; i++) {
        x = sin(x) + cos(x) * 0.5;
        result += x / i;
        
        /* Branch creates scheduling barrier */
        if (i % 3 == 0) {
            result = sqrt(fabs(result));
        } else if (i % 5 == 0) {
            result = result * result * 0.5;
        }
    }
    
    return result;
}
