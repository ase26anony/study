/* test1.c - Floating-point intensive with modulo scheduling */
#include <math.h>

#define SIZE 1024

void compute_kernel_fp(float *restrict a, float *restrict b, 
                       float *restrict c, int n) {
    for (int i = 0; i < n; i++) {
        // Complex FP operations with dependencies
        float t1 = a[i] * b[i];
        float t2 = sinf(t1) + cosf(a[i]);
        float t3 = t2 * t2 - sqrtf(fabsf(b[i]));
        c[i] = t3 * 0.5f + t1;
        
        // Additional computation to create scheduling pressure
        for (int j = 0; j < 4; j++) {
            c[i] += (a[i] * j) / (b[i] + 1.0f);
        }
    }
}

void matrix_multiply(float A[SIZE][SIZE], float B[SIZE][SIZE],
                     float C[SIZE][SIZE]) {
    // Triple nested loop - good for software pipelining
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            float sum = 0.0f;
            for (int k = 0; k < SIZE; k++) {
                // Mixed operations with dependencies
                sum += A[i][k] * B[k][j];
                sum = sum * 0.99f + 0.01f; // Prevent optimization
            }
            C[i][j] = sum;
        }
    }
}

// Volatile to prevent dead code elimination
volatile int trigger = 0;

void test1_main() {
    float arr1[SIZE], arr2[SIZE], arr3[SIZE];
    float mat1[SIZE][SIZE], mat2[SIZE][SIZE], mat3[SIZE][SIZE];
    
    // Initialize with non-zero values
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (float)(i % 100) * 0.1f;
        arr2[i] = (float)((i + 1) % 100) * 0.2f;
    }
    
    if (trigger) {
        compute_kernel_fp(arr1, arr2, arr3, SIZE);
        matrix_multiply(mat1, mat2, mat3);
    }
}
