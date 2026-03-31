/* cache_detection_test.c
 * Designed to trigger GCC's internal CPU cache detection logic
 * for x86/x86-64 targets, specifically targeting the switch cases
 * for cache descriptor values in driver-i386.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/* Use volatile to prevent compile-time optimization of loop bounds */
volatile int outer_limit = 1000;
volatile int inner_limit = 500;
volatile int stride = 8;

/* Large arrays that will exceed L1 cache */
#define LARGE_SIZE 10000
static int array1[LARGE_SIZE];
static int array2[LARGE_SIZE];
static double darray1[LARGE_SIZE/2];
static double darray2[LARGE_SIZE/2];
static char carray[LARGE_SIZE * 4];

/* Matrix multiplication style computation */
void matrix_style_computation(int n) {
    volatile int i, j, k;
    int sum = 0;
    
    /* Triple nested loop - common pattern for cache optimization */
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            int temp = 0;
            for (k = 0; k < n; k++) {
                /* Access with different strides */
                temp += array1[i * n + k] * array2[k * n + j];
            }
            array1[i * n + j] = temp;
            sum += temp;
        }
    }
    
    /* Prevent dead code elimination */
    if (sum == 0) printf("Zero sum\n");
}

/* Non-unit stride access pattern */
void stride_access_computation(int limit, int stride_val) {
    volatile int i;
    double acc = 0.0;
    
    /* Access every stride_val-th element */
    for (i = 0; i < limit; i += stride_val) {
        darray1[i] = darray2[i] * 1.5;
        acc += darray1[i];
        
        /* Mix in some integer operations */
        array1[i] = (int)(darray1[i] * 100);
    }
    
    /* Cross-array access with potential cache line effects */
    for (i = 0; i < limit - 64; i++) {
        carray[i] = carray[i + 64] ^ 0x55;  /* Potential cache line aliasing */
    }
    
    if (acc > 1e10) printf("Large accumulation\n");
}

/* Copy with potential cache conflicts */
void cache_line_copy(int size) {
    volatile int i;
    
    /* Copy between arrays with same cache line alignment */
    for (i = 0; i < size; i++) {
        array2[i] = array1[i] * 2 + 1;
    }
    
    /* Reverse copy to cause more cache traffic */
    for (i = size - 1; i >= 0; i--) {
        array1[i] = array2[size - 1 - i] / 2;
    }
}

/* Initialize arrays with pseudo-random data */
void init_arrays(void) {
    volatile int i;
    for (i = 0; i < LARGE_SIZE; i++) {
        array1[i] = i % 100;
        array2[i] = (i * 3) % 100;
    }
    for (i = 0; i < LARGE_SIZE/2; i++) {
        darray1[i] = i * 0.5;
        darray2[i] = i * 0.3;
    }
    for (i = 0; i < LARGE_SIZE * 4; i++) {
        carray[i] = i % 256;
    }
}

int main(int argc, char **argv) {
    volatile int matrix_size = 64;  /* Small enough for L1, but patterns matter */
    volatile int copy_size = 5000;
    
    /* Initialize data */
    init_arrays();
    
    printf("Starting cache-sensitive computations...\n");
    
    /* Execute different computation patterns */
    
    /* Pattern 1: Matrix-style (good for tiling optimizations) */
    matrix_style_computation(matrix_size);
    
    /* Pattern 2: Stride access (tests prefetching) */
    stride_access_computation(inner_limit, stride);
    
    /* Pattern 3: Cache line copying (tests line utilization) */
    cache_line_copy(copy_size);
    
    /* Pattern 4: Mixed data type operations */
    {
        volatile int i, j;
        double mixed_acc = 0.0;
        
        for (i = 0; i < outer_limit; i++) {
            for (j = 0; j < 100; j++) {
                /* Mix int and double operations */
                double temp = darray1[i % (LARGE_SIZE/2)] * array1[j];
                darray2[j % (LARGE_SIZE/2)] = temp;
                mixed_acc += temp;
                
                /* Char array access with bit manipulation */
                carray[(i * 16 + j) % (LARGE_SIZE * 4)] ^= (char)temp;
            }
        }
        
        printf("Mixed accumulation result: %f\n", mixed_acc);
    }
    
    /* Final computation that uses all arrays */
    {
        volatile long final_sum = 0;
        volatile int i;
        
        for (i = 0; i < LARGE_SIZE; i++) {
            final_sum += array1[i] + array2[i];
            if (i < LARGE_SIZE/2) {
                final_sum += (long)darray1[i];
                final_sum += (long)darray2[i];
            }
        }
        
        printf("Final checksum: %ld\n", final_sum);
    }
    
    /* Conditional compilation for different architectures */
#ifdef __x86_64__
    printf("Compiled for x86-64 architecture\n");
    /* Additional x86-64 specific patterns */
    {
        volatile long i;
        for (i = 0; i < 1000000; i++) {
            /* Empty loop that might be optimized differently */
            asm volatile("" : : : "memory");
        }
    }
#endif
    
#ifdef __i386__
    printf("Compiled for i386 architecture\n");
#endif
    
    /* Explicit CPU feature detection if available */
#ifdef __GNUC__
    /* These builtins might trigger CPUID queries */
    __builtin_cpu_init();
    if (__builtin_cpu_supports("sse")) {
        printf("SSE supported\n");
    }
    if (__builtin_cpu_supports("sse2")) {
        printf("SSE2 supported\n");
    }
#endif
    
    return 0;
}
