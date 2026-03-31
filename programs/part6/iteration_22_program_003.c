/* test1.c - Floating-point intensive computations with nested loops */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define ITERS 1000

/* Complex floating-point computation with data dependencies */
void compute_fft_like(float *real, float *imag, int n) {
    float temp_real, temp_imag;
    int i, j, k;
    
    /* Outer loop with multiple iterations */
    for (k = 0; k < ITERS; k++) {
        /* Nested loops creating scheduling pressure */
        for (i = 0; i < n; i++) {
            temp_real = real[i];
            temp_imag = imag[i];
            
            /* Inner loop with mixed operations */
            for (j = 0; j < n; j++) {
                if (j != i) {
                    float angle = 2.0f * M_PI * i * j / n;
                    float cos_val = cosf(angle);
                    float sin_val = sinf(angle);
                    
                    /* Complex multiplication */
                    float r = real[j] * cos_val - imag[j] * sin_val;
                    float im = real[j] * sin_val + imag[j] * cos_val;
                    
                    /* Accumulate with data dependencies */
                    temp_real += r * 0.01f;
                    temp_imag += im * 0.01f;
                    
                    /* Additional arithmetic to create more instructions */
                    temp_real = temp_real * 0.99f + 0.01f * r;
                    temp_imag = temp_imag * 0.99f + 0.01f * im;
                }
            }
            
            /* Conditional store */
            if (temp_real > 0.0f && temp_imag > 0.0f) {
                real[i] = temp_real * 0.5f;
                imag[i] = temp_imag * 0.5f;
            } else {
                real[i] = temp_real;
                imag[i] = temp_imag;
            }
        }
        
        /* Another loop with different pattern */
        for (i = 1; i < n - 1; i++) {
            /* Stencil computation with dependencies */
            real[i] = (real[i-1] + real[i] + real[i+1]) / 3.0f;
            imag[i] = (imag[i-1] + imag[i] + imag[i+1]) / 3.0f;
            
            /* More arithmetic operations */
            real[i] = real[i] * real[i] - imag[i] * imag[i];
            imag[i] = 2.0f * real[i] * imag[i];
        }
    }
}

/* Matrix multiplication with complex scheduling requirements */
void matrix_operations(float A[SIZE][SIZE], float B[SIZE][SIZE], 
                       float C[SIZE][SIZE], int block_size) {
    int i, j, k, ii, jj, kk;
    
    /* Blocked matrix multiplication - creates many nested loops */
    for (ii = 0; ii < SIZE; ii += block_size) {
        for (jj = 0; jj < SIZE; jj += block_size) {
            for (kk = 0; kk < SIZE; kk += block_size) {
                /* Micro-kernel with unrolled inner loops */
                for (i = ii; i < ii + block_size && i < SIZE; i++) {
                    for (j = jj; j < jj + block_size && j < SIZE; j++) {
                        float sum = C[i][j];
                        for (k = kk; k < kk + block_size && k < SIZE; k++) {
                            /* Mixed operations: multiply, add, conditional */
                            float prod = A[i][k] * B[k][j];
                            sum += prod;
                            
                            /* Branch to create control flow */
                            if (prod < 0.0f) {
                                sum -= prod * 0.5f;  /* Compensate */
                            }
                        }
                        C[i][j] = sum;
                    }
                }
            }
        }
    }
}

/* Main test function for file1 */
void test1_main(int argc, char **argv) {
    float real[SIZE], imag[SIZE];
    float A[SIZE][SIZE], B[SIZE][SIZE], C[SIZE][SIZE];
    int i, j;
    
    /* Initialize with volatile to prevent optimization */
    volatile int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    for (i = 0; i < SIZE; i++) {
        real[i] = (float)(i * seed) / 100.0f;
        imag[i] = (float)((i + seed) * 3) / 100.0f;
        
        for (j = 0; j < SIZE; j++) {
            A[i][j] = (float)(i * j + seed) / 1000.0f;
            B[i][j] = (float)(i + j * seed) / 1000.0f;
            C[i][j] = 0.0f;
        }
    }
    
    /* Call compute-intensive functions */
    compute_fft_like(real, imag, SIZE);
    matrix_operations(A, B, C, 16);
    
    /* Use results to prevent dead code elimination */
    float sum = 0.0f;
    for (i = 0; i < SIZE; i++) {
        sum += real[i] + imag[i];
        for (j = 0; j < SIZE; j++) {
            sum += C[i][j];
        }
    }
    
    /* Volatile store to ensure computation isn't optimized away */
    volatile float result = sum;
    (void)result;
}
