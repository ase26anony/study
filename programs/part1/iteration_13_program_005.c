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

/* Force compiler to consider cache-aware optimizations */
#define ARRAY_SIZE 10000
#define BLOCK_SIZE 64

/* Volatile variables to prevent compile-time optimization */
volatile int outer_limit = ARRAY_SIZE;
volatile int stride = 16;

/* Different loop patterns to trigger various cache optimizations */

/* Pattern 1: Matrix multiplication-like triple nested loop */
void matrix_multiply_pattern(int n, double* restrict A, double* restrict B, double* restrict C) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += A[i * n + k] * B[k * n + j];
            }
            C[i * n + j] = sum;
        }
    }
}

/* Pattern 2: Non-unit stride access with cache line considerations */
void stride_access_pattern(int* array, int size, int stride_val) {
    long long sum = 0;
    for (int i = 0; i < size; i += stride_val) {
        array[i] = array[i] * 3 + 1;
        sum += array[i];
    }
    /* Use sum to prevent dead code elimination */
    printf("Stride pattern sum: %lld\n", sum);
}

/* Pattern 3: Blocked matrix transposition for cache locality */
void blocked_transpose(int n, double* restrict src, double* restrict dst) {
    int block = BLOCK_SIZE;
    for (int i = 0; i < n; i += block) {
        for (int j = 0; j < n; j += block) {
            for (int ii = i; ii < i + block && ii < n; ii++) {
                for (int jj = j; jj < j + block && jj < n; jj++) {
                    dst[jj * n + ii] = src[ii * n + jj];
                }
            }
        }
    }
}

/* Pattern 4: Mixed data type operations */
void mixed_data_pattern(char* char_arr, int* int_arr, double* double_arr, int size) {
    for (int i = 0; i < size; i++) {
        int_arr[i] = char_arr[i] * 2;
        double_arr[i] = int_arr[i] * 1.5;
        char_arr[i] = (char)(double_arr[i] / 3.0);
    }
}

/* Pattern 5: Loop with potential cache line aliasing */
void aliasing_pattern(int* arr1, int* arr2, int size) {
    /* arr2 might alias with arr1 + offset */
    for (int i = 0; i < size - 1; i++) {
        arr1[i] = arr2[i + 1] + arr1[i] * 2;
    }
}

int main(int argc, char** argv) {
    /* Use command line args to vary behavior and prevent optimization */
    int matrix_size = (argc > 1) ? atoi(argv[1]) : 256;
    int use_stride = (argc > 2) ? atoi(argv[2]) : 8;
    
    /* Allocate large arrays to exceed L1 cache */
    double* matrixA = (double*)aligned_alloc(64, matrix_size * matrix_size * sizeof(double));
    double* matrixB = (double*)aligned_alloc(64, matrix_size * matrix_size * sizeof(double));
    double* matrixC = (double*)aligned_alloc(64, matrix_size * matrix_size * sizeof(double));
    
    int* int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    char* char_array = (char*)malloc(ARRAY_SIZE * sizeof(char));
    double* double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    /* Initialize arrays with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < matrix_size * matrix_size; i++) {
        matrixA[i] = (double)rand() / RAND_MAX;
        matrixB[i] = (double)rand() / RAND_MAX;
    }
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = rand() % 100;
        char_array[i] = (char)(rand() % 256);
        double_array[i] = (double)rand() / RAND_MAX;
    }
    
    /* Execute different patterns to trigger various cache optimizations */
    
    /* Pattern 1: Matrix multiplication */
    matrix_multiply_pattern(matrix_size, matrixA, matrixB, matrixC);
    
    /* Pattern 2: Stride access */
    stride_access_pattern(int_array, ARRAY_SIZE, use_stride);
    
    /* Pattern 3: Blocked transpose */
    blocked_transpose(matrix_size, matrixA, matrixC);
    
    /* Pattern 4: Mixed data types */
    mixed_data_pattern(char_array, int_array, double_array, ARRAY_SIZE / 4);
    
    /* Pattern 5: Potential aliasing */
    aliasing_pattern(int_array, int_array + ARRAY_SIZE / 2, ARRAY_SIZE / 2);
    
    /* Compute and print a result to prevent dead code elimination */
    double total = 0.0;
    for (int i = 0; i < matrix_size * matrix_size; i += matrix_size + 1) {
        total += matrixC[i];
    }
    printf("Matrix trace: %f\n", total);
    
    /* Clean up */
    free(matrixA);
    free(matrixB);
    free(matrixC);
    free(int_array);
    free(char_array);
    free(double_array);
    
    return 0;
}

/* Conditional compilation for different x86 architectures */
#ifdef __x86_64__
/* Code that might benefit from specific x86 cache optimizations */
void x86_specific_optimizations() {
    /* Use __builtin_cpu functions to explicitly trigger CPU detection */
    __builtin_cpu_init();
    
    /* Check for specific features that might influence cache detection */
    if (__builtin_cpu_supports("sse2")) {
        /* SSE2-specific operations */
    }
    
    if (__builtin_cpu_supports("avx")) {
        /* AVX-specific operations */
    }
}
#endif

/* Additional architecture-specific blocks */
#if defined(__i386__) || defined(__x86_64__)
/* Force consideration of various cache configurations */
void cache_sensitive_loop(int* data, int size) {
    /* Loop designed to be sensitive to cache line size */
    for (int i = 0; i < size; i += 64 / sizeof(int)) {
        data[i] = data[i] * 3 - 1;
    }
}
#endif
