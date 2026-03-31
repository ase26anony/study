/* cache_detection_test.c
 * Designed to trigger GCC driver's CPU cache detection logic
 * for x86 cache descriptor values: 0x0a, 0x0c, 0x0d, 0x0e, 0x21,
 * 0x24, 0x2c, 0x39-0x3e, 0x41-0x45, 0x48-0x49, 0x4e, 0x60,
 * 0x66-0x68, 0x78-0x80, 0x82-0x87
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Force compiler to consider cache characteristics by using
 * arrays that exceed typical L1/L2 cache sizes */
#define LARGE_SIZE 10000
#define MEDIUM_SIZE 5000
#define SMALL_SIZE 1000

/* Volatile variables to prevent compile-time optimization */
volatile int outer_limit = LARGE_SIZE;
volatile int inner_limit = MEDIUM_SIZE;
volatile int stride = 16;

/* Function prototypes to force different optimization contexts */
void matrix_multiply_kernel(int n, double *restrict A, double *restrict B, double *restrict C);
void stride_access_pattern(int size, int stride, int *array);
void cache_line_aliasing_test(int size, char *src, char *dst);
void mixed_data_type_operations(int iterations);

/* Main computational kernel - matrix multiplication style */
void matrix_multiply_kernel(int n, double *restrict A, double *restrict B, double *restrict C) {
    /* Triple nested loop - compiler may apply cache-aware optimizations */
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
void stride_access_pattern(int size, int stride, int *array) {
    /* Access every Nth element to test cache line utilization */
    for (int i = 0; i < size; i += stride) {
        array[i] = array[i] * 3 + 7;
    }
    
    /* Reverse stride pattern */
    for (int i = size - 1; i >= 0; i -= stride / 2) {
        array[i] = array[i] / 2;
    }
}

/* Potential cache line aliasing scenario */
void cache_line_aliasing_test(int size, char *src, char *dst) {
    /* Copy with potential cache line conflicts */
    for (int i = 0; i < size; i += 64) {  /* Common cache line size */
        for (int j = 0; j < 64 && (i + j) < size; j++) {
            dst[i + j] = src[i + j] ^ 0x55;  /* Simple transformation */
        }
    }
}

/* Mixed data type operations to vary access patterns */
void mixed_data_type_operations(int iterations) {
    static double darray[MEDIUM_SIZE];
    static int iarray[LARGE_SIZE];
    static char carray[LARGE_SIZE * 2];
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < MEDIUM_SIZE; i++) {
        darray[i] = (i % 100) * 0.01;
    }
    
    for (int i = 0; i < LARGE_SIZE; i++) {
        iarray[i] = i * 3;
        carray[i] = (i % 256);
    }
    
    /* Mixed type computation loop */
    double sum = 0.0;
    for (int iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < MEDIUM_SIZE; i++) {
            /* Mix double and int operations */
            darray[i] = darray[i] * (iarray[i % LARGE_SIZE] + 1);
            sum += darray[i];
            
            /* Char array access with different stride */
            if (i % 4 == 0) {
                carray[i * 2] = (char)(sum * 100);
            }
        }
    }
    
    /* Prevent dead code elimination */
    if (sum < 0) {
        printf("Unexpected negative sum: %f\n", sum);
    }
}

int main(int argc, char *argv[]) {
    /* Use command line args to vary behavior and prevent optimization */
    int matrix_size = (argc > 1) ? atoi(argv[1]) : 200;
    int num_iterations = (argc > 2) ? atoi(argv[2]) : 5;
    
    /* Large heap allocations to exceed cache sizes */
    double *matrixA = (double*)malloc(matrix_size * matrix_size * sizeof(double));
    double *matrixB = (double*)malloc(matrix_size * matrix_size * sizeof(double));
    double *matrixC = (double*)malloc(matrix_size * matrix_size * sizeof(double));
    
    int *large_int_array = (int*)malloc(LARGE_SIZE * sizeof(int));
    char *char_src = (char*)malloc(LARGE_SIZE * 2);
    char *char_dst = (char*)malloc(LARGE_SIZE * 2);
    
    if (!matrixA || !matrixB || !matrixC || !large_int_array || !char_src || !char_dst) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < matrix_size * matrix_size; i++) {
        matrixA[i] = (i % 100) * 0.01;
        matrixB[i] = (i % 50) * 0.02;
    }
    
    for (int i = 0; i < LARGE_SIZE; i++) {
        large_int_array[i] = i;
        char_src[i] = (i % 256);
    }
    
    printf("Starting cache-sensitive computations...\n");
    
    /* Execute different cache-sensitive patterns */
    
    /* 1. Matrix multiplication kernel */
    matrix_multiply_kernel(matrix_size, matrixA, matrixB, matrixC);
    
    /* 2. Non-unit stride access */
    stride_access_pattern(LARGE_SIZE, stride, large_int_array);
    
    /* 3. Cache line aliasing test */
    cache_line_aliasing_test(LARGE_SIZE * 2, char_src, char_dst);
    
    /* 4. Mixed data type operations */
    mixed_data_type_operations(num_iterations);
    
    /* Compute a result to prevent dead code elimination */
    double final_result = 0.0;
    for (int i = 0; i < matrix_size; i++) {
        final_result += matrixC[i * matrix_size + i];
    }
    
    for (int i = 0; i < LARGE_SIZE; i += 100) {
        final_result += large_int_array[i];
    }
    
    printf("Final result: %f\n", final_result);
    
    /* Cleanup */
    free(matrixA);
    free(matrixB);
    free(matrixC);
    free(large_int_array);
    free(char_src);
    free(char_dst);
    
    return 0;
}

/* Conditional compilation for different x86 architectures */
#ifdef __x86_64__
/* Code block that might benefit from Nehalem-specific optimizations */
void nehalem_optimized_path(void) {
    /* This code path might trigger cache descriptor 0x49 */
    volatile int array[4096];
    for (int i = 0; i < 4096; i++) {
        array[i] = array[i] + i;
    }
}
#endif

#ifdef __i386__
/* Code block for 32-bit x86 with different cache characteristics */
void i386_optimized_path(void) {
    /* Might trigger older cache descriptors */
    volatile short array[8192];
    for (int i = 0; i < 8192; i += 2) {
        array[i] = array[i] * 2;
    }
}
#endif
