/* cache_detection_test.c
 * Designed to trigger GCC driver's CPU cache detection logic
 * for x86 cache descriptor values: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c,
 * 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x41-0x45, 0x48, 0x49, 0x4e,
 * 0x60, 0x66-0x68, 0x78-0x80, 0x82-0x87
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force compiler to consider cache-aware optimizations */
#define ARRAY_SIZE 10000
#define MATRIX_SIZE 512

/* Volatile variables to prevent compile-time optimization */
volatile int v_size = ARRAY_SIZE;
volatile int v_matrix = MATRIX_SIZE;

/* Large arrays that exceed typical L1 cache sizes */
static int int_array1[ARRAY_SIZE];
static int int_array2[ARRAY_SIZE];
static double double_array1[ARRAY_SIZE];
static double double_array2[ARRAY_SIZE];
static char char_array[ARRAY_SIZE * 4];  /* Larger byte array */

/* Matrix for multiplication test */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];

/* Function prototypes to force different optimization contexts */
void matrix_multiply(int size);
void stride_access_pattern(int size, int stride);
void cache_line_aliasing_test(int size);
void mixed_data_type_operations(int size);

/* Explicit CPU detection if available */
#ifdef __x86_64__
void check_cpu_features(void) {
    /* These builtins may trigger CPUID queries */
    __builtin_cpu_init();
    if (__builtin_cpu_supports("sse")) {
        /* SSE support check */
    }
    if (__builtin_cpu_supports("avx")) {
        /* AVX support check */
    }
}
#endif

/* Matrix multiplication - classic cache-sensitive algorithm */
void matrix_multiply(int size) {
    int i, j, k;
    double sum;
    
    /* Initialize matrices */
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            matrix_a[i][j] = (i + j) * 0.1;
            matrix_b[i][j] = (i - j) * 0.1;
            matrix_c[i][j] = 0.0;
        }
    }
    
    /* Triple nested loop - highly cache sensitive */
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            sum = 0.0;
            for (k = 0; k < size; k++) {
                sum += matrix_a[i][k] * matrix_b[k][j];
            }
            matrix_c[i][j] = sum;
        }
    }
}

/* Non-unit stride access pattern */
void stride_access_pattern(int size, int stride) {
    int i;
    double result = 0.0;
    
    /* Access every 'stride' element - tests cache line utilization */
    for (i = 0; i < size; i += stride) {
        double_array1[i] = double_array2[i] * 1.5;
        result += double_array1[i];
    }
    
    /* Reverse stride pattern */
    for (i = size - 1; i >= 0; i -= stride) {
        int_array1[i] = int_array2[i] * 2;
        result += int_array1[i];
    }
    
    /* Prevent dead code elimination */
    if (result < 0) printf("Impossible\n");
}

/* Test potential cache line aliasing */
void cache_line_aliasing_test(int size) {
    int i, j;
    const int block_size = 64; /* Typical cache line size */
    
    /* Access arrays with potential aliasing */
    for (i = 0; i < size; i += block_size) {
        for (j = 0; j < block_size && (i + j) < size; j++) {
            int index = i + j;
            int_array1[index] = int_array2[index] + char_array[index];
            char_array[index] = (char)(int_array1[index] & 0xFF);
        }
    }
    
    /* Interleaved access pattern */
    for (i = 0; i < size; i++) {
        double_array1[i] = (double)int_array1[i % 256] * 0.5;
    }
}

/* Mixed data type operations */
void mixed_data_type_operations(int size) {
    int i;
    double acc_double = 0.0;
    int acc_int = 0;
    
    /* Operations on different data types */
    for (i = 0; i < size; i++) {
        /* Mix int and double operations */
        int_array1[i] = i * 2;
        double_array1[i] = int_array1[i] * 1.414;
        
        /* Char array access with different stride */
        char_array[i * 4] = (char)(int_array1[i] & 0xFF);
        char_array[i * 4 + 1] = (char)((int_array1[i] >> 8) & 0xFF);
        
        /* Accumulate results */
        acc_double += double_array1[i];
        acc_int += int_array1[i];
    }
    
    /* Cross-array operations */
    for (i = 1; i < size; i++) {
        double_array2[i] = double_array1[i] + double_array1[i-1];
        int_array2[i] = int_array1[i] - int_array1[i-1];
    }
}

/* Main function with architecture-specific code paths */
int main(int argc, char **argv) {
    int size = v_size;
    int matrix_size = v_matrix;
    int stride = 8; /* Non-power-of-two stride */
    double total_result = 0.0;
    
    /* Initialize with pseudo-random but reproducible values */
    srand(42);
    for (int i = 0; i < size; i++) {
        int_array1[i] = rand() % 100;
        int_array2[i] = rand() % 100;
        double_array1[i] = (double)(rand() % 100) / 10.0;
        double_array2[i] = (double)(rand() % 100) / 10.0;
    }
    
    /* Architecture-specific code blocks */
#ifdef __x86_64__
    /* x86-64 specific optimizations */
    check_cpu_features();
    
    /* Different code paths for different march scenarios */
#if defined(__tune_core2__) || defined(__tune_nocona__)
    /* Core2/Nocona specific patterns */
    stride = 16;
#elif defined(__tune_nehalem__) || defined(__tune_westmere__)
    /* Nehalem/Westmere patterns */
    stride = 32;
#else
    /* Generic x86-64 */
    stride = 8;
#endif
#endif
    
#ifdef __i386__
    /* 32-bit x86 specific */
    stride = 4;
#endif
    
    /* Execute cache-sensitive algorithms */
    matrix_multiply(matrix_size);
    
    /* Vary stride patterns to test different cache behaviors */
    stride_access_pattern(size, stride);
    stride_access_pattern(size, stride * 2);
    stride_access_pattern(size, stride / 2);
    
    /* Cache line and aliasing tests */
    cache_line_aliasing_test(size);
    
    /* Mixed operations */
    mixed_data_type_operations(size);
    
    /* Compute final result to prevent dead code elimination */
    for (int i = 0; i < size; i += 64) {
        total_result += int_array1[i] + double_array1[i];
    }
    
    /* Use result to prevent optimization */
    printf("Cache test result: %f\n", total_result);
    
    return 0;
}
