/* cache_detection_test.c
 * Designed to trigger GCC driver's CPU cache detection logic
 * for x86 cache descriptor values: 0x0a, 0x0c, 0x0d, 0x0e, 0x21,
 * 0x24, 0x2c, 0x39-0x3e, 0x41-0x45, 0x48-0x49, 0x4e, 0x60,
 * 0x66-0x68, 0x78-0x80, 0x82-0x87
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force compiler to consider cache-aware optimizations */
#define CACHE_LINE_SIZE 64
#define L1_SIZE 32768
#define L2_SIZE 262144

/* Volatile variables to prevent compile-time optimization */
volatile int outer_limit = 1000;
volatile int inner_limit = 1000;
volatile int stride = 16;

/* Large arrays that exceed L1/L2 cache */
static int matrix_a[2048][2048];
static int matrix_b[2048][2048];
static int matrix_c[2048][2048];
static double dbl_array[50000];
static char char_array[100000];

/* Function to force cache-dependent optimizations */
void matrix_multiply(int n, volatile int limit) {
    int i, j, k;
    int local_limit = limit;
    
    /* Triple nested loop - typical matrix multiplication pattern */
    for (i = 0; i < local_limit; i++) {
        for (j = 0; j < local_limit; j++) {
            int sum = 0;
            for (k = 0; k < local_limit; k++) {
                /* Access with different strides to test cache behavior */
                sum += matrix_a[i][k] * matrix_b[k][j];
            }
            matrix_c[i][j] = sum;
        }
    }
}

/* Non-unit stride access pattern */
void stride_access(volatile int step) {
    int i;
    long long sum = 0;
    int local_step = step;
    
    /* Access every Nth element to test cache line utilization */
    for (i = 0; i < 100000; i += local_step) {
        char_array[i] = (char)(i % 256);
        sum += char_array[i];
    }
    
    /* Prevent dead code elimination */
    if (sum == 0) printf("Impossible\n");
}

/* Array copy with potential cache line aliasing */
void cache_line_copy(void) {
    int i, j;
    
    /* Copy with different access patterns */
    for (i = 0; i < 50000; i++) {
        /* Mix int and double operations */
        dbl_array[i] = (double)matrix_a[i % 2048][0];
    }
    
    /* Reverse copy to cause potential cache conflicts */
    for (i = 49999; i >= 0; i--) {
        matrix_a[i % 2048][0] = (int)dbl_array[i];
    }
}

/* Mixed data type operations */
void mixed_operations(volatile int iter) {
    int i, j;
    int local_iter = iter;
    double accum = 0.0;
    
    for (i = 0; i < local_iter; i++) {
        for (j = 0; j < 1000; j++) {
            /* Mix operations on different data types */
            dbl_array[j] = dbl_array[j] * 1.01 + (double)char_array[i % 1000];
            accum += dbl_array[j];
            
            /* Integer operations */
            matrix_b[i % 2048][j % 2048] = 
                matrix_a[j % 2048][i % 2048] + (int)accum;
        }
    }
    
    /* Use result to prevent optimization */
    if (accum < 0) printf("Negative\n");
}

/* Explicit CPU feature detection if available */
#ifdef __x86_64__
void check_cpu_features(void) {
    /* These builtins may trigger CPUID queries */
    #ifdef __GNUC__
    __builtin_cpu_init();
    if (__builtin_cpu_supports("sse")) {
        /* SSE support implies certain cache characteristics */
        asm volatile("" ::: "memory");
    }
    if (__builtin_cpu_supports("avx")) {
        /* AVX support for newer processors */
        asm volatile("" ::: "memory");
    }
    #endif
}
#endif

int main(int argc, char *argv[]) {
    int i, j;
    long long total_sum = 0;
    clock_t start, end;
    
    /* Initialize arrays with non-zero values */
    srand(time(NULL));
    for (i = 0; i < 2048; i++) {
        for (j = 0; j < 2048; j++) {
            matrix_a[i][j] = rand() % 100;
            matrix_b[i][j] = rand() % 100;
        }
    }
    
    for (i = 0; i < 50000; i++) {
        dbl_array[i] = (double)(rand() % 1000) / 10.0;
    }
    
    for (i = 0; i < 100000; i++) {
        char_array[i] = (char)(rand() % 256);
    }
    
    /* Conditional compilation for different x86 architectures */
    #ifdef __x86_64__
    check_cpu_features();
    
    /* Different code paths that might encourage different -march settings */
    #if defined(__AVX__)
    /* AVX-optimized path */
    printf("AVX architecture detected\n");
    #elif defined(__SSE4_2__)
    /* SSE4.2 path */
    printf("SSE4.2 architecture detected\n");
    #else
    /* Generic x86-64 path */
    printf("Generic x86-64 architecture\n");
    #endif
    
    #elif defined(__i386__)
    printf("32-bit x86 architecture\n");
    #endif
    
    start = clock();
    
    /* Execute various cache-intensive operations */
    
    /* 1. Matrix multiplication pattern */
    matrix_multiply(512, outer_limit % 512);
    
    /* 2. Non-unit stride access */
    stride_access(stride);
    
    /* 3. Cache line copying with aliasing */
    cache_line_copy();
    
    /* 4. Mixed data type operations */
    mixed_operations(inner_limit % 100);
    
    /* Additional loop patterns to stress cache detection */
    
    /* Blocked matrix transpose for better cache utilization */
    int block_size = 64; /* Typical cache line friendly size */
    for (i = 0; i < 2048; i += block_size) {
        for (j = 0; j < 2048; j += block_size) {
            int ii, jj;
            for (ii = i; ii < i + block_size && ii < 2048; ii++) {
                for (jj = j; jj < j + block_size && jj < 2048; jj++) {
                    int temp = matrix_a[ii][jj];
                    matrix_a[ii][jj] = matrix_b[jj][ii];
                    matrix_b[jj][ii] = temp;
                }
            }
        }
    }
    
    /* Reduction operation with temporal locality */
    double sum_dbl = 0.0;
    for (i = 0; i < 50000; i++) {
        sum_dbl += dbl_array[i];
        if (i % 1000 == 0) {
            /* Periodic operation to break prefetch patterns */
            dbl_array[i] = sum_dbl / (i + 1);
        }
    }
    
    /* Final computation using all arrays */
    for (i = 0; i < 1000; i++) {
        for (j = 0; j < 1000; j++) {
            total_sum += matrix_c[i][j] + (int)dbl_array[(i * j) % 50000];
        }
    }
    
    end = clock();
    
    printf("Total sum: %lld\n", total_sum);
    printf("Execution time: %f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    
    return 0;
}
