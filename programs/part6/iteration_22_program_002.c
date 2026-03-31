/* test1.c - Floating-point intensive computations with nested loops */
#include <math.h>
#include <stdlib.h>

/* Function with tight floating-point loop that creates scheduling pressure */
void compute_heat_transfer(int size, double* restrict in, double* restrict out) {
    volatile int seed = size; /* Prevent optimization */
    
    for (int iter = 0; iter < 100; iter++) {
        /* Mixed operations: multiply, add, divide, sqrt */
        for (int i = 1; i < size - 1; i++) {
            for (int j = 1; j < size - 1; j++) {
                int idx = i * size + j;
                /* Create complex data dependencies */
                double diff1 = in[idx-1] - in[idx];
                double diff2 = in[idx+1] - in[idx];
                double diff3 = in[idx-size] - in[idx];
                double diff4 = in[idx+size] - in[idx];
                
                /* Mixed arithmetic operations to create varied instruction types */
                out[idx] = in[idx] + 0.25 * (diff1 + diff2 + diff3 + diff4);
                
                /* Conditional branch inside loop */
                if (out[idx] > 1000.0) {
                    out[idx] = sqrt(out[idx]) * 0.5;
                } else if (out[idx] < -1000.0) {
                    out[idx] = fabs(out[idx]) * 0.3;
                }
            }
        }
        
        /* Swap buffers */
        double* temp = in;
        in = out;
        out = temp;
    }
}

/* Another function with different pattern */
void matrix_operations(int n, double* restrict A, double* restrict B, double* restrict C) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                /* Complex addressing with mixed operations */
                sum += A[i * n + k] * B[k * n + j];
                /* Create scheduling barrier with function call */
                if (k % 8 == 0) {
                    sum = fmod(sum, 100.0);
                }
            }
            C[i * n + j] = sum;
            
            /* Branch with data-dependent condition */
            if ((i + j) % 3 == 0) {
                C[i * n + j] *= 1.5;
            }
        }
    }
}
