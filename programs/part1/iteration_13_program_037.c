/* test_cache_detection.c
 * Designed to trigger CPU cache detection logic in GCC driver
 * Compile with various x86 -march and -mtune options to exercise
 * the cache descriptor switch cases in driver-i386.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Use volatile to prevent compile-time optimization */
volatile int outer_limit = 1000;
volatile int inner_limit = 1000;
volatile int stride = 16;

/* Large arrays that exceed typical L1 cache sizes */
#define LARGE_SIZE 10000
static int array1[LARGE_SIZE];
static int array2[LARGE_SIZE];
static double darray1[5000];
static double darray2[5000];
static char carray[20000];

/* Matrix multiplication style computation */
void matrix_style_computation(int n) {
    volatile int i, j, k;
    int sum = 0;
    
    /* Triple nested loop - typical for cache optimization */
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            int temp = 0;
            for (k = 0; k < n; k++) {
                /* Access with different strides */
                temp += array1[i * stride + k] * array2[k * stride + j];
            }
            darray1[i * n + j] = temp;
        }
    }
}

/* Non-unit stride access pattern */
void stride_access_computation(int limit, int step) {
    volatile int i;
    double acc = 0.0;
    
    /* Access every Nth element - tests cache line utilization */
    for (i = 0; i < limit; i += step) {
        acc += darray1[i] * 1.5;
        darray2[i] = acc;
        
        /* Mix in some integer operations */
        array1[i] = (int)acc % 256;
    }
    
    /* Prevent dead code elimination */
    if (acc > 1e10) {
        printf("Accumulator large: %f\n", acc);
    }
}

/* Copy with potential cache line aliasing */
void copy_with_aliasing(int size) {
    volatile int i;
    
    /* Copy between arrays with offset - can cause cache conflicts */
    for (i = 0; i < size; i++) {
        int src_idx = i;
        int dst_idx = (i + 64) % size;  /* Offset by typical cache line size */
        array2[dst_idx] = array1[src_idx] + carray[i];
    }
}

/* Mixed data type operations */
void mixed_type_operations(int iterations) {
    volatile int i, j;
    
    for (i = 0; i < iterations; i++) {
        /* Convert between types - different access patterns */
        double dval = (double)array1[i];
        int ival = (int)darray1[i % 5000];
        char cval = (char)(ival % 256);
        
        /* Store in different arrays with different strides */
        carray[i * 2] = cval;
        array2[i] = ival;
        darray2[i % 5000] = dval;
        
        /* Some arithmetic to keep it non-trivial */
        if (i % 3 == 0) {
            darray1[i % 5000] = dval * 0.75 + ival * 0.25;
        }
    }
}

/* Initialize arrays with pseudo-random data */
void initialize_arrays(void) {
    volatile int i;
    
    for (i = 0; i < LARGE_SIZE; i++) {
        array1[i] = (i * 37) % 100;
        array2[i] = (i * 73) % 100;
    }
    
    for (i = 0; i < 5000; i++) {
        darray1[i] = (double)(i % 100) * 0.5;
        darray2[i] = (double)(i % 100) * 0.3;
    }
    
    for (i = 0; i < 20000; i++) {
        carray[i] = (char)(i % 128);
    }
}

int main(int argc, char *argv[]) {
    volatile int compute_size = 100;
    volatile int result = 0;
    
    /* Initialize with some data */
    initialize_arrays();
    
    /* 
     * Different code paths for different architectures
     * This encourages testing with various -march flags
     */
    
#ifdef __x86_64__
    /* Code optimized for x86-64 with various cache descriptors */
    printf("x86-64 architecture detected\n");
    
    /* Force use of cache-dependent optimizations */
    matrix_style_computation(compute_size);
    stride_access_computation(outer_limit, stride);
    
#elif defined(__i386__)
    /* Code for 32-bit x86 */
    printf("i386 architecture detected\n");
    
    /* Different computation patterns for 32-bit */
    copy_with_aliasing(inner_limit);
    mixed_type_operations(compute_size * 2);
    
#else
    /* Generic fallback */
    printf("Generic architecture\n");
    matrix_style_computation(50);
#endif

    /* 
     * Additional architecture-specific blocks to encourage
     * compilation with different -march settings
     */
    
#if defined(__tune_core2__) || defined(__tune_nocona__)
    /* Optimizations for Core 2 or Nocona processors */
    printf("Core2/Nocona tuning\n");
    for (int i = 0; i < 100; i++) {
        result += array1[i * 8] * array2[i * 8];  /* Larger stride */
    }
#endif

#if defined(__tune_nehalem__) || defined(__tune_westmere__)
    /* Nehalem/Westmere specific patterns */
    printf("Nehalem/Westmere tuning\n");
    stride_access_computation(2000, 8);
#endif

    /* Final computation using all arrays */
    for (int i = 0; i < 1000; i++) {
        result += array1[i] + (int)darray1[i % 500] + carray[i * 2];
    }
    
    printf("Final result: %d\n", result);
    
    /* 
     * Explicit CPU feature detection if available
     * This may prompt the driver to initialize cache detection
     */
#ifdef __GNUC__
    /* GCC builtins for CPU detection */
    __builtin_cpu_init();
    
    /* Check for specific features that require cache info */
    if (__builtin_cpu_supports("sse2") ||
        __builtin_cpu_supports("avx") ||
        __builtin_cpu_supports("avx2")) {
        printf("Vector extensions supported\n");
    }
#endif
    
    return result % 100;
}
