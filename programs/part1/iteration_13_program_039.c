/* cache_detection_test.c
 * Designed to trigger GCC driver's CPU cache detection logic
 * for x86/x86-64 targets, specifically targeting uncovered
 * cache descriptor switch cases in driver-i386.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Volatile variables to prevent compile-time optimization */
volatile int outer_limit = 1000;
volatile int inner_limit = 500;
volatile int stride_val = 16;

/* Large arrays to exceed typical L1/L2 cache sizes */
#define LARGE_SIZE 10000
#define MEDIUM_SIZE 5000
#define SMALL_SIZE 1000

/* Function prototypes to force different optimization contexts */
void matrix_multiply_style(int n, volatile int limit);
void stride_access_pattern(int *arr, int size, int stride);
void cache_line_aliasing_test(double *src, double *dst, int size);
int compute_checksum(int *data, int size);

int main(int argc, char *argv[]) {
    /* Use command line args to vary behavior and prevent optimization */
    int use_heap = (argc > 1) ? atoi(argv[1]) : 1;
    int iter_count = (argc > 2) ? atoi(argv[2]) : 5;
    
    printf("Starting cache detection test...\n");
    
    /* Conditional compilation for different x86 architectures */
    #ifdef __x86_64__
    printf("Compiled for x86_64 architecture\n");
    /* Force consideration of different cache configurations */
    #if defined(__AVX__) || defined(__SSE4_2__)
    printf("Using vector extensions that require cache-aware optimizations\n");
    #endif
    #endif
    
    #ifdef __i386__
    printf("Compiled for i386 architecture\n");
    #endif
    
    /* Allocate arrays with different sizes and types */
    int *int_array1, *int_array2;
    double *double_array1, *double_array2;
    
    if (use_heap) {
        /* Heap allocation - size unknown at compile time */
        int_array1 = (int*)malloc(LARGE_SIZE * sizeof(int));
        int_array2 = (int*)malloc(LARGE_SIZE * sizeof(int));
        double_array1 = (double*)malloc(MEDIUM_SIZE * sizeof(double));
        double_array2 = (double*)malloc(MEDIUM_SIZE * sizeof(double));
    } else {
        /* Static allocation - still large enough to exceed caches */
        static int static_int_array1[LARGE_SIZE];
        static int static_int_array2[LARGE_SIZE];
        static double static_double_array1[MEDIUM_SIZE];
        static double static_double_array2[MEDIUM_SIZE];
        int_array1 = static_int_array1;
        int_array2 = static_int_array2;
        double_array1 = static_double_array1;
        double_array2 = static_double_array2;
    }
    
    /* Initialize arrays with pseudo-random but deterministic values */
    srand(42);
    for (int i = 0; i < LARGE_SIZE; i++) {
        int_array1[i] = rand() % 100;
        if (i < MEDIUM_SIZE) {
            double_array1[i] = (double)(rand() % 100) / 3.0;
        }
    }
    
    int total_checksum = 0;
    
    /* Execute multiple optimization patterns in sequence */
    for (int iter = 0; iter < iter_count; iter++) {
        printf("Iteration %d:\n", iter + 1);
        
        /* Pattern 1: Matrix multiplication style triple nested loop */
        matrix_multiply_style(SMALL_SIZE, outer_limit / (iter + 1));
        
        /* Pattern 2: Non-unit stride access */
        stride_access_pattern(int_array1, LARGE_SIZE, stride_val * (iter + 1));
        
        /* Pattern 3: Cache line aliasing test */
        cache_line_aliasing_test(double_array1, double_array2, MEDIUM_SIZE);
        
        /* Copy with potential cache conflicts */
        memcpy(int_array2, int_array1, LARGE_SIZE * sizeof(int));
        
        /* Compute checksum to ensure computations aren't optimized away */
        int checksum = compute_checksum(int_array2, LARGE_SIZE);
        total_checksum += checksum;
        
        /* Vary parameters to force different optimization decisions */
        stride_val = (stride_val * 3) % 32 + 1;
    }
    
    printf("Total checksum: %d\n", total_checksum);
    
    if (use_heap) {
        free(int_array1);
        free(int_array2);
        free(double_array1);
        free(double_array2);
    }
    
    return total_checksum != 0 ? 0 : 1;
}

/* Triple nested loop similar to matrix multiplication
 * Compiler should consider cache blocking for this pattern */
void matrix_multiply_style(int n, volatile int limit) {
    /* Use local arrays to force stack usage */
    double a[256][256];
    double b[256][256];
    double c[256][256];
    
    /* Initialize */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            a[i][j] = (double)(i + j) / n;
            b[i][j] = (double)(i * j) / n;
            c[i][j] = 0.0;
        }
    }
    
    /* Matrix multiplication kernel - compiler should optimize for cache */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += a[i][k] * b[k][j];
            }
            c[i][j] = sum;
        }
    }
    
    /* Use result to prevent dead code elimination */
    volatile double dummy = 0.0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            dummy += c[i][j];
        }
    }
}

/* Access pattern with configurable stride
 * Tests compiler's ability to optimize for cache line size */
void stride_access_pattern(int *arr, int size, int stride) {
    volatile int result = 0;
    
    /* Forward stride access */
    for (int i = 0; i < size; i += stride) {
        arr[i] = arr[i] * 3 + 1;
        result += arr[i];
    }
    
    /* Reverse stride access */
    for (int i = size - 1; i >= 0; i -= stride) {
        arr[i] = arr[i] / 2 - 1;
        result -= arr[i];
    }
    
    /* Prevent optimization */
    if (result == 0) {
        arr[0] = 1;
    }
}

/* Test potential cache line aliasing */
void cache_line_aliasing_test(double *src, double *dst, int size) {
    /* Copy with offset that might cause cache conflicts */
    int offset = 64 / sizeof(double); /* Typical cache line boundary */
    
    for (int i = 0; i < size - offset; i++) {
        dst[i + offset] = src[i] * 2.0 - src[i + 1];
    }
    
    /* Second pass with different access pattern */
    for (int i = offset; i < size; i++) {
        src[i - offset] = dst[i] + src[i] / 2.0;
    }
}

/* Compute simple checksum */
int compute_checksum(int *data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum = (sum * 31 + data[i]) % 1000000;
    }
    return sum;
}
