/* cache_detection_test.c
 * Designed to trigger GCC driver's CPU cache detection logic
 * for x86 cache descriptor values in driver-i386.cc lines 127-244
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Use volatile to prevent compile-time optimization */
volatile int outer_limit = 1000;
volatile int inner_limit = 500;
volatile int stride = 16;

/* Large arrays that exceed typical L1 cache sizes */
#define LARGE_SIZE 10000
static int array1[LARGE_SIZE];
static int array2[LARGE_SIZE];
static double darray1[LARGE_SIZE];
static double darray2[LARGE_SIZE];
static char carray1[LARGE_SIZE];
static char carray2[LARGE_SIZE];

/* Matrix for multiplication-like operations */
#define MATRIX_SIZE 128
static int matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static int matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static int matrix_c[MATRIX_SIZE][MATRIX_SIZE];

/* Function prototypes to force different optimization considerations */
void matrix_multiply(int size, volatile int limit);
void stride_access_pattern(int size, int stride_val);
void cache_line_test(int iterations);
void mixed_data_type_operations(void);

/* Main computational kernel with varied access patterns */
void matrix_multiply(int size, volatile int limit) {
    int i, j, k;
    int local_limit = limit % size;
    
    /* Triple nested loop - classic matrix multiplication pattern */
    for (i = 0; i < local_limit; i++) {
        for (j = 0; j < size; j++) {
            int sum = 0;
            for (k = 0; k < size; k++) {
                sum += matrix_a[i][k] * matrix_b[k][j];
            }
            matrix_c[i][j] = sum;
        }
    }
    
    /* Additional access pattern with different stride */
    for (i = 0; i < size; i += stride) {
        for (j = 0; j < size; j++) {
            matrix_a[i][j] = matrix_b[j][i] + matrix_c[i][j];
        }
    }
}

/* Non-unit stride access pattern */
void stride_access_pattern(int size, int stride_val) {
    int i, j;
    long long sum = 0;
    
    /* Access every Nth element to test cache line utilization */
    for (i = 0; i < size; i += stride_val) {
        for (j = 0; j < size; j++) {
            sum += array1[i * size + j];
            array2[j * size + i] = (int)(sum % 256);
        }
    }
    
    /* Reverse stride pattern */
    for (i = size - 1; i >= 0; i -= stride_val / 2) {
        for (j = 0; j < size; j += 2) {
            darray1[i] = darray2[j] * 1.5;
            darray2[j] = darray1[i] / 2.0;
        }
    }
}

/* Test potential cache line aliasing effects */
void cache_line_test(int iterations) {
    int i, j;
    volatile int temp = 0;
    
    /* Copy between arrays with potential cache conflicts */
    for (j = 0; j < iterations; j++) {
        for (i = 0; i < LARGE_SIZE - 64; i += 64) {
            /* Copy 64-byte blocks (typical cache line size) */
            memcpy(&carray2[i], &carray1[i], 64);
            temp += carray1[i];
        }
        
        /* Interleaved access pattern */
        for (i = 0; i < LARGE_SIZE; i += 128) {
            array1[i] = array2[i + 64] + temp;
            array2[i + 32] = array1[i] * 2;
        }
    }
}

/* Operations mixing different data types */
void mixed_data_type_operations(void) {
    int i;
    double dsum = 0.0;
    int isum = 0;
    
    /* Mixed type operations in same loop */
    for (i = 0; i < LARGE_SIZE; i++) {
        darray1[i] = (double)array1[i] * 1.41421356;
        dsum += darray1[i];
        
        array2[i] = (int)(darray2[i] * 100.0);
        isum += array2[i];
        
        carray1[i] = (char)((array1[i] + array2[i]) % 256);
    }
    
    /* Use results to prevent dead code elimination */
    if (dsum > 0.0) {
        array1[0] = (int)dsum;
    }
    if (isum > 0) {
        array2[0] = isum % 1000;
    }
}

/* Conditional compilation for different x86 architectures */
#ifdef __x86_64__
void x86_64_specific_operations(void) {
    /* Operations that might benefit from specific x86-64 optimizations */
    unsigned long long big_sum = 0;
    int i;
    
    for (i = 0; i < LARGE_SIZE; i++) {
        big_sum += (unsigned long long)array1[i] * array2[i];
    }
    
    /* Force use of 64-bit registers */
    asm volatile("" : "+r" (big_sum));
}
#endif

#ifdef __i386__
void i386_specific_operations(void) {
    /* 32-bit specific patterns */
    int i, j;
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            matrix_a[i][j] = matrix_b[j][i] * 3;
        }
    }
}
#endif

int main(int argc, char *argv[]) {
    int i, j;
    long long total_sum = 0;
    volatile int iterations = 10;
    
    /* Initialize arrays with pseudo-random but deterministic values */
    srand(42);
    for (i = 0; i < LARGE_SIZE; i++) {
        array1[i] = rand() % 100;
        array2[i] = rand() % 100;
        darray1[i] = (double)(rand() % 100) / 3.0;
        darray2[i] = (double)(rand() % 100) / 7.0;
        carray1[i] = (char)(rand() % 256);
    }
    
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = rand() % 50;
            matrix_b[i][j] = rand() % 50;
        }
    }
    
    /* Use command line args to vary parameters */
    if (argc > 1) {
        iterations = atoi(argv[1]) % 20 + 5;
    }
    if (argc > 2) {
        stride = atoi(argv[2]) % 32 + 4;
    }
    
    printf("Starting cache-sensitive computations...\n");
    
    /* Execute various cache-sensitive patterns */
    for (i = 0; i < iterations; i++) {
        matrix_multiply(MATRIX_SIZE, outer_limit);
        stride_access_pattern(512, stride);
        cache_line_test(3);
        mixed_data_type_operations();
        
        /* Architecture-specific code paths */
#ifdef __x86_64__
        x86_64_specific_operations();
#endif
        
#ifdef __i386__
        i386_specific_operations();
#endif
        
        /* Accumulate results from all operations */
        for (j = 0; j < 1000; j += 8) {
            total_sum += array1[j] + array2[j] + (int)darray1[j];
        }
    }
    
    /* Final computation using all arrays */
    double final_result = 0.0;
    for (i = 0; i < LARGE_SIZE; i += 8) {
        final_result += array1[i] * 0.5 + darray1[i] + carray1[i];
    }
    
    printf("Total sum: %lld\n", total_sum);
    printf("Final result: %f\n", final_result);
    printf("Computation complete.\n");
    
    return (int)(total_sum % 256);
}
