/* cache_detection_test.c
 * Designed to trigger GCC driver's CPU cache detection logic
 * Compile with various x86-specific flags to exercise cache descriptor cases
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Force compiler to consider cache parameters for these arrays */
#define LARGE_SIZE 10000
#define MEDIUM_SIZE 5000
#define SMALL_SIZE 1000

/* Volatile variables to prevent compile-time optimization */
volatile int v_limit = LARGE_SIZE;
volatile int v_stride = 16;
volatile int v_seed = 42;

/* Function prototypes to prevent inlining */
double compute_matrix(int size);
void stride_access_pattern(int* array, int size, int stride);
void cache_line_test(char* src, char* dst, int size);

int main(int argc, char** argv) {
    /* Use command line args to vary behavior */
    int matrix_size = (argc > 1) ? atoi(argv[1]) : MEDIUM_SIZE;
    int use_prefetch = (argc > 2) ? atoi(argv[2]) : 1;
    
    /* Allocate arrays that exceed typical L1 cache */
    int* array1 = (int*)malloc(LARGE_SIZE * sizeof(int));
    int* array2 = (int*)malloc(LARGE_SIZE * sizeof(int));
    double* matrix_a = (double*)malloc(matrix_size * matrix_size * sizeof(double));
    double* matrix_b = (double*)malloc(matrix_size * matrix_size * sizeof(double));
    char* buffer1 = (char*)malloc(LARGE_SIZE * 64); /* 64-byte aligned for cache lines */
    char* buffer2 = (char*)malloc(LARGE_SIZE * 64);
    
    if (!array1 || !array2 || !matrix_a || !matrix_b || !buffer1 || !buffer2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(v_seed);
    for (int i = 0; i < LARGE_SIZE; i++) {
        array1[i] = rand() % 100;
        array2[i] = 0;
    }
    
    for (int i = 0; i < matrix_size * matrix_size; i++) {
        matrix_a[i] = (double)rand() / RAND_MAX;
        matrix_b[i] = (double)rand() / RAND_MAX;
    }
    
    memset(buffer1, 'A', LARGE_SIZE * 64);
    memset(buffer2, 0, LARGE_SIZE * 64);
    
    /* Test 1: Matrix multiplication-like triple nested loop */
    /* This pattern heavily depends on cache blocking optimizations */
    printf("Starting matrix computation...\n");
    double result = compute_matrix(matrix_size);
    printf("Matrix result: %f\n", result);
    
    /* Test 2: Non-unit stride access pattern */
    /* Compiler may use prefetching based on cache line size */
    printf("Performing stride access test...\n");
    stride_access_pattern(array1, v_limit, v_stride);
    
    /* Test 3: Cache line copying with potential aliasing */
    printf("Testing cache line behavior...\n");
    cache_line_test(buffer1, buffer2, LARGE_SIZE);
    
    /* Test 4: Mixed data type operations */
    /* Accesses different cache levels due to varying data sizes */
    {
        int sum_int = 0;
        double sum_double = 0.0;
        
        for (int i = 0; i < MEDIUM_SIZE; i += 8) {
            /* Mix int and double operations */
            sum_int += array1[i];
            sum_double += matrix_a[i % matrix_size];
            
            /* Conditional to prevent vectorization from skipping cache considerations */
            if (sum_int % 7 == 0) {
                array2[i] = sum_int;
            }
        }
        printf("Mixed type sums: int=%d, double=%f\n", sum_int, sum_double);
    }
    
    /* Test 5: Nested loops with varying access patterns */
    {
        double temp = 0.0;
        int block_size = 64; /* Typical cache line size */
        
        for (int i = 0; i < MEDIUM_SIZE; i += block_size) {
            for (int j = 0; j < MEDIUM_SIZE; j += block_size) {
                int limit_i = (i + block_size < MEDIUM_SIZE) ? i + block_size : MEDIUM_SIZE;
                int limit_j = (j + block_size < MEDIUM_SIZE) ? j + block_size : MEDIUM_SIZE;
                
                for (int ii = i; ii < limit_i; ii++) {
                    for (int jj = j; jj < limit_j; jj++) {
                        /* Simulate small matrix block operation */
                        int idx = ii * MEDIUM_SIZE + jj;
                        temp += array1[idx % LARGE_SIZE] * 0.5;
                    }
                }
            }
        }
        printf("Blocked computation result: %f\n", temp);
    }
    
    /* Clean up */
    free(array1);
    free(array2);
    free(matrix_a);
    free(matrix_b);
    free(buffer1);
    free(buffer2);
    
    return 0;
}

/* Matrix computation function - triggers cache-aware optimizations */
double compute_matrix(int size) {
    double* temp = (double*)malloc(size * size * sizeof(double));
    if (!temp) return 0.0;
    
    double sum = 0.0;
    
    /* Triple nested loop - compiler may apply cache blocking */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            double acc = 0.0;
            for (int k = 0; k < size; k++) {
                acc += matrix_a[i * size + k] * matrix_b[k * size + j];
            }
            temp[i * size + j] = acc;
            sum += acc;
        }
    }
    
    /* Additional processing to ensure computation isn't optimized away */
    for (int i = 0; i < size * size; i += 17) {
        temp[i] *= 1.0001;
        sum += temp[i];
    }
    
    free(temp);
    return sum;
}

/* Stride access pattern - tests prefetching decisions */
void stride_access_pattern(int* array, int size, int stride) {
    int result = 0;
    
    /* Non-unit stride accesses challenge cache efficiency */
    for (int i = 0; i < size; i += stride) {
        result += array[i];
        
        /* Add dependency to prevent reordering */
        if (i > 0) {
            array[i] += array[i - stride] % 256;
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Stride access result: %d\n", result);
}

/* Cache line test - potential for cache line aliasing */
void cache_line_test(char* src, char* dst, int size) {
    int checksum = 0;
    
    /* Copy with potential cache line conflicts */
    for (int i = 0; i < size * 64; i += 64) {
        /* Copy entire cache line */
        for (int j = 0; j < 64; j++) {
            dst[i + j] = src[i + j];
            checksum += dst[i + j];
        }
        
        /* Interleave with another access pattern */
        if (i % 128 == 0) {
            for (int j = 0; j < 32; j++) {
                dst[i + j * 2] ^= 0x55;
            }
        }
    }
    
    printf("Cache line test checksum: %d\n", checksum);
}

/* Conditional compilation for different x86 architectures */
#ifdef __x86_64__
/* Code that might benefit from specific x86 cache optimizations */
void x86_specific_optimization() {
    /* Use inline assembly hint to suggest cache considerations */
    asm volatile("" ::: "memory");
    
    /* Array operations that might use SSE/AVX depending on -march */
    float farray[1024];
    for (int i = 0; i < 1024; i++) {
        farray[i] = i * 0.1f;
    }
}
#endif

#ifdef __i386__
/* 32-bit specific optimizations */
void i386_specific_optimization() {
    /* Different memory access pattern for 32-bit */
    int small_array[512];
    for (int i = 0; i < 512; i++) {
        small_array[i] = i * 2;
    }
}
#endif
