/* Test 1: Floating-point intensive computations with nested loops */
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
        
        /* Additional computation to create dependencies */
        if (i % 3 == 0) {
            sum = sum * 1.0001;
        } else if (i % 7 == 0) {
            sum = sum / 1.00005;
        }
    }
    
    return 4.0 * sum;
}

void matrix_multiply_3x3(double A[3][3], double B[3][3], double C[3][3]) {
    /* Triple nested loop - creates complex scheduling graph */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            C[i][j] = 0.0;
            for (int k = 0; k < 3; k++) {
                C[i][j] += A[i][k] * B[k][j];
                /* Conditional creates branch scheduling opportunity */
                if (C[i][j] > 1000.0) {
                    C[i][j] = fmod(C[i][j], 1000.0);
                }
            }
        }
    }
}

double test1_main(int argc, char **argv) {
    int iterations = argc > 1 ? atoi(argv[1]) : 100000;
    if (iterations < 1000) iterations = 1000;
    
    double result = compute_pi_approximation(iterations);
    
    /* Create matrix multiplication to add more scheduling complexity */
    double A[3][3], B[3][3], C[3][3];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            A[i][j] = (i + j) * 0.1 + global_seed;
            B[i][j] = (i * j) * 0.2 - global_seed;
        }
    }
    
    matrix_multiply_3x3(A, B, C);
    
    /* Use result to prevent optimization */
    return result + C[0][0];
}
