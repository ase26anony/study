/* cache_detection_test.c
 * Designed to trigger CPU cache detection logic in GCC driver
 * Compile with various x86 -march and -mtune options to exercise
 * cache descriptor switch cases in driver-i386.cc
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
static char carray1[20000];
static char carray2[20000];

/* Matrix multiplication style triple nested loop */
void matrix_style_loop(int n) {
    volatile int i, j, k;
    int sum = 0;
    
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
void stride_access_loop(int limit, int step) {
    volatile int i;
    double acc = 0.0;
    
    for (i = 0; i < limit; i += step) {
        /* Access every Nth element to test cache line utilization */
        acc += darray1[i] * 1.5;
        darray2[i] = acc;
        
        /* Mix in char array access for different data sizes */
        carray1[i % 20000] = (char)(acc * 0.01);
    }
}

/* Copy with potential cache line aliasing */
void copy_with_aliasing(int size, int offset) {
    volatile int i;
    
    for (i = 0; i < size; i++) {
        /* Potential cache line conflicts */
        array2[(i + offset) % LARGE_SIZE] = array1[i] * 2;
        
        /* Additional computation to prevent simple copy elimination */
        if (i % 8 == 0) {
            carray2[i % 20000] = carray1[i % 20000] ^ 0x55;
        }
    }
}

/* Initialize arrays with pseudo-random data */
void init_arrays(void) {
    volatile int i;
    
    for (i = 0; i < LARGE_SIZE; i++) {
        array1[i] = (i * 3) % 97;
        array2[i] = (i * 7) % 113;
    }
    
    for (i = 0; i < 5000; i++) {
        darray1[i] = (i * 0.3) - 25.0;
        darray2[i] = (i * 0.7) + 10.0;
    }
    
    for (i = 0; i < 20000; i++) {
        carray1[i] = (char)(i % 256);
        carray2[i] = 0;
    }
}

int main(int argc, char *argv[]) {
    volatile int matrix_size = 64;  /* Prevent constant propagation */
    volatile int copy_offset = 32;  /* Force runtime evaluation */
    
    /* Initialize with different patterns */
    init_arrays();
    
    /* Execute different loop patterns that benefit from cache optimization */
    
    /* Pattern 1: Matrix-style computation */
    matrix_style_loop(matrix_size);
    
    /* Pattern 2: Strided access */
    stride_access_loop(inner_limit * 10, stride);
    
    /* Pattern 3: Copy with offset causing potential cache conflicts */
    copy_with_aliasing(outer_limit * 5, copy_offset);
    
    /* Additional mixed pattern */
    volatile int i, j;
    double total = 0.0;
    
    for (i = 0; i < outer_limit; i++) {
        for (j = 0; j < inner_limit; j++) {
            /* Complex addressing to defeat simple analysis */
            int idx = (i * 17 + j * 23) % LARGE_SIZE;
            total += array1[idx] * 0.5 + darray1[j % 5000];
            
            /* Conditional store to prevent vectorization in some cases */
            if (total > 1000.0) {
                array2[idx] = (int)total;
                total *= 0.9;
            }
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Cache test result: %f\n", total);
    printf("Array checksum: %d %d\n", 
           array1[LARGE_SIZE/2] + array2[LARGE_SIZE/3],
           (int)darray1[1000] + (int)darray2[2000]);
    
    return 0;
}
