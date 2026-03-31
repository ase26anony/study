/* File 1: compute-intensive.c - Floating point intensive computations
   Designed to trigger software pipelining and scheduler state save/restore */
#include <math.h>
#include <stdio.h>

#define SIZE 1024
#define ITERATIONS 100

/* Complex floating-point computation with data dependencies */
void compute_kernel(float *restrict a, float *restrict b, float *restrict c, 
                    int n, int iterations) {
    volatile int seed = n; /* Prevent optimization */
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Multiple nested loops with varying dependencies */
        for (int i = 1; i < n - 1; i++) {
            /* Create complex dependency chain */
            float t1 = a[i] * b[i] + seed;
            float t2 = sinf(t1) * cosf(b[i]);
            float t3 = t2 * t2 - a[i-1];
            float t4 = sqrtf(fabsf(t3)) + 0.5f;
            
            /* Cross-iteration dependency */
            c[i] = t4 * b[i+1] + (iter > 0 ? c[i] * 0.1f : 0.0f);
            
            /* Conditional that creates scheduling barriers */
            if (t4 > 100.0f) {
                a[i] = t4 * 0.9f;
            } else {
                a[i] = t4 * 1.1f;
            }
        }
        
        /* Another loop with different pattern */
        for (int i = n - 2; i > 0; i--) {
            b[i] = (a[i] + a[i-1] + a[i+1]) / 3.0f;
            /* Mix integer and float operations */
            int idx = (int)(b[i] * 10) % n;
            c[i] += b[idx] * 0.01f;
        }
    }
}

/* Matrix-like operations with triangular loops */
void matrix_operations(float mat[SIZE][SIZE], float vec[SIZE], float result[SIZE]) {
    /* Triple nested loop - creates scheduling pressure */
    for (int i = 0; i < SIZE; i++) {
        float sum = 0.0f;
        for (int j = 0; j < SIZE; j++) {
            /* Create anti-dependencies */
            float temp = mat[i][j] * vec[j];
            mat[i][j] = temp * 0.99f; /* Write after read */
            sum += temp;
            
            /* Inner conditional loop */
            for (int k = 0; k < 4; k++) {
                if (temp > 0) {
                    sum += sinf(temp * k) * 0.1f;
                }
            }
        }
        result[i] = sum;
        
        /* Loop with pointer arithmetic */
        float *p = &mat[i][0];
        for (int j = 0; j < SIZE; j += 4) {
            p[j] *= 1.01f;
            p[j+1] *= 0.99f;
            p[j+2] = p[j] + p[j+1];
            p[j+3] = p[j+2] * 0.5f;
        }
    }
}

/* Main computation function */
void intensive_computation() {
    float array1[SIZE], array2[SIZE], array3[SIZE];
    float matrix[SIZE][SIZE];
    float result[SIZE];
    
    /* Initialize with non-constant values */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i % 100) * 0.1f;
        array2[i] = ((i * 3) % 97) * 0.2f;
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = ((i * j) % 101) * 0.01f;
        }
    }
    
    /* Multiple passes with different parameters */
    compute_kernel(array1, array2, array3, SIZE, ITERATIONS);
    compute_kernel(array2, array3, array1, SIZE, ITERATIONS/2);
    
    matrix_operations(matrix, array1, result);
    
    /* Use results to prevent elimination */
    volatile float sink = 0.0f;
    for (int i = 0; i < SIZE; i++) {
        sink += result[i] + array3[i];
    }
}
