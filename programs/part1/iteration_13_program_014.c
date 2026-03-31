/* cache_detection_test.c
 * 
 * This program is designed to trigger GCC's internal CPU cache detection
 * logic by using computational patterns that benefit from cache-aware
 * optimizations and compiling with x86-specific tuning options.
 * 
 * Compile with various -march and -mtune options to exercise different
 * cache descriptor cases in driver-i386.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Use volatile to prevent compile-time optimization */
volatile int outer_limit = 1000;
volatile int inner_limit = 1000;
volatile int stride = 16;

/* Large arrays to exceed L1 cache */
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
    
    /* Triple nested loop - typical matrix multiplication pattern */
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            int temp = 0;
            for (k = 0; k < n; k++) {
                /* Access with different strides to test cache behavior */
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
    
    /* Access every 'step'-th element to test cache line utilization */
    for (i = 0; i < limit; i += step) {
        acc += darray1[i] * 1.5;
        darray2[i] = acc;
        
        /* Also access neighboring elements occasionally */
        if (i % 32 == 0 && i + 1 < limit) {
            darray2[i + 1] = darray1[i] * 0.5;
        }
    }
}

/* Copy with potential cache line aliasing */
void copy_with_aliasing(int size, int offset) {
    volatile int i;
    
    /* Copy between arrays with offset that might cause cache conflicts */
    for (i = 0; i < size; i++) {
        array2[i + offset] = array1[i] * 2;
        
        /* Mix in char array access to vary access sizes */
        carray[i % 20000] = (char)(array1[i] & 0xFF);
    }
}

/* Initialize arrays with pseudo-random but deterministic values */
void init_arrays(void) {
    volatile int i;
    for (i = 0; i < LARGE_SIZE; i++) {
        array1[i] = (i * 13 + 7) % 100;
        array2[i] = (i * 17 + 11) % 100;
    }
    for (i = 0; i < 5000; i++) {
        darray1[i] = (i * 1.5) / (i + 1.0);
        darray2[i] = 0.0;
    }
}

int main(int argc, char *argv[]) {
    volatile int n = 100;  /* Prevent constant propagation */
    volatile int result = 0;
    
    /* Initialize with deterministic but non-constant values */
    init_arrays();
    
    /* Execute different computation patterns */
    matrix_style_computation(n);
    stride_access_computation(5000, 8);
    copy_with_aliasing(3000, 64);
    
    /* Additional computation to use results */
    for (int i = 0; i < 100; i++) {
        result += array1[i * 10] + (int)darray1[i * 5];
    }
    
    /* Use result to prevent dead code elimination */
    printf("Computation result: %d\n", result);
    
    /* Conditional compilation for different architectures */
#ifdef __x86_64__
    printf("x86_64 architecture detected\n");
    /* Code that might benefit from specific x86_64 optimizations */
    __asm__ volatile ("" : : : "memory");  /* Memory barrier */
#endif
    
#ifdef __i386__
    printf("i386 architecture detected\n");
    /* 32-bit specific patterns */
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
