/* cache_detection_test.c
 * Designed to trigger CPU cache descriptor detection in GCC driver
 * Compile with various x86-specific flags to exercise cache detection logic
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

/* Large arrays that exceed typical L1 cache sizes */
static int large_array1[ARRAY_SIZE];
static int large_array2[ARRAY_SIZE];
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];
static char char_buffer[ARRAY_SIZE * 4];

/* Function prototypes to prevent inlining */
double __attribute__((noinline)) matrix_multiply(int size);
void __attribute__((noinline)) stride_access(int stride, int limit);
void __attribute__((noinline)) cache_line_test(int iterations);

/* Matrix multiplication - classic cache-sensitive algorithm */
double matrix_multiply(int size) {
    double sum = 0.0;
    
    /* Initialize matrices */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            matrix_a[i][j] = (i + j) * 0.1;
            matrix_b[i][j] = (i - j) * 0.2;
            matrix_c[i][j] = 0.0;
        }
    }
    
    /* Triple nested loop - highly cache sensitive */
    for (int i = 0; i < size; i++) {
        for (int k = 0; k < size; k++) {
            double aik = matrix_a[i][k];
            for (int j = 0; j < size; j++) {
                matrix_c[i][j] += aik * matrix_b[k][j];
            }
        }
    }
    
    /* Accumulate result */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            sum += matrix_c[i][j];
        }
    }
    
    return sum;
}

/* Non-unit stride access pattern */
void stride_access(int stride, int limit) {
    volatile int result = 0;
    
    /* Access every Nth element - tests cache line utilization */
    for (int i = 0; i < limit; i += stride) {
        large_array1[i] = large_array2[i] * 2 + i;
        result += large_array1[i];
    }
    
    /* Reverse stride access */
    for (int i = limit - 1; i >= 0; i -= stride) {
        large_array2[i] = large_array1[i] / 2 - i;
        result -= large_array2[i];
    }
    
    /* Prevent dead code elimination */
    if (result < 0) {
        printf("Stride result: %d\n", result);
    }
}

/* Test potential cache line aliasing */
void cache_line_test(int iterations) {
    int block_size = 64; /* Typical cache line size */
    int offset = 16;     /* Potential misalignment */
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Copy with potential cache line conflicts */
        for (int i = 0; i < ARRAY_SIZE - block_size; i += block_size) {
            memcpy(&char_buffer[i + offset], 
                   &char_buffer[i], 
                   block_size - offset);
        }
        
        /* Scatter-gather pattern */
        for (int i = 0; i < ARRAY_SIZE; i += 8) {
            char_buffer[i] = (char_buffer[i] + char_buffer[i + 4]) / 2;
            char_buffer[i + 1] = (char_buffer[i + 1] + char_buffer[i + 5]) / 2;
            char_buffer[i + 2] = (char_buffer[i + 2] + char_buffer[i + 6]) / 2;
            char_buffer[i + 3] = (char_buffer[i + 3] + char_buffer[i + 7]) / 2;
        }
    }
}

/* Conditional compilation for different x86 architectures */
#ifdef __x86_64__
void x86_64_specific_code(void) {
    /* Code that might benefit from specific x86-64 cache optimizations */
    __builtin_cpu_init();
    
    /* Check for specific CPU features that imply certain cache descriptors */
    if (__builtin_cpu_supports("sse3")) {
        /* SSE3 often comes with specific cache configurations */
        printf("SSE3 detected - may imply specific cache descriptors\n");
    }
    
    if (__builtin_cpu_supports("avx")) {
        /* AVX processors have specific cache characteristics */
        printf("AVX detected - checking cache hierarchy\n");
    }
}
#endif

#ifdef __i386__
void i386_specific_code(void) {
    /* Legacy x86 code paths */
    printf("32-bit x86 mode - different cache detection path\n");
}
#endif

int main(int argc, char *argv[]) {
    double total = 0.0;
    int i, j;
    
    /* Initialize data with non-zero values */
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; i++) {
        large_array1[i] = rand() % 100;
        large_array2[i] = rand() % 100;
    }
    
    for (i = 0; i < ARRAY_SIZE * 4; i++) {
        char_buffer[i] = (char)(rand() % 256);
    }
    
    printf("Starting cache-sensitive computations...\n");
    
    /* Execute different cache-sensitive patterns */
    
    /* 1. Matrix multiplication - tests L1/L2 cache blocking */
    total += matrix_multiply(matrix_dim);
    
    /* 2. Various stride patterns */
    stride_access(1, outer_limit);      /* Unit stride */
    stride_access(2, outer_limit);      /* 2-element stride */
    stride_access(4, outer_limit);      /* 4-element stride (common for int) */
    stride_access(8, outer_limit);      /* 8-element stride */
    stride_access(16, outer_limit);     /* 16-element stride */
    stride_access(32, outer_limit);     /* 32-element stride */
    
    /* 3. Cache line aliasing test */
    cache_line_test(10);
    
    /* 4. Mixed data type access pattern */
    for (i = 0; i < ARRAY_SIZE - 100; i += 67) { /* Prime number stride */
        double temp = 0.0;
        for (j = 0; j < 100; j++) {
            temp += large_array1[i + j] * 0.5;
            char_buffer[i + j] = (char)(large_array2[i + j] & 0xFF);
        }
        total += temp;
    }
    
    /* 5. Architecture-specific paths */
#if defined(__x86_64__)
    x86_64_specific_code();
#elif defined(__i386__)
    i386_specific_code();
#endif
    
    /* Final result to prevent dead code elimination */
    printf("Total accumulation: %f\n", total);
    printf("Array element at midpoint: %d\n", large_array1[ARRAY_SIZE/2]);
    
    return 0;
}
