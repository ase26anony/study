/* cache_detection_test.c
 * Designed to trigger GCC driver's CPU cache detection logic
 * for x86 targets, covering cache descriptor cases 0x0a through 0x87
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Volatile variables to prevent compile-time optimization */
volatile int outer_limit = 1000;
volatile int inner_limit = 500;
volatile int stride = 16;

/* Large arrays that exceed typical L1 cache sizes */
#define LARGE_SIZE 10000
#define MEDIUM_SIZE 5000
#define SMALL_SIZE 2000

static int array1[LARGE_SIZE];
static double array2[MEDIUM_SIZE];
static char array3[LARGE_SIZE * 2];
static int matrix_a[SMALL_SIZE][SMALL_SIZE];
static int matrix_b[SMALL_SIZE][SMALL_SIZE];
static int matrix_c[SMALL_SIZE][SMALL_SIZE];

/* Function prototypes to force different optimization considerations */
void matrix_multiply(int size);
void stride_access_pattern(int limit, int step);
void cache_line_test(int iterations);
void mixed_operations(void);

/* Matrix multiplication - classic cache-sensitive operation */
void matrix_multiply(int size) {
    int i, j, k;
    int sum;
    
    /* Initialize matrices */
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            matrix_a[i][j] = (i + j) % 256;
            matrix_b[i][j] = (i - j + 256) % 256;
            matrix_c[i][j] = 0;
        }
    }
    
    /* Perform multiplication with different loop orders */
    /* This tests compiler's cache-aware loop transformations */
    for (i = 0; i < size; i++) {
        for (k = 0; k < size; k++) {
            sum = matrix_a[i][k];
            for (j = 0; j < size; j++) {
                matrix_c[i][j] += sum * matrix_b[k][j];
            }
        }
    }
    
    /* Alternative loop order */
    for (j = 0; j < size; j++) {
        for (k = 0; k < size; k++) {
            sum = matrix_b[k][j];
            for (i = 0; i < size; i++) {
                matrix_c[i][j] += matrix_a[i][k] * sum;
            }
        }
    }
}

/* Non-unit stride access pattern */
void stride_access_pattern(int limit, int step) {
    int i, j;
    double acc = 0.0;
    
    /* Access array with varying strides */
    for (i = 0; i < limit; i += step) {
        array2[i] = (double)i * 1.5;
        acc += array2[i];
    }
    
    /* Reverse stride pattern */
    for (j = limit - 1; j >= 0; j -= step / 2) {
        array2[j] += acc;
        acc *= 0.99;
    }
    
    /* Store result to prevent dead code elimination */
    array2[0] = acc;
}

/* Test potential cache line aliasing effects */
void cache_line_test(int iterations) {
    int i, j, k;
    volatile int temp;
    
    /* Copy between arrays with potential aliasing */
    for (k = 0; k < iterations; k++) {
        for (i = 0; i < LARGE_SIZE; i += 64) {  /* 64-byte cache line size */
            for (j = 0; j < 16; j++) {          /* Work within a cache line */
                array3[i * 2 + j] = (char)(array1[i + j] & 0xFF);
            }
        }
        
        /* Reverse copy */
        for (i = LARGE_SIZE - 64; i >= 0; i -= 64) {
            for (j = 15; j >= 0; j--) {
                temp = array3[i * 2 + j];
                array1[i + j] = temp * 2;
            }
        }
    }
}

/* Mixed data type operations */
void mixed_operations(void) {
    int i, j;
    double fp_acc = 0.0;
    int int_acc = 0;
    
    /* Mixed int/double operations in nested loops */
    for (i = 0; i < MEDIUM_SIZE; i++) {
        for (j = 0; j < 8; j++) {
            /* Varying access patterns */
            int index = (i * 17 + j * 23) % MEDIUM_SIZE;
            array2[index] = (double)array1[i] * 1.414;
            fp_acc += array2[index];
            
            int_acc += array1[i] * j;
            array3[(i * 8 + j) % (LARGE_SIZE * 2)] = (char)(int_acc & 0xFF);
        }
        
        /* Conditional with data-dependent branch */
        if (int_acc > 1000000) {
            int_acc /= 2;
            fp_acc *= 0.5;
        }
    }
    
    /* Store accumulated results */
    array1[0] = int_acc;
    array2[1] = fp_acc;
}

/* Main function with architecture-specific code paths */
int main(int argc, char *argv[]) {
    int i, result = 0;
    clock_t start, end;
    
    /* Initialize arrays with pseudo-random but deterministic values */
    srand(42);
    for (i = 0; i < LARGE_SIZE; i++) {
        array1[i] = rand() % 1000;
    }
    for (i = 0; i < LARGE_SIZE * 2; i++) {
        array3[i] = (char)(rand() % 256);
    }
    
    printf("Starting cache-sensitive computations...\n");
    start = clock();
    
    /* Execute different cache-sensitive patterns */
    
    /* 1. Matrix operations - sensitive to cache size and associativity */
    matrix_multiply(256);
    
    /* 2. Stride patterns - tests prefetching and cache line utilization */
    stride_access_pattern(outer_limit, stride);
    
    /* 3. Cache line boundary testing */
    cache_line_test(inner_limit / 100);
    
    /* 4. Mixed data type operations */
    mixed_operations();
    
    /* Architecture-specific code blocks */
    /* These encourage compilation with different -march flags */
    
#ifdef __x86_64__
    /* Code that might benefit from 64-bit specific optimizations */
    unsigned long long big_acc = 0;
    for (i = 0; i < LARGE_SIZE; i++) {
        big_acc += (unsigned long long)array1[i] * i;
    }
    result = (int)(big_acc % 1000000);
#endif

#ifdef __i386__
    /* 32-bit specific patterns */
    int small_matrix[32][32];
    for (i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            small_matrix[i][j] = i * j - result;
        }
    }
#endif

#if defined(__SSE2__) || defined(__AVX__)
    /* Vectorization-friendly code */
    double vector_acc[4] = {0.0, 0.0, 0.0, 0.0};
    for (i = 0; i < MEDIUM_SIZE - 3; i += 4) {
        vector_acc[0] += array2[i];
        vector_acc[1] += array2[i+1];
        vector_acc[2] += array2[i+2];
        vector_acc[3] += array2[i+3];
    }
    array2[0] = vector_acc[0] + vector_acc[1] + vector_acc[2] + vector_acc[3];
#endif

    /* Final reduction to produce observable output */
    for (i = 0; i < LARGE_SIZE; i += 128) {
        result += array1[i];
    }
    
    end = clock();
    
    printf("Computation complete. Result: %d\n", result);
    printf("Time: %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    return result % 100;
}
