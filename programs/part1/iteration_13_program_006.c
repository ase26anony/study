/* cache_detection_test.c
 * Designed to trigger GCC's internal CPU cache detection logic
 * for x86/x86-64 targets, covering specific cache descriptor values
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Force compiler to consider different cache configurations */
#ifdef __x86_64__
#define ARCH_SPECIFIC_CODE 1
#endif

/* Large arrays to exceed typical L1 cache sizes */
#define SIZE_A 10000
#define SIZE_B 5000
#define SIZE_C 2000

/* Volatile to prevent compile-time optimization */
volatile int outer_limit = 100;
volatile int stride = 8;

/* Matrix multiplication kernel - benefits from cache-aware optimization */
void matrix_multiply_kernel(int n, volatile int limit) {
    static double A[SIZE_C][SIZE_C];
    static double B[SIZE_C][SIZE_C];
    static double C[SIZE_C][SIZE_C];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            A[i][j] = (i + j) * 0.1;
            B[i][j] = (i - j) * 0.2;
            C[i][j] = 0.0;
        }
    }
    
    /* Triple nested loop - classic cache optimization target */
    for (int i = 0; i < n; i++) {
        for (int k = 0; k < n; k++) {
            double a = A[i][k];
            for (int j = 0; j < n; j++) {
                C[i][j] += a * B[k][j];
            }
        }
    }
    
    /* Use result to prevent dead code elimination */
    volatile double sum = 0.0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            sum += C[i][j];
        }
    }
    (void)sum; /* Suppress unused warning */
}

/* Non-unit stride access pattern */
void stride_access_kernel(int* array, int size, int stride_val) {
    volatile long long accum = 0;
    
    /* Access every stride_val-th element */
    for (int i = 0; i < size; i += stride_val) {
        array[i] = array[i] * 3 + 1;
        accum += array[i];
    }
    
    /* Another loop with different stride */
    for (int i = 1; i < size; i += stride_val * 2) {
        array[i] = array[i] / 2;
        accum -= array[i];
    }
    
    (void)accum;
}

/* Cache line aliasing test */
void copy_with_aliasing(char* src, char* dst, int size, int offset) {
    /* Copy with potential cache line conflicts */
    for (int i = 0; i < size; i++) {
        dst[(i + offset) % size] = src[i] + src[(i + 1) % size];
    }
    
    /* Reverse copy */
    for (int i = size - 1; i >= 0; i--) {
        src[i] = dst[(i * 2) % size] - 1;
    }
}

/* Mixed data type operations */
void mixed_data_types_kernel(void) {
    static int int_array[SIZE_A];
    static double double_array[SIZE_B];
    static char char_array[SIZE_A * 2];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE_A; i++) {
        int_array[i] = i % 256;
        double_array[i % SIZE_B] = i * 0.01;
        char_array[i] = (char)(i % 128);
    }
    
    /* Operations mixing different data types */
    volatile double result = 0.0;
    for (int i = 0; i < SIZE_A - 1; i++) {
        /* Mix int and double operations */
        double temp = int_array[i] * double_array[i % SIZE_B];
        char_array[i] = (char)(temp / 10.0);
        result += temp;
        
        /* Conditional with data-dependent access */
        if (i % 16 == 0) {
            int_array[i] = (int)(double_array[(i / 16) % SIZE_B] * 100);
        }
    }
    
    (void)result;
}

/* Architecture-specific code blocks */
#ifdef __x86_64__
void x86_64_specific_workload(void) {
    /* Code that might benefit from specific x86-64 cache optimizations */
    static long long big_array[SIZE_A];
    volatile long long sum = 0;
    
    for (int i = 0; i < SIZE_A; i++) {
        big_array[i] = i * i;
        if (i % 64 == 0) { /* Cache line boundary */
            sum += big_array[i];
        }
    }
    
    /* Nested loops with varying access patterns */
    for (int block = 0; block < 16; block++) {
        for (int i = block * 64; i < (block + 1) * 64 && i < SIZE_A; i++) {
            big_array[i] = (big_array[i] << 3) | (big_array[i] >> 5);
        }
    }
    
    (void)sum;
}
#endif

/* Main function with multiple optimization targets */
int main(int argc, char** argv) {
    /* Use arguments to make bounds dynamic */
    int matrix_size = (argc > 1) ? atoi(argv[1]) : 100;
    int iter_count = (argc > 2) ? atoi(argv[2]) : 5;
    
    if (matrix_size > SIZE_C) matrix_size = SIZE_C;
    
    printf("Starting cache-sensitive workload...\n");
    printf("Matrix size: %d, Iterations: %d\n", matrix_size, iter_count);
    
    /* Allocate heap memory for additional cache pressure */
    int* heap_array1 = (int*)malloc(SIZE_A * sizeof(int));
    int* heap_array2 = (int*)malloc(SIZE_A * sizeof(int));
    char* char_buffer = (char*)malloc(SIZE_A * 2);
    
    if (!heap_array1 || !heap_array2 || !char_buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize heap arrays */
    for (int i = 0; i < SIZE_A; i++) {
        heap_array1[i] = i;
        heap_array2[i] = SIZE_A - i;
    }
    memset(char_buffer, 'A', SIZE_A * 2);
    
    /* Execute multiple kernels to give compiler optimization opportunities */
    for (int iter = 0; iter < iter_count; iter++) {
        /* Each kernel represents different cache access patterns */
        matrix_multiply_kernel(matrix_size, outer_limit);
        
        stride_access_kernel(heap_array1, SIZE_A, stride + iter);
        
        copy_with_aliasing(char_buffer, char_buffer + SIZE_A, SIZE_A, 64);
        
        mixed_data_types_kernel();
        
        #ifdef __x86_64__
        x86_64_specific_workload();
        #endif
        
        /* Vary stride to prevent pattern recognition */
        stride = (stride % 16) + 4;
    }
    
    /* Final computation using all arrays */
    volatile long long final_sum = 0;
    for (int i = 0; i < SIZE_A; i++) {
        final_sum += heap_array1[i] + heap_array2[i] + char_buffer[i];
    }
    
    printf("Final checksum: %lld\n", final_sum);
    
    /* Cleanup */
    free(heap_array1);
    free(heap_array2);
    free(char_buffer);
    
    return 0;
}
