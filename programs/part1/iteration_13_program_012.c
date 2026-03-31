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
#define ARRAY_SIZE 10000
#define BLOCK_SIZE 64

/* Volatile variables to prevent compile-time optimization */
volatile int outer_limit = ARRAY_SIZE;
volatile int stride = 8;

/* Different array types to test various cache line behaviors */
static int matrix_a[ARRAY_SIZE][ARRAY_SIZE];
static double matrix_b[ARRAY_SIZE][ARRAY_SIZE];
static char char_array[ARRAY_SIZE * 8];
static int result_array[ARRAY_SIZE];

/* Matrix multiplication kernel - benefits from cache blocking */
void matrix_multiply(int size) {
    volatile int n = size;
    int i, j, k;
    
    /* Triple nested loop - compiler may apply cache-aware transformations */
    for (i = 0; i < n; i += BLOCK_SIZE) {
        for (j = 0; j < n; j += BLOCK_SIZE) {
            for (k = 0; k < n; k++) {
                int ii, jj;
                for (ii = i; ii < i + BLOCK_SIZE && ii < n; ii++) {
                    for (jj = j; jj < j + BLOCK_SIZE && jj < n; jj++) {
                        matrix_a[ii][jj] += matrix_b[ii][k] * matrix_a[k][jj];
                    }
                }
            }
        }
    }
}

/* Non-unit stride access pattern - tests cache line utilization */
double stride_access(int limit, int step) {
    volatile int s = step;
    double sum = 0.0;
    int i;
    
    for (i = 0; i < limit; i += s) {
        sum += matrix_b[i][0] * 1.5;
        /* Mix in char array access for different data sizes */
        char_array[i % (ARRAY_SIZE * 8)] = (char)(sum * 0.1);
    }
    return sum;
}

/* Array copy with potential cache line aliasing */
void copy_with_aliasing(int size) {
    volatile int n = size;
    int i, j;
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j += 16) {  /* Potential cache line conflict */
            result_array[i] = matrix_a[j][i] + matrix_a[i][j];
        }
    }
}

/* Initialize arrays with pseudo-random data */
void init_arrays(void) {
    int i, j;
    srand(time(NULL));
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        for (j = 0; j < ARRAY_SIZE; j++) {
            matrix_a[i][j] = rand() % 100;
            matrix_b[i][j] = (double)(rand() % 100) / 10.0;
        }
    }
    
    for (i = 0; i < ARRAY_SIZE * 8; i++) {
        char_array[i] = (char)(rand() % 256);
    }
}

/* Explicit CPU feature detection - may prompt driver initialization */
#ifdef __x86_64__
void check_cpu_features(void) {
    /* These builtins may cause the driver to initialize CPU detection */
    __builtin_cpu_init();
    
    /* Check various features that might be associated with different cache configs */
    if (__builtin_cpu_supports("sse2")) {
        /* SSE2 often comes with specific cache architectures */
    }
    if (__builtin_cpu_supports("avx")) {
        /* AVX processors have different cache hierarchies */
    }
}
#endif

int main(int argc, char *argv[]) {
    double total_sum = 0.0;
    int i;
    
    /* Use command line args to prevent constant propagation */
    volatile int matrix_size = (argc > 1) ? atoi(argv[1]) : 500;
    if (matrix_size > ARRAY_SIZE) matrix_size = ARRAY_SIZE;
    
    init_arrays();
    
#ifdef __x86_64__
    /* Explicit CPU detection - triggers driver's cache detection */
    check_cpu_features();
#endif
    
    /* Execute different loop patterns that benefit from cache-aware optimizations */
    
    /* Pattern 1: Matrix multiplication with cache blocking opportunities */
    matrix_multiply(matrix_size);
    
    /* Pattern 2: Non-unit stride access */
    total_sum += stride_access(outer_limit, stride);
    
    /* Pattern 3: Copy with potential cache aliasing */
    copy_with_aliasing(matrix_size / 2);
    
    /* Pattern 4: Additional mixed access pattern */
    for (i = 0; i < matrix_size; i++) {
        /* Access with varying strides to defeat prefetching */
        int idx = (i * 7) % matrix_size;
        result_array[i] = matrix_a[idx][i] + (int)(matrix_b[i][idx] * 100);
        
        /* Occasionally access far memory */
        if (i % 32 == 0) {
            result_array[i] += char_array[(i * 13) % (ARRAY_SIZE * 8)];
        }
    }
    
    /* Compute final result to prevent dead code elimination */
    int final_result = 0;
    for (i = 0; i < matrix_size; i++) {
        final_result += result_array[i] % 100;
    }
    
    printf("Cache test result: %d (sum: %f)\n", final_result, total_sum);
    
    return final_result % 256;
}
