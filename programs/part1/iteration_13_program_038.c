/* cache_detection_test.c
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

/* Large arrays to exceed L1 cache */
#define LARGE_SIZE 10000
static int matrix_a[LARGE_SIZE][LARGE_SIZE/10];
static int matrix_b[LARGE_SIZE/10][LARGE_SIZE];
static int matrix_c[LARGE_SIZE/10][LARGE_SIZE/10];
static double double_array[5000][500];
static char char_array[20000][100];

/* Matrix multiplication - triple nested loop pattern */
void matrix_multiply(int n, int m, int p) {
    volatile int i, j, k;
    int sum;
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < p; j++) {
            sum = 0;
            for (k = 0; k < m; k++) {
                sum += matrix_a[i][k] * matrix_b[k][j];
            }
            matrix_c[i][j] = sum;
        }
    }
}

/* Non-unit stride access pattern */
int stride_access(int size, int stride_val) {
    volatile int i;
    int total = 0;
    
    for (i = 0; i < size; i += stride_val) {
        total += double_array[i % 5000][0];
    }
    return total;
}

/* Cache line aliasing test - copying with potential conflicts */
void cache_line_copy(int size) {
    volatile int i, j;
    
    for (i = 0; i < size; i++) {
        for (j = 0; j < 100; j++) {
            char_array[(i * 31) % 20000][j] = 
                char_array[(i * 17) % 20000][j] + 1;
        }
    }
}

/* Mixed data type operations */
double mixed_operations(int iterations) {
    volatile int i, j;
    double result = 0.0;
    
    for (i = 0; i < iterations; i++) {
        for (j = 0; j < 100; j++) {
            result += double_array[i % 5000][j % 500] * 
                     (matrix_a[i % LARGE_SIZE][j % (LARGE_SIZE/10)] / 256.0);
        }
    }
    return result;
}

/* Initialize arrays with pseudo-random data */
void initialize_arrays(void) {
    volatile int i, j;
    
    for (i = 0; i < LARGE_SIZE; i++) {
        for (j = 0; j < LARGE_SIZE/10; j++) {
            matrix_a[i][j] = (i * 17 + j * 13) % 256;
        }
    }
    
    for (i = 0; i < LARGE_SIZE/10; i++) {
        for (j = 0; j < LARGE_SIZE; j++) {
            matrix_b[i][j] = (i * 19 + j * 11) % 256;
        }
    }
    
    for (i = 0; i < 5000; i++) {
        for (j = 0; j < 500; j++) {
            double_array[i][j] = (i * 0.1 + j * 0.01);
        }
    }
}

int main(int argc, char *argv[]) {
    clock_t start, end;
    double cpu_time_used;
    int result_int;
    double result_double;
    
    /* Force CPU feature detection if available */
    #ifdef __x86_64__
    /* These builtins may trigger CPUID queries */
    __builtin_cpu_init();
    #endif
    
    printf("Starting cache detection test...\n");
    
    start = clock();
    
    /* Initialize data */
    initialize_arrays();
    
    /* Execute different loop patterns to trigger various optimizations */
    
    /* Pattern 1: Matrix multiplication (good for tiling optimizations) */
    printf("Running matrix multiplication...\n");
    matrix_multiply(outer_limit/10, inner_limit/10, outer_limit/10);
    
    /* Pattern 2: Stride access (tests prefetching) */
    printf("Running stride access pattern...\n");
    result_int = stride_access(outer_limit * 100, stride);
    
    /* Pattern 3: Cache line operations */
    printf("Running cache line copy...\n");
    cache_line_copy(inner_limit);
    
    /* Pattern 4: Mixed operations */
    printf("Running mixed operations...\n");
    result_double = mixed_operations(outer_limit/2);
    
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    printf("Test completed in %.2f seconds\n", cpu_time_used);
    printf("Results: int=%d, double=%.2f\n", result_int, result_double);
    
    /* Conditional compilation for different architectures */
    #ifdef __x86_64__
    printf("x86_64 architecture detected\n");
    #if defined(__AVX512F__)
    printf("AVX512 support detected - testing wider vectors\n");
    #elif defined(__AVX2__)
    printf("AVX2 support detected\n");
    #elif defined(__SSE4_2__)
    printf("SSE4.2 support detected\n");
    #endif
    #endif
    
    return 0;
}
