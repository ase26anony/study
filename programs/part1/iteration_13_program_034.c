/* cache_detection_test.c
 * Designed to trigger GCC driver's CPU cache detection logic
 * for uncovered cache descriptor values (0x0a, 0x0c, 0x0d, 0x0e, 0x21, etc.)
 * Compile with various x86-specific tuning options to exercise the switch cases
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

/* Different array types to exercise various cache line scenarios */
static int int_array1[LARGE_SIZE];
static int int_array2[LARGE_SIZE];
static double double_array1[MEDIUM_SIZE][MEDIUM_SIZE/100];
static double double_array2[MEDIUM_SIZE][MEDIUM_SIZE/100];
static char char_array[LARGE_SIZE * 4]; /* Larger to force cache line considerations */

/* Matrix multiplication-like kernel - benefits from cache-aware optimizations */
void matrix_multiply_kernel(int size) {
    volatile int n = size;
    double sum;
    
    /* Triple nested loop - classic cache optimization target */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            sum = 0.0;
            for (int k = 0; k < n; k++) {
                /* Access with different strides to exercise cache lines */
                sum += double_array1[i][k] * double_array2[k][j];
            }
            double_array1[i][j] = sum;
        }
    }
}

/* Non-unit stride access pattern - exercises cache line utilization */
void stride_access_kernel(int limit, int step) {
    volatile int n = limit;
    volatile int s = step;
    int accumulator = 0;
    
    /* Access every 's'-th element - can trigger prefetch logic */
    for (int i = 0; i < n; i += s) {
        accumulator += int_array1[i];
        int_array1[i] = accumulator;
    }
    
    /* Reverse stride pattern */
    for (int i = n - 1; i >= 0; i -= s) {
        accumulator -= int_array2[i];
        int_array2[i] = accumulator;
    }
}

/* Copy with potential cache line aliasing */
void copy_with_aliasing(int size, int offset) {
    volatile int n = size;
    volatile int off = offset;
    
    /* Copy between arrays with offset - can cause cache conflicts */
    for (int i = 0; i < n; i++) {
        int src_idx = i;
        int dst_idx = (i + off) % n;
        int_array2[dst_idx] = int_array1[src_idx];
    }
    
    /* Additional copy with different data types */
    for (int i = 0; i < n; i++) {
        char_array[i] = (char)(int_array1[i] & 0xFF);
    }
}

/* Mixed data type operations */
void mixed_operations(int iterations) {
    volatile int iter = iterations;
    double fp_acc = 0.0;
    int int_acc = 0;
    
    for (int i = 0; i < iter; i++) {
        /* Mix float and int operations */
        fp_acc += (double)int_array1[i % LARGE_SIZE] * 0.5;
        int_acc += (int)fp_acc;
        
        /* Conditional with data-dependent access */
        if (i % 8 == 0) {
            char_array[i % (LARGE_SIZE * 4)] = (char)(int_acc & 0xFF);
        }
    }
}

/* Initialize arrays with pseudo-random but deterministic values */
void initialize_arrays(void) {
    srand(42); /* Fixed seed for reproducibility */
    
    for (int i = 0; i < LARGE_SIZE; i++) {
        int_array1[i] = rand() % 100;
        int_array2[i] = rand() % 100;
    }
    
    for (int i = 0; i < MEDIUM_SIZE; i++) {
        for (int j = 0; j < MEDIUM_SIZE/100; j++) {
            double_array1[i][j] = (double)(rand() % 1000) / 10.0;
            double_array2[i][j] = (double)(rand() % 1000) / 10.0;
        }
    }
    
    for (int i = 0; i < LARGE_SIZE * 4; i++) {
        char_array[i] = (char)(rand() % 256);
    }
}

int main(int argc, char *argv[]) {
    clock_t start, end;
    double cpu_time_used;
    
    /* Initialize data */
    initialize_arrays();
    
    start = clock();
    
    /* Execute various kernels that benefit from cache-aware optimizations */
    
    /* Kernel 1: Matrix operations */
    matrix_multiply_kernel(inner_limit / 10);
    
    /* Kernel 2: Stride patterns */
    stride_access_kernel(outer_limit, stride);
    
    /* Kernel 3: Copy with aliasing */
    copy_with_aliasing(outer_limit / 2, 64);
    
    /* Kernel 4: Mixed operations */
    mixed_operations(100000);
    
    /* Additional loop patterns to exercise different optimizations */
    
    /* Tiled matrix addition - benefits from cache blocking */
    for (int ii = 0; ii < MEDIUM_SIZE; ii += 64) {
        for (int jj = 0; jj < MEDIUM_SIZE/100; jj += 64) {
            for (int i = ii; i < ii + 64 && i < MEDIUM_SIZE; i++) {
                for (int j = jj; j < jj + 64 && j < MEDIUM_SIZE/100; j++) {
                    double_array1[i][j] += double_array2[i][j];
                }
            }
        }
    }
    
    /* Reduction pattern */
    double sum = 0.0;
    for (int i = 0; i < LARGE_SIZE; i++) {
        sum += int_array1[i] * 0.01;
    }
    
    /* Scatter/gather pattern */
    for (int i = 0; i < LARGE_SIZE; i += 4) {
        int idx = (i * 17) % LARGE_SIZE; /* Non-linear access */
        int_array2[idx] = int_array1[i] + int_array1[i+1];
    }
    
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    /* Print results to prevent dead code elimination */
    printf("Cache detection test completed in %.3f seconds\n", cpu_time_used);
    printf("Final sum: %.2f\n", sum);
    printf("Sample values: %d, %d, %c\n", 
           int_array1[0], int_array2[0], char_array[0]);
    
    /* Conditional compilation for different architectures */
#ifdef __x86_64__
    printf("Compiled for x86_64 architecture\n");
    /* Additional x86-64 specific code */
    #ifdef __AVX__
    printf("AVX instructions available\n");
    #endif
    #ifdef __SSE4_2__
    printf("SSE4.2 instructions available\n");
    #endif
#endif
    
#ifdef __i386__
    printf("Compiled for i386 architecture\n");
#endif
    
    /* Explicit CPU feature detection if available */
#ifdef __GNUC__
    /* These builtins may trigger CPUID queries */
    __builtin_cpu_init();
    if (__builtin_cpu_supports("sse")) {
        printf("SSE supported\n");
    }
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported\n");
    }
#endif
    
    return 0;
}
