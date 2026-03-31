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
void matrix_multiply_kernel(int n, double *restrict A, 
                           double *restrict B, double *restrict C);
void stride_access_pattern(int size, int stride, int *array);
void cache_line_aliasing_test(int size, char *src, char *dst);

int main(int argc, char *argv[]) {
    /* Use command line args to vary parameters at runtime */
    int matrix_size = (argc > 1) ? atoi(argv[1]) : 500;
    int test_iterations = (argc > 2) ? atoi(argv[2]) : 10;
    
    /* Allocate arrays that will stress cache hierarchy */
    double *matrixA = (double*)aligned_alloc(64, matrix_size * matrix_size * sizeof(double));
    double *matrixB = (double*)aligned_alloc(64, matrix_size * matrix_size * sizeof(double));
    double *matrixC = (double*)aligned_alloc(64, matrix_size * matrix_size * sizeof(double));
    
    int *int_array = (int*)malloc(LARGE_SIZE * sizeof(int));
    char *char_src = (char*)malloc(MEDIUM_SIZE * sizeof(char) * 64);
    char *char_dst = (char*)malloc(MEDIUM_SIZE * sizeof(char) * 64);
    
    if (!matrixA || !matrixB || !matrixC || !int_array || !char_src || !char_dst) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random but deterministic values */
    srand(42);
    for (int i = 0; i < matrix_size * matrix_size; i++) {
        matrixA[i] = (double)rand() / RAND_MAX;
        matrixB[i] = (double)rand() / RAND_MAX;
        matrixC[i] = 0.0;
    }
    
    for (int i = 0; i < LARGE_SIZE; i++) {
        int_array[i] = rand() % 100;
    }
    
    memset(char_src, 'A', MEDIUM_SIZE * 64);
    memset(char_dst, 0, MEDIUM_SIZE * 64);
    
    double total_result = 0.0;
    
    /* Execute different loop patterns that benefit from cache-aware optimizations */
    for (int iter = 0; iter < test_iterations; iter++) {
        /* Pattern 1: Matrix multiplication - stresses cache blocking */
        matrix_multiply_kernel(matrix_size, matrixA, matrixB, matrixC);
        
        /* Extract a result to prevent dead code elimination */
        for (int i = 0; i < matrix_size; i++) {
            total_result += matrixC[i * matrix_size + i];
        }
        
        /* Pattern 2: Non-unit stride access - tests prefetching */
        stride_access_pattern(LARGE_SIZE, stride + iter, int_array);
        
        /* Pattern 3: Cache line aliasing test */
        cache_line_aliasing_test(MEDIUM_SIZE, char_src, char_dst);
        
        /* Vary stride to create different access patterns */
        stride = (stride * 13 + 7) % 32;
    }
    
    /* Mix data types in nested loops to trigger different optimizations */
    {
        double sum = 0.0;
        int count = 0;
        
        /* Complex nested loop with mixed operations */
        for (int i = 0; i < outer_limit; i += 64) {
            for (int j = 0; j < inner_limit; j += 8) {
                for (int k = 0; k < SMALL_SIZE; k++) {
                    /* Mix int and double operations */
                    int idx = (i * 31 + j * 17 + k * 7) % LARGE_SIZE;
                    double val = int_array[idx] * matrixA[(i * j) % (matrix_size * matrix_size)];
                    sum += val;
                    count++;
                    
                    /* Conditional to prevent vectorization simplification */
                    if (val > 1000.0) {
                        int_array[idx] = (int)(val / 1000.0);
                    }
                }
            }
        }
        
        total_result += sum / (count + 1);
    }
    
    printf("Final result: %f\n", total_result);
    
    /* Cleanup */
    free(matrixA);
    free(matrixB);
    free(matrixC);
    free(int_array);
    free(char_src);
    free(char_dst);
    
    return 0;
}

/* Matrix multiplication kernel - will benefit from cache blocking */
void matrix_multiply_kernel(int n, double *restrict A, 
                           double *restrict B, double *restrict C) {
    /* Triple nested loop - prime target for cache-aware optimizations */
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

/* Non-unit stride access pattern - tests hardware prefetching */
void stride_access_pattern(int size, int stride, int *array) {
    int sum = 0;
    /* Access every stride-th element */
    for (int i = 0; i < size; i += stride) {
        sum += array[i];
        /* Modulo operation to create dependency chain */
        array[i] = (array[i] * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Prevent dead code elimination */
    array[0] = sum;
}

/* Test potential cache line aliasing */
void cache_line_aliasing_test(int size, char *src, char *dst) {
    /* Copy with potential aliasing */
    for (int i = 0; i < size * 64; i += 64) {
        /* Copy one cache line at a time */
        for (int j = 0; j < 64; j++) {
            dst[i + j] = src[i + j] + 1;
        }
    }
    
    /* Reverse copy to create conflict */
    for (int i = size * 64 - 64; i >= 0; i -= 64) {
        for (int j = 63; j >= 0; j--) {
            src[i + j] = dst[i + j] - 1;
        }
    }
}

/* Conditional compilation for different x86 architectures */
#ifdef __x86_64__
/* Code that might benefit from specific x86 cache optimizations */
void x86_specific_optimizations() {
    /* Use builtins to explicitly trigger CPU detection */
    __builtin_cpu_init();
    
    /* Check for features that might influence cache detection */
    if (__builtin_cpu_supports("sse2")) {
        /* SSE2-optimized path */
    }
    
    if (__builtin_cpu_supports("avx")) {
        /* AVX-optimized path */
    }
}
#endif

/* Alternative implementation for different -march targets */
#if defined(__i386__) || defined(__x86_64__)
void cache_sensitive_workload() {
    /* Workload that varies based on assumed cache sizes */
    static int buffer[1024 * 1024];  /* 4MB - exceeds most L2 caches */
    
    /* Fibonacci-like access pattern */
    int a = 0, b = 1;
    for (int i = 0; i < 1000000; i++) {
        int idx = (a * b) % (1024 * 1024);
        buffer[idx] = a + b;
        int temp = a + b;
        a = b;
        b = temp % 1000;
    }
}
#endif
