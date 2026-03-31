/* cache_detection_test.c
 * Designed to trigger GCC driver cache detection for x86 targets
 * Compile with various -march and -mtune options to exercise different cache descriptor cases
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force compiler to consider different cache configurations */
#ifdef __x86_64__
#define CACHE_LINE 64
#else
#define CACHE_LINE 32
#endif

/* Large arrays to exceed typical L1/L2 cache sizes */
#define SIZE_A 10000
#define SIZE_B 5000
#define SIZE_C 2000

/* Volatile to prevent compile-time optimization */
volatile int outer_limit = 100;
volatile int stride = 7;

/* Matrix multiplication kernel - benefits from cache-aware optimizations */
void matrix_multiply_kernel(int n, double* A, double* B, double* C) {
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

/* Non-unit stride access pattern - tests cache line utilization */
double stride_access_kernel(double* array, int size, int stride_val) {
    double result = 0.0;
    for (int i = 0; i < size; i += stride_val) {
        array[i] = array[i] * 1.01 + i;
        result += array[i];
    }
    return result;
}

/* Cache line aliasing test - copying with potential conflicts */
void cache_line_copy(int* src, int* dst, int size) {
    for (int i = 0; i < size; i += CACHE_LINE / sizeof(int)) {
        dst[i] = src[i] * 2 - src[size - i - 1];
    }
}

/* Mixed data type operations - different access patterns */
void mixed_data_kernel(char* char_arr, int* int_arr, double* double_arr, int size) {
    for (int i = 0; i < size; i++) {
        char_arr[i] = (char)(i % 256);
        int_arr[i] = i * 2 + char_arr[i];
        double_arr[i] = int_arr[i] * 0.5 + char_arr[i] * 0.1;
    }
}

int main(int argc, char** argv) {
    /* Use command line args to vary behavior and prevent optimization */
    int matrix_size = (argc > 1) ? atoi(argv[1]) : 128;
    if (matrix_size > 500) matrix_size = 500; /* Limit for reasonable runtime */
    
    /* Allocate large arrays that exceed typical cache sizes */
    double* matrix_A = (double*)malloc(SIZE_A * sizeof(double));
    double* matrix_B = (double*)malloc(SIZE_B * sizeof(double));
    double* matrix_C = (double*)malloc(SIZE_C * sizeof(double));
    
    int* int_array1 = (int*)malloc(SIZE_A * sizeof(int));
    int* int_array2 = (int*)malloc(SIZE_A * sizeof(int));
    char* char_array = (char*)malloc(SIZE_B * sizeof(char));
    
    if (!matrix_A || !matrix_B || !matrix_C || !int_array1 || !int_array2 || !char_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random but deterministic values */
    srand(42);
    for (int i = 0; i < SIZE_A; i++) {
        matrix_A[i] = (double)(rand() % 1000) / 100.0;
        int_array1[i] = rand() % 10000;
    }
    for (int i = 0; i < SIZE_B; i++) {
        matrix_B[i] = (double)(rand() % 1000) / 100.0;
        char_array[i] = (char)(rand() % 256);
    }
    
    double total_result = 0.0;
    
    /* Execute different loop patterns to trigger various cache optimizations */
    
    /* Pattern 1: Matrix multiplication - benefits from cache blocking */
    if (matrix_size > 10) {
        int small_size = matrix_size;
        double* small_A = (double*)malloc(small_size * small_size * sizeof(double));
        double* small_B = (double*)malloc(small_size * small_size * sizeof(double));
        double* small_C = (double*)malloc(small_size * small_size * sizeof(double));
        
        if (small_A && small_B && small_C) {
            for (int i = 0; i < small_size * small_size; i++) {
                small_A[i] = (double)(i % 100) * 0.1;
                small_B[i] = (double)((i + 1) % 100) * 0.2;
            }
            
            matrix_multiply_kernel(small_size, small_A, small_B, small_C);
            
            /* Use result to prevent dead code elimination */
            for (int i = 0; i < small_size * small_size; i++) {
                total_result += small_C[i];
            }
            
            free(small_A);
            free(small_B);
            free(small_C);
        }
    }
    
    /* Pattern 2: Non-unit stride access */
    total_result += stride_access_kernel(matrix_A, SIZE_A, stride);
    
    /* Pattern 3: Cache line copying with potential aliasing */
    cache_line_copy(int_array1, int_array2, SIZE_A);
    for (int i = 0; i < SIZE_A; i += 256) {
        total_result += int_array2[i];
    }
    
    /* Pattern 4: Mixed data type operations */
    mixed_data_kernel(char_array, int_array1, matrix_B, SIZE_B / 2);
    for (int i = 0; i < SIZE_B / 2; i += 128) {
        total_result += matrix_B[i] + int_array1[i] + char_array[i];
    }
    
    /* Pattern 5: Additional nested loops with varying bounds */
    for (int i = 0; i < outer_limit; i++) {
        for (int j = 0; j < i % 16 + 1; j++) {
            for (int k = 0; k < 100; k++) {
                matrix_C[(i * 17 + j) % SIZE_C] += 
                    matrix_A[(i * 13 + k) % SIZE_A] * 
                    matrix_B[(j * 19 + k) % SIZE_B];
            }
        }
    }
    
    /* Use results to prevent optimization */
    printf("Total result: %f\n", total_result);
    
    /* Clean up */
    free(matrix_A);
    free(matrix_B);
    free(matrix_C);
    free(int_array1);
    free(int_array2);
    free(char_array);
    
    return 0;
}
