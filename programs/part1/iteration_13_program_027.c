/* 
 * cache_detection_test.c
 * Designed to trigger GCC driver's CPU cache detection logic
 * for x86 cache descriptor values: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 
 * 0x24, 0x2c, 0x39-0x3e, 0x41-0x45, 0x48-0x49, 0x4e, 0x60, 
 * 0x66-0x68, 0x78-0x80, 0x82-0x87
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Use volatile to prevent compile-time optimization */
volatile int outer_limit = 1000;
volatile int inner_limit = 1000;
volatile int stride = 16;

/* Large arrays that exceed typical L1 cache sizes */
#define LARGE_SIZE 10000
static int matrix_a[LARGE_SIZE][LARGE_SIZE/100];
static int matrix_b[LARGE_SIZE/100][LARGE_SIZE];
static int matrix_c[LARGE_SIZE/10][LARGE_SIZE/10];
static double double_array[5000][5000/100];
static char char_array[20000][20000/100];

/* Function prototypes to force different optimization considerations */
void matrix_multiply(int n, volatile int limit);
void stride_access_pattern(int size, int stride);
void cache_line_test(int iterations);
void mixed_data_types_operation(void);

/* Matrix multiplication - triple nested loop */
void matrix_multiply(int n, volatile int limit) {
    int i, j, k;
    int sum;
    
    /* Use volatile parameter to prevent constant propagation */
    volatile int local_limit = limit > n ? n : limit;
    
    for (i = 0; i < local_limit; i++) {
        for (j = 0; j < local_limit; j++) {
            sum = 0;
            for (k = 0; k < local_limit; k++) {
                /* Access with different patterns to stress cache */
                sum += matrix_a[i][k] * matrix_b[k][j];
            }
            matrix_c[i][j] = sum;
        }
    }
}

/* Non-unit stride access pattern */
void stride_access_pattern(int size, int stride) {
    int i, j;
    long long accumulator = 0;
    
    for (i = 0; i < size; i += stride) {
        for (j = 0; j < size; j += stride) {
            /* Access every Nth element to test cache line utilization */
            accumulator += matrix_c[i % (LARGE_SIZE/10)][j % (LARGE_SIZE/10)];
            /* Mix in double precision operations */
            double_array[i % 5000][j % (5000/100)] = accumulator * 0.5;
        }
    }
    
    /* Prevent dead code elimination */
    if (accumulator == 0) {
        printf("Impossible condition\n");
    }
}

/* Cache line aliasing test */
void cache_line_test(int iterations) {
    int i, j;
    char *src = (char *)char_array;
    char *dst = (char *)&char_array[10000];
    
    for (i = 0; i < iterations; i++) {
        /* Copy with potential cache line conflicts */
        for (j = 0; j < 100000; j += 64) { /* Typical cache line size */
            dst[j] = src[j] + i;
        }
        
        /* Reverse copy pattern */
        for (j = 100000 - 64; j >= 0; j -= 64) {
            src[j] = dst[j] - i;
        }
    }
}

/* Mixed data type operations */
void mixed_data_types_operation(void) {
    int i, j;
    double sum_d = 0.0;
    int sum_i = 0;
    
    for (i = 0; i < 1000; i++) {
        for (j = 0; j < 1000; j++) {
            /* Alternate between data types */
            if ((i + j) % 2 == 0) {
                sum_d += double_array[i % 5000][j % (5000/100)];
            } else {
                sum_i += matrix_c[i % (LARGE_SIZE/10)][j % (LARGE_SIZE/10)];
            }
            
            /* Char array access with different stride */
            char_array[(i * 17) % 20000][(j * 13) % (20000/100)] = 
                (char)((sum_i + (int)sum_d) & 0xFF);
        }
    }
    
    printf("Mixed type sums: int=%d, double=%f\n", sum_i, sum_d);
}

/* Main workload with architecture-specific code paths */
int main(int argc, char **argv) {
    int i, result = 0;
    clock_t start, end;
    
    /* Initialize arrays with pseudo-random but deterministic values */
    srand(42);
    for (i = 0; i < LARGE_SIZE; i++) {
        for (int j = 0; j < LARGE_SIZE/100; j++) {
            matrix_a[i][j] = rand() % 100;
            if (j < LARGE_SIZE/100) {
                matrix_b[j][i] = rand() % 100;
            }
        }
    }
    
    printf("Starting cache-sensitive workload...\n");
    start = clock();
    
    /* Architecture-specific code blocks to encourage different -march usages */
#ifdef __x86_64__
    /* Code that benefits from 64-bit optimizations */
    printf("64-bit x86 architecture detected\n");
    
    /* Force CPU feature detection if builtins are available */
#ifdef __GNUC__
    /* This may prompt driver to check CPU features */
    __builtin_cpu_init();
#endif
    
    /* Different optimization strategies based on compilation target */
#if defined(__tune_core2__) || defined(__tune_nocona__)
    printf("Compiled for Core2/Nocona target\n");
    matrix_multiply(500, outer_limit);
    stride_access_pattern(2000, stride);
#elif defined(__tune_nehalem__) || defined(__tune_westmere__)
    printf("Compiled for Nehalem/Westmere target\n");
    matrix_multiply(800, outer_limit);
    stride_access_pattern(3000, stride * 2);
#elif defined(__tune_haswell__) || defined(__tune_broadwell__)
    printf("Compiled for Haswell/Broadwell target\n");
    matrix_multiply(1200, outer_limit);
    stride_access_pattern(4000, stride * 4);
#else
    /* Generic x86-64 code path */
    printf("Generic x86-64 optimization path\n");
    matrix_multiply(300, outer_limit);
    stride_access_pattern(1500, stride);
#endif
    
#elif defined(__i386__)
    /* 32-bit specific code path */
    printf("32-bit x86 architecture detected\n");
    matrix_multiply(200, outer_limit);
    stride_access_pattern(1000, stride);
#else
    /* Fallback for other architectures */
    printf("Non-x86 architecture\n");
    matrix_multiply(100, outer_limit);
#endif
    
    /* Common workload for all architectures */
    cache_line_test(100);
    mixed_data_types_operation();
    
    /* Additional loop patterns */
    for (i = 0; i < outer_limit; i++) {
        int j;
        for (j = 0; j < inner_limit; j++) {
            /* Complex addressing pattern */
            int idx = (i * 31 + j * 17) % LARGE_SIZE;
            int idy = (i * 13 + j * 29) % (LARGE_SIZE/100);
            matrix_c[idx % (LARGE_SIZE/10)][idy % (LARGE_SIZE/10)] += 
                matrix_a[idx][idy] * matrix_b[idy][idx % (LARGE_SIZE/100)];
        }
    }
    
    end = clock();
    printf("Workload completed in %.2f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Compute final result to prevent dead code elimination */
    for (i = 0; i < 100; i++) {
        result += matrix_c[i % (LARGE_SIZE/10)][i % (LARGE_SIZE/10)];
    }
    
    printf("Final result: %d\n", result);
    return result != 0 ? 0 : 1;
}
