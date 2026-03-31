/* test_cache_detection.c
 * Designed to trigger GCC driver's CPU cache detection logic
 * for x86 cache descriptor values: 0x0a, 0x0c, 0x0d, 0x0e, 0x21,
 * 0x24, 0x2c, 0x39-0x3e, 0x41-0x45, 0x48-0x49, 0x4e, 0x60,
 * 0x66-0x68, 0x78-0x80, 0x82-0x87
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Force compiler to consider cache-aware optimizations */
#define ARRAY_SIZE 10000
#define MATRIX_SIZE 256

/* Volatile variables to prevent compile-time optimization */
volatile int outer_limit = ARRAY_SIZE;
volatile int matrix_dim = MATRIX_SIZE;

/* Different array types to test various cache line behaviors */
static int int_array1[ARRAY_SIZE];
static int int_array2[ARRAY_SIZE];
static double double_array[ARRAY_SIZE];
static char char_array[ARRAY_SIZE * 4]; /* Larger to exceed cache */

/* Matrix for cache-intensive operations */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];

/* Function to force cache-dependent optimizations */
void cache_sensitive_operations(int n, int m) {
    volatile int i, j, k;
    double sum;
    
    /* Pattern 1: Matrix multiplication - tests L1/L2 cache blocking */
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            sum = 0.0;
            for (k = 0; k < n; k++) {
                sum += matrix_a[i][k] * matrix_b[k][j];
            }
            matrix_c[i][j] = sum;
        }
    }
    
    /* Pattern 2: Non-unit stride access - tests cache line utilization */
    int stride = 16; /* Force non-sequential access */
    for (i = 0; i < ARRAY_SIZE; i += stride) {
        int_array2[i] = int_array1[i] * 2 + 1;
    }
    
    /* Pattern 3: Copy with potential cache line aliasing */
    for (i = 0; i < ARRAY_SIZE; i++) {
        char_array[i] = (char)(int_array1[i] & 0xFF);
    }
    
    /* Pattern 4: Double precision operations with temporal locality */
    for (i = 0; i < ARRAY_SIZE - m; i++) {
        double_array[i] = double_array[i] * 0.99 + double_array[i + m] * 0.01;
    }
}

/* Another cache-intensive pattern with different access pattern */
void nested_loop_pattern(int limit) {
    volatile int i, j, k;
    int block_size = 32; /* Typical cache line size */
    
    /* Blocked matrix transposition - benefits from cache size knowledge */
    for (i = 0; i < limit; i += block_size) {
        for (j = 0; j < limit; j += block_size) {
            for (k = i; k < i + block_size && k < limit; k++) {
                for (int l = j; l < j + block_size && l < limit; l++) {
                    matrix_b[l][k] = matrix_a[k][l];
                }
            }
        }
    }
    
    /* Scalar product with reduction - tests associativity */
    double dot_product = 0.0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        dot_product += double_array[i] * (i % 7);
    }
    
    /* Prevent dead code elimination */
    if (dot_product < 0) {
        printf("Unexpected negative dot product\n");
    }
}

/* Initialize arrays with pseudo-random but deterministic values */
void initialize_arrays(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array1[i] = (i * 37) % 1000;
        double_array[i] = (i * 0.01);
    }
    
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = (i + j) * 0.5;
            matrix_b[i][j] = (i * j) * 0.3;
        }
    }
}

/* Main function with architecture-specific code paths */
int main(int argc, char **argv) {
    int iterations = 10;
    volatile int use_optimized_path = 1;
    
    /* Initialize data */
    initialize_arrays();
    
    /* Force compiler to consider different x86 architectures */
#ifdef __x86_64__
    /* Code that benefits from specific cache configurations */
    printf("x86_64 architecture detected\n");
    
    /* Multiple cache-intensive patterns */
    for (int iter = 0; iter < iterations; iter++) {
        cache_sensitive_operations(matrix_dim, iter % 8 + 1);
        nested_loop_pattern(matrix_dim);
        
        /* Mix data types to test different cache line behaviors */
        for (int i = 0; i < ARRAY_SIZE - 1; i++) {
            int_array1[i + 1] = int_array1[i] + int_array2[i];
        }
    }
    
#elif defined(__i386__)
    printf("i386 architecture detected\n");
    /* Simpler patterns for 32-bit */
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        int_array2[i] = int_array1[i * 2] + int_array1[i * 2 + 1];
    }
#endif
    
    /* Additional architecture-agnostic cache test */
    {
        volatile int i, j;
        /* Stream-like access pattern */
        for (i = 0; i < ARRAY_SIZE; i += 64) { /* Cache line sized jumps */
            for (j = 0; j < 64 && i + j < ARRAY_SIZE; j++) {
                char_array[i + j] = (char)((int_array1[i + j] + j) & 0xFF);
            }
        }
    }
    
    /* Compute and print a result to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum += int_array1[i % ARRAY_SIZE];
    }
    printf("Result checksum: %d\n", checksum);
    
    return 0;
}

/* Additional function to create complex control flow */
void complex_cache_pattern(int *data, int size, int passes) {
    volatile int i, pass;
    int *buffer = malloc(size * sizeof(int));
    
    if (!buffer) return;
    
    for (pass = 0; pass < passes; pass++) {
        /* Forward pass */
        for (i = 1; i < size; i++) {
            buffer[i] = data[i - 1] + pass;
        }
        
        /* Backward pass with stride */
        for (i = size - 2; i >= 0; i -= 2) {
            data[i] = buffer[i + 1] - pass;
        }
        
        /* Middle-out pattern */
        int mid = size / 2;
        for (i = 0; i < mid; i++) {
            int tmp = data[i];
            data[i] = data[size - i - 1];
            data[size - i - 1] = tmp;
        }
    }
    
    free(buffer);
}
