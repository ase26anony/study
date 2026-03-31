/* cache_detection_test.c
 * Designed to trigger GCC driver's CPU cache detection logic
 * for x86 cache descriptor values: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c,
 * 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x41-0x45, 0x48, 0x49, 0x4e,
 * 0x60, 0x66-0x68, 0x78-0x80, 0x82-0x87
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Use volatile to prevent compile-time optimization */
volatile int outer_limit = 1000;
volatile int inner_limit = 500;

/* Large arrays that exceed typical L1 cache sizes */
#define LARGE_SIZE 10000
#define MEDIUM_SIZE 5000
#define SMALL_SIZE 1000

static int array1[LARGE_SIZE];
static double array2[MEDIUM_SIZE];
static char array3[LARGE_SIZE];
static int matrix_a[SMALL_SIZE][SMALL_SIZE];
static int matrix_b[SMALL_SIZE][SMALL_SIZE];
static int matrix_c[SMALL_SIZE][SMALL_SIZE];

/* Function to force compiler to consider cache optimizations */
void matrix_multiply_optimized(int n, volatile int limit) {
    int i, j, k;
    int block_size = 32; /* Common cache-friendly block size */
    
    /* Blocked matrix multiplication - compiler may optimize based on cache */
    for (i = 0; i < n; i += block_size) {
        for (j = 0; j < n; j += block_size) {
            for (k = 0; k < n; k += block_size) {
                /* Mini matrix multiplication within block */
                for (int ii = i; ii < i + block_size && ii < n; ii++) {
                    for (int jj = j; jj < j + block_size && jj < n; jj++) {
                        int sum = matrix_c[ii][jj];
                        for (int kk = k; kk < k + block_size && kk < n; kk++) {
                            sum += matrix_a[ii][kk] * matrix_b[kk][jj];
                        }
                        matrix_c[ii][jj] = sum;
                    }
                }
            }
        }
    }
}

/* Non-unit stride access pattern */
void stride_access_pattern(int stride, volatile int iterations) {
    long long sum = 0;
    int i;
    
    /* Access every 'stride' element - tests cache line utilization */
    for (i = 0; i < LARGE_SIZE; i += stride) {
        array1[i] = i * 2;
        sum += array1[i];
    }
    
    /* Reverse stride pattern */
    for (i = LARGE_SIZE - 1; i >= 0; i -= stride / 2) {
        array1[i] = i * 3;
        sum -= array1[i];
    }
    
    printf("Stride pattern result: %lld\n", sum);
}

/* Cache line aliasing test */
void cache_line_aliasing_test(void) {
    int i, j;
    volatile int offset = 16; /* Common cache line size offset */
    
    /* Copy with potential cache line conflicts */
    for (i = 0; i < MEDIUM_SIZE; i++) {
        for (j = 0; j < 8; j++) { /* 8 ints per 64-byte cache line */
            int idx = (i * 8 + j + offset) % LARGE_SIZE;
            array3[idx] = (char)(array1[i] + j);
        }
    }
}

/* Mixed data type operations */
void mixed_data_type_operations(volatile int scale) {
    int i;
    double accumulator = 0.0;
    
    /* Mix int and double operations */
    for (i = 0; i < MEDIUM_SIZE; i++) {
        array2[i] = (double)array1[i] * 1.5;
        accumulator += array2[i];
        
        /* Conditional store based on array value */
        if (array1[i] % 7 == 0) {
            array3[i % LARGE_SIZE] = (char)(array1[i] % 256);
        }
    }
    
    printf("Mixed operations accumulator: %f\n", accumulator);
}

#ifdef __x86_64__
/* x86-64 specific optimizations */
void x86_64_specific_workload(void) {
    /* Use __builtin_cpu functions to trigger CPU detection */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("sse2")) {
        printf("SSE2 supported - using vector-friendly pattern\n");
        
        /* Pattern that might benefit from SSE vectorization */
        for (int i = 0; i < LARGE_SIZE - 3; i += 4) {
            array1[i] = i;
            array1[i+1] = i+1;
            array1[i+2] = i+2;
            array1[i+3] = i+3;
        }
    }
    
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported - wider operations\n");
    }
}
#endif

#ifdef __i386__
/* i386 specific code path */
void i386_specific_workload(void) {
    printf("32-bit x86 mode\n");
    
    /* Different access pattern for 32-bit */
    for (int i = 0; i < SMALL_SIZE; i++) {
        for (int j = 0; j < SMALL_SIZE; j++) {
            matrix_a[i][j] = i * j;
            matrix_b[i][j] = i + j;
        }
    }
}
#endif

int main(int argc, char *argv[]) {
    volatile int n = 256; /* Matrix size */
    volatile int stride = 13; /* Prime number for non-uniform stride */
    volatile int seed = 12345;
    
    /* Initialize arrays with pseudo-random data */
    srand(seed);
    for (int i = 0; i < LARGE_SIZE; i++) {
        array1[i] = rand() % 1000;
    }
    
    for (int i = 0; i < MEDIUM_SIZE; i++) {
        array2[i] = (double)(rand() % 1000) / 10.0;
    }
    
    memset(array3, 0, LARGE_SIZE);
    
    printf("Starting cache-sensitive workload...\n");
    
    /* Execute different optimization patterns */
    
    /* 1. Matrix multiplication (cache-blocking sensitive) */
    matrix_multiply_optimized(n, inner_limit);
    
    /* 2. Non-unit stride pattern */
    stride_access_pattern(stride, outer_limit);
    
    /* 3. Cache line aliasing test */
    cache_line_aliasing_test();
    
    /* 4. Mixed data type operations */
    mixed_data_type_operations(inner_limit);
    
    /* Architecture-specific paths */
#ifdef __x86_64__
    x86_64_specific_workload();
#endif
    
#ifdef __i386__
    i386_specific_workload();
#endif
    
    /* Final computation using all arrays */
    long long final_sum = 0;
    for (int i = 0; i < LARGE_SIZE; i += 64) { /* Cache line granularity */
        final_sum += array1[i];
        final_sum += (int)array3[i];
    }
    
    for (int i = 0; i < MEDIUM_SIZE; i += 32) {
        final_sum += (long long)array2[i];
    }
    
    printf("Final result: %lld\n", final_sum);
    
    return 0;
}
