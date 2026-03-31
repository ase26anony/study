/* cache_detection_test.c
 * Designed to trigger GCC driver's CPU cache detection logic
 * for x86 cache descriptor values: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c,
 * 0x39-0x3e, 0x41-0x45, 0x48, 0x49, 0x4e, 0x60, 0x66-0x68, 0x78-0x80,
 * 0x82-0x87
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
static int matrix_a[LARGE_SIZE][LARGE_SIZE/100];
static int matrix_b[LARGE_SIZE/100][LARGE_SIZE];
static int matrix_c[LARGE_SIZE/100][LARGE_SIZE/100];
static double double_array[5000][500];
static char char_array[20000][200];

/* Function to force compiler to consider cache optimizations */
void matrix_multiply(int n, int m, int p) {
    /* Triple nested loop - typical for cache optimization */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int sum = 0;
            for (int k = 0; k < p; k++) {
                sum += matrix_a[i][k] * matrix_b[k][j];
            }
            matrix_c[i][j] = sum;
        }
    }
}

/* Non-unit stride access pattern */
void stride_access(int size, int stride_val) {
    volatile int result = 0;
    for (int i = 0; i < size; i += stride_val) {
        for (int j = 0; j < size; j += stride_val) {
            result += double_array[i][j];
            char_array[i][j] = (char)(result & 0xFF);
        }
    }
}

/* Cache line aliasing test */
void cache_line_copy(int size) {
    for (int i = 0; i < size - 64; i++) {
        /* Potential cache line conflicts */
        char_array[i][0] = char_array[i + 64][0];
        double_array[i/8][0] = double_array[(i + 32)/8][0];
    }
}

/* Mixed data type operations */
void mixed_operations(int iterations) {
    volatile double acc_double = 0.0;
    volatile int acc_int = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Access different array types */
        acc_double += double_array[i % 5000][0] * 1.5;
        acc_int += matrix_a[i % LARGE_SIZE][0] * 2;
        
        /* Conditional to prevent vectorization in some cases */
        if (acc_int % 7 == 0) {
            char_array[i % 20000][0] = (char)acc_int;
        }
    }
}

/* Initialize arrays with pseudo-random data */
void init_arrays(void) {
    srand(time(NULL));
    
    for (int i = 0; i < LARGE_SIZE; i++) {
        for (int j = 0; j < LARGE_SIZE/100; j++) {
            matrix_a[i][j] = rand() % 100;
            if (j < LARGE_SIZE/100 && i < LARGE_SIZE/100) {
                matrix_b[j][i] = rand() % 100;
            }
        }
    }
    
    for (int i = 0; i < 5000; i++) {
        for (int j = 0; j < 500; j++) {
            double_array[i][j] = (double)(rand() % 1000) / 10.0;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use arguments to prevent constant propagation */
    int n = (argc > 1) ? atoi(argv[1]) : outer_limit;
    int m = (argc > 2) ? atoi(argv[2]) : inner_limit;
    int s = (argc > 3) ? atoi(argv[3]) : stride;
    
    printf("Starting cache detection test with n=%d, m=%d, stride=%d\n", n, m, s);
    
    /* Initialize data */
    init_arrays();
    
    /* Execute different loop patterns to trigger various optimizations */
    
    /* Pattern 1: Matrix multiplication - benefits from cache blocking */
    printf("Running matrix multiplication...\n");
    matrix_multiply(n / 10, m / 10, 100);
    
    /* Pattern 2: Non-unit stride access */
    printf("Running stride access...\n");
    stride_access(n * 2, s);
    
    /* Pattern 3: Cache line operations */
    printf("Running cache line copy...\n");
    cache_line_copy(n * 10);
    
    /* Pattern 4: Mixed operations */
    printf("Running mixed operations...\n");
    mixed_operations(n * 100);
    
    /* Compute and print result to prevent dead code elimination */
    volatile int total = 0;
    for (int i = 0; i < 100; i++) {
        total += matrix_c[i % (LARGE_SIZE/100)][0];
    }
    
    printf("Test completed. Result checksum: %d\n", total);
    
    /* Conditional compilation for different architectures */
    #ifdef __x86_64__
        printf("Compiled for x86_64 architecture\n");
        /* Additional x86_64 specific code */
        #ifdef __SSE2__
            printf("SSE2 support detected\n");
        #endif
        #ifdef __AVX__
            printf("AVX support detected\n");
        #endif
    #endif
    
    #ifdef __i386__
        printf("Compiled for i386 architecture\n");
    #endif
    
    return 0;
}
