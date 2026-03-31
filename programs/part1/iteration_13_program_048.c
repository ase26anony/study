/* cache_detection_test.c
 * Designed to trigger CPU cache detection logic in GCC driver
 * Compile with various x86-specific flags to exercise cache descriptor cases
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Force compiler to consider different cache configurations */
#ifdef __x86_64__
#define ARCH_SPECIFIC_CODE 1
#endif

/* Large arrays to exceed typical L1/L2 cache sizes */
#define LARGE_SIZE 10000
#define MEDIUM_SIZE 5000
#define SMALL_STRIDE 16

/* Volatile variables to prevent compile-time optimization */
volatile int outer_limit = LARGE_SIZE;
volatile int inner_limit = MEDIUM_SIZE;
volatile int stride = SMALL_STRIDE;

/* Matrix multiplication style triple nested loop */
void matrix_style_operations(int n, double* restrict A, double* restrict B, double* restrict C) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += A[i * n + k] * B[k * n + j];
            }
            C[i * n + j] = sum;
        }
    }
}

/* Non-unit stride access pattern */
void stride_access_pattern(int* array, int size, int stride_val) {
    long long sum = 0;
    for (int i = 0; i < size; i += stride_val) {
        array[i] = i * 2;
        sum += array[i];
        
        /* Access with different strides to create cache pressure */
        if (i % 3 == 0) {
            array[i + 1] = array[i] * 3;
        }
    }
    
    /* Prevent dead code elimination */
    if (sum < 0) printf("Impossible\n");
}

/* Cache line aliasing test */
void cache_line_aliasing(char* src, char* dst, int size) {
    /* Copy with potential cache line conflicts */
    for (int i = 0; i < size; i += 64) {  /* Typical cache line size */
        for (int j = 0; j < 64 && (i + j) < size; j++) {
            dst[i + j] = src[i + j] + 1;
        }
    }
    
    /* Reverse copy to create more cache traffic */
    for (int i = size - 1; i >= 0; i -= 32) {
        src[i] = dst[i] * 2;
    }
}

/* Mixed data type operations */
void mixed_data_operations(void) {
    static double darray[MEDIUM_SIZE];
    static int iarray[LARGE_SIZE];
    static char carray[LARGE_SIZE * 2];
    
    /* Initialize with volatile to prevent pre-computation */
    volatile int init_val = time(NULL) % 100;
    
    for (int i = 0; i < MEDIUM_SIZE; i++) {
        darray[i] = (i + init_val) * 1.5;
    }
    
    for (int i = 0; i < LARGE_SIZE; i++) {
        iarray[i] = i * init_val;
        if (i % 8 == 0) {
            carray[i] = (i + init_val) % 256;
        }
    }
    
    /* Cross-type operations */
    for (int i = 0; i < MEDIUM_SIZE; i++) {
        iarray[i] = (int)(darray[i] * 2.0);
    }
}

/* Function with branching to create complex control flow */
void branching_cache_access(int* data, int size, int threshold) {
    int sum1 = 0, sum2 = 0;
    
    for (int i = 0; i < size; i++) {
        if (data[i] > threshold) {
            /* Hot path - sequential access */
            sum1 += data[i];
            data[i] = data[i] * 2;
        } else {
            /* Cold path - strided access */
            sum2 += data[i % 1024];
            if (i % 7 == 0) {
                data[i] = data[i] / 2;
            }
        }
        
        /* Periodic flush-like behavior */
        if (i % 512 == 0) {
            for (int j = 0; j < 64; j++) {
                data[(i + j) % size] = j;
            }
        }
    }
    
    /* Use results to prevent elimination */
    if (sum1 > sum2) {
        data[0] = sum1 - sum2;
    }
}

int main(int argc, char** argv) {
    /* Use command line args to create runtime variability */
    int matrix_size = (argc > 1) ? atoi(argv[1]) : 100;
    if (matrix_size > 500) matrix_size = 500;
    if (matrix_size < 50) matrix_size = 50;
    
    /* Dynamic allocation to force heap access patterns */
    double* matA = (double*)malloc(matrix_size * matrix_size * sizeof(double));
    double* matB = (double*)malloc(matrix_size * matrix_size * sizeof(double));
    double* matC = (double*)malloc(matrix_size * matrix_size * sizeof(double));
    
    int* int_data = (int*)malloc(LARGE_SIZE * sizeof(int));
    char* char_src = (char*)malloc(LARGE_SIZE * sizeof(char));
    char* char_dst = (char*)malloc(LARGE_SIZE * sizeof(char));
    
    if (!matA || !matB || !matC || !int_data || !char_src || !char_dst) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    srand(time(NULL));
    for (int i = 0; i < matrix_size * matrix_size; i++) {
        matA[i] = (double)rand() / RAND_MAX;
        matB[i] = (double)rand() / RAND_MAX;
    }
    
    for (int i = 0; i < LARGE_SIZE; i++) {
        int_data[i] = rand() % 1000;
        char_src[i] = rand() % 256;
    }
    
    /* Execute various cache-intensive patterns */
    
    /* 1. Matrix multiplication pattern */
    matrix_style_operations(matrix_size, matA, matB, matC);
    
    /* 2. Non-unit stride access */
    stride_access_pattern(int_data, LARGE_SIZE, stride);
    
    /* 3. Cache line operations */
    cache_line_aliasing(char_src, char_dst, LARGE_SIZE);
    
    /* 4. Mixed data type operations */
    mixed_data_operations();
    
    /* 5. Branching with cache access */
    branching_cache_access(int_data, LARGE_SIZE, 500);
    
    /* Compute and print a result to prevent dead code elimination */
    double total = 0.0;
    for (int i = 0; i < matrix_size * matrix_size; i += matrix_size + 1) {
        total += matC[i];
    }
    
    int int_sum = 0;
    for (int i = 0; i < LARGE_SIZE; i += 128) {
        int_sum += int_data[i];
    }
    
    printf("Results: matrix_diag_sum = %.6f, strided_int_sum = %d\n", 
           total / (matrix_size * 1.0), int_sum);
    
    /* Cleanup */
    free(matA);
    free(matB);
    free(matC);
    free(int_data);
    free(char_src);
    free(char_dst);
    
    return 0;
}
