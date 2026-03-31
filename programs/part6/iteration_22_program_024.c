/* File 1: Floating-point intensive computations with nested loops */
#include <math.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERS 1000

/* Volatile to prevent optimization */
volatile int g_seed = 42;

/* Complex FP computation with data dependencies */
double compute_fp_kernel(double* restrict a, double* restrict b, 
                         double* restrict c, int n) {
    double sum = 0.0;
    
    /* Outer loop creates scheduling pressure */
    for (int iter = 0; iter < ITERS; iter++) {
        /* Inner loop with FP operations - good for software pipelining */
        for (int i = 0; i < n; i++) {
            /* Mixed FP operations creating dependencies */
            double t = a[i] * b[i];
            t = t + sin(t * 0.01);
            t = cos(t) * exp(t * 0.001);
            c[i] = t * t + sqrt(fabs(t));
            
            /* Accumulate with dependency chain */
            sum += c[i] * (i % 8);
        }
        
        /* Conditional branch creates basic block boundaries */
        if (sum > 1000.0) {
            /* Additional computation in branch */
            for (int i = 0; i < n/2; i++) {
                a[i] = b[i] * 0.5 + c[i] * 0.5;
            }
        } else {
            /* Alternative path */
            for (int i = 0; i < n/2; i++) {
                b[i] = a[i] * 2.0 - c[i];
            }
        }
    }
    
    return sum;
}

/* Matrix multiplication with triple nested loops */
void matrix_multiply(double A[SIZE][SIZE], double B[SIZE][SIZE], 
                     double C[SIZE][SIZE]) {
    /* Triple nested loop - creates complex scheduling opportunities */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            double sum = 0.0;
            /* Innermost loop with FP multiply-add */
            for (int k = 0; k < SIZE; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
            
            /* Conditional inside loop */
            if (C[i][j] < 0) {
                C[i][j] = -C[i][j];
            }
        }
    }
}

/* Function with mixed operations */
double mixed_operations(int n) {
    double result = 1.0;
    volatile double v = 2.0; /* Prevent optimization */
    
    for (int i = 0; i < n; i++) {
        /* Mixed integer/FP operations */
        int idx = (i * 37) % n;
        result = result * v + sin(idx * 0.1);
        result = result / (1.0 + fabs(cos(result)));
        
        /* Branch with FP compare */
        if (result > 100.0) {
            result = result * 0.99;
        } else if (result < -100.0) {
            result = result * 0.99 + 1.0;
        }
    }
    
    return result;
}
