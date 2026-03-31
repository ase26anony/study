/* cache_detection_test.c
 * Designed to trigger GCC driver's CPU cache detection logic
 * for x86 cache descriptor values: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24,
 * 0x2c, 0x39-0x3e, 0x41-0x45, 0x48-0x49, 0x4e, 0x60, 0x66-0x68,
 * 0x78-0x80, 0x82-0x87
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

/* Different array types to test various cache line behaviors */
static int matrix_a[LARGE_SIZE][LARGE_SIZE];
static double matrix_b[MEDIUM_SIZE][MEDIUM_SIZE];
static char char_array[LARGE_SIZE * 4];
static int result_array[LARGE_SIZE];

/* Matrix multiplication kernel - benefits from cache blocking */
void matrix_multiply_kernel(int size) {
    volatile int n = size;
    int i, j, k;
    
    /* Initialize matrices */
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            matrix_a[i][j] = (i + j) % 256;
            matrix_b[i][j] = (i * j) / 256.0;
        }
    }
    
    /* Triple nested loop - compiler may apply cache-aware optimizations */
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            double sum = 0.0;
            for (k = 0; k < n; k++) {
                sum += matrix_a[i][k] * matrix_b[k][j];
            }
            result_array[i] += (int)sum;
        }
    }
}

/* Non-unit stride access pattern - tests cache line utilization */
void stride_access_kernel(int limit, int step) {
    volatile int n = limit;
    volatile int s = step;
    int i, acc = 0;
    
    for (i = 0; i < n; i += s) {
        char_array[i] = (char)(i % 256);
        acc += char_array[i];
    }
    
    /* Use result to prevent dead code elimination */
    result_array[0] += acc;
}

/* Cache line aliasing test - copying between arrays */
void cache_line_copy_kernel(int size) {
    volatile int n = size;
    int temp[SMALL_SIZE];
    int i, j;
    
    for (j = 0; j < 100; j++) {
        for (i = 0; i < n; i++) {
            temp[i % SMALL_SIZE] = matrix_a[i % LARGE_SIZE][j % LARGE_SIZE];
            result_array[i % LARGE_SIZE] += temp[i % SMALL_SIZE];
        }
    }
}

/* Mixed data type operations */
void mixed_operations_kernel(int iterations) {
    volatile int iter = iterations;
    int i;
    double d_acc = 0.0;
    int i_acc = 0;
    
    for (i = 0; i < iter; i++) {
        /* Mix int and double operations */
        d_acc += matrix_b[i % MEDIUM_SIZE][(i * 7) % MEDIUM_SIZE];
        i_acc += matrix_a[i % LARGE_SIZE][(i * 13) % LARGE_SIZE];
        
        /* Conditional store based on value */
        if (d_acc > 1000.0) {
            char_array[i % (LARGE_SIZE * 4)] = (char)(i_acc % 256);
            d_acc = 0.0;
        }
    }
    
    result_array[1] += i_acc + (int)d_acc;
}

/* Main workload with multiple cache-sensitive patterns */
int main(int argc, char **argv) {
    int i, total = 0;
    clock_t start, end;
    
    /* Initialize with non-zero values */
    srand(time(NULL));
    for (i = 0; i < LARGE_SIZE; i++) {
        result_array[i] = rand() % 100;
    }
    
    start = clock();
    
    /* Execute different cache-sensitive kernels */
    
    /* 1. Matrix multiplication - benefits from cache blocking */
    matrix_multiply_kernel(SMALL_SIZE);
    
    /* 2. Various stride patterns */
    stride_access_kernel(outer_limit, 1);      /* Unit stride */
    stride_access_kernel(outer_limit, stride); /* Non-unit stride */
    stride_access_kernel(outer_limit, 64);     /* Cache line sized stride */
    
    /* 3. Cache line copying with potential aliasing */
    cache_line_copy_kernel(inner_limit);
    
    /* 4. Mixed data type operations */
    mixed_operations_kernel(inner_limit * 10);
    
    /* 5. Additional nested loops with different access patterns */
    for (int block = 0; block < 10; block++) {
        for (i = block * 100; i < (block + 1) * 100; i++) {
            for (int j = 0; j < MEDIUM_SIZE; j += 8) {
                matrix_b[i % MEDIUM_SIZE][j] += 
                    matrix_a[j % LARGE_SIZE][i % LARGE_SIZE] * 0.5;
            }
        }
    }
    
    /* Accumulate final result */
    for (i = 0; i < LARGE_SIZE; i++) {
        total += result_array[i];
    }
    
    end = clock();
    
    printf("Cache test completed. Result: %d\n", total % 1000);
    printf("Time: %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    return 0;
}

/* Conditional compilation for different x86 architectures */
#ifdef __x86_64__
/* x86-64 specific optimizations */
void x86_64_specific_kernel(void) {
    /* Use SSE/AVX style memory patterns */
    for (int i = 0; i < LARGE_SIZE; i += 4) {
        /* Simulate vectorized access pattern */
        int sum = matrix_a[i][0] + matrix_a[i+1][1] + 
                  matrix_a[i+2][2] + matrix_a[i+3][3];
        result_array[i % LARGE_SIZE] += sum;
    }
}
#endif

#ifdef __i386__
/* i386 specific code path */
void i386_specific_kernel(void) {
    /* Different access pattern for 32-bit */
    for (int i = 0; i < MEDIUM_SIZE; i++) {
        for (int j = 0; j < 4; j++) {
            matrix_b[i][j] = matrix_a[i % SMALL_SIZE][j] * 0.25;
        }
    }
}
#endif
