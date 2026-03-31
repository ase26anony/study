/* test_cache_detection.c
 * 
 * This program is designed to trigger GCC's internal CPU cache detection
 * logic by using compiler flags that require cache-aware optimizations
 * and by providing workload patterns that benefit from cache tuning.
 * 
 * Compile with various x86-specific flags to exercise different cache
 * descriptor cases in driver-i386.cc (lines 127-244).
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Use volatile to prevent compile-time optimization of loop bounds */
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
    
    /* Triple nested loop - common pattern for cache optimization */
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            int temp = 0;
            for (k = 0; k < n; k++) {
                /* Access with different strides to test various cache behaviors */
                temp += array1[i * stride + k] * array2[k * stride + j];
            }
            darray1[i * n + j] = temp;
        }
    }
}

/* Non-unit stride access pattern */
void stride_access_pattern(int size, int step) {
    volatile int i;
    double acc = 0.0;
    
    /* Access every 'step'-th element - tests cache line utilization */
    for (i = 0; i < size; i += step) {
        acc += darray1[i] * 1.5;
        darray2[i] = acc;
    }
    
    /* Print to prevent dead code elimination */
    if (acc > 1000000) {
        printf("Stride pattern result: %f\n", acc);
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
void mixed_operations(int iterations) {
    volatile int i, j;
    
    for (i = 0; i < iterations; i++) {
        /* Mix int, double and char operations */
        double dval = darray1[i % 5000];
        int ival = array1[i % LARGE_SIZE];
        char cval = carray[i % 20000];
        
        /* Complex enough to prevent simple optimizations */
        darray2[i % 5000] = dval * ival + (cval / 256.0);
        
        /* Conditional to create branch prediction patterns */
        if (i % 100 == 0) {
            array2[i % LARGE_SIZE] = ival * 2;
        }
    }
}

/* Initialize arrays with pseudo-random data */
void init_arrays(void) {
    volatile int i;
    
    srand(time(NULL));
    for (i = 0; i < LARGE_SIZE; i++) {
        array1[i] = rand() % 100;
        array2[i] = rand() % 100;
    }
    
    for (i = 0; i < 5000; i++) {
        darray1[i] = (double)(rand() % 1000) / 10.0;
        darray2[i] = 0.0;
    }
    
    for (i = 0; i < 20000; i++) {
        carray[i] = (char)(rand() % 256);
    }
}

int main(int argc, char *argv[]) {
    /* Use arguments to vary behavior and prevent constant propagation */
    int matrix_size = (argc > 1) ? atoi(argv[1]) : 50;
    int stride_val = (argc > 2) ? atoi(argv[2]) : 8;
    
    if (matrix_size <= 0) matrix_size = 50;
    if (stride_val <= 0) stride_val = 8;
    
    init_arrays();
    
    /* Execute different computation patterns */
    matrix_style_computation(matrix_size);
    stride_access_pattern(5000, stride_val);
    copy_with_aliasing(LARGE_SIZE / 2);
    mixed_operations(10000);
    
    /* Final computation and output to ensure side effects */
    double final_sum = 0.0;
    volatile int i;
    for (i = 0; i < 1000; i++) {
        final_sum += darray2[i % 5000];
    }
    
    printf("Final result: %f\n", final_sum);
    
    /* Conditional compilation for different architectures */
    #ifdef __x86_64__
    /* Code that might benefit from specific x86-64 cache optimizations */
    printf("x86_64 architecture detected\n");
    #endif
    
    #ifdef __i386__
    /* Code for 32-bit x86 */
    printf("i386 architecture detected\n");
    #endif
    
    /* Explicit CPU feature detection if available */
    #ifdef __GNUC__
    /* These builtins may trigger CPUID queries */
    __builtin_cpu_init();
    if (__builtin_cpu_supports("sse2")) {
        printf("SSE2 supported\n");
    }
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported\n");
    }
    #endif
    
    return 0;
}
