/* 
 * cache_detection_test.c
 * Designed to trigger GCC driver's CPU cache detection logic
 * for x86 cache descriptor values in driver-i386.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Use volatile to prevent compile-time optimization */
volatile int outer_limit = 1000;
volatile int inner_limit = 1000;
volatile int stride = 16;

/* Large arrays that exceed typical L1 cache sizes */
#define LARGE_SIZE 10000
static int array1[LARGE_SIZE];
static int array2[LARGE_SIZE];
static double darray1[LARGE_SIZE];
static double darray2[LARGE_SIZE];
static char carray1[LARGE_SIZE];
static char carray2[LARGE_SIZE];

/* Matrix for matrix multiplication */
#define MATRIX_SIZE 256
static int matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static int matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static int matrix_c[MATRIX_SIZE][MATRIX_SIZE];

/* Function prototypes to force different optimization considerations */
void matrix_multiply(int size);
void stride_access_pattern(int limit, int step);
void cache_line_aliasing_test(int iterations);
void mixed_data_type_operations(void);

/* Main computational kernel with varied access patterns */
void matrix_multiply(int size) {
    int i, j, k;
    volatile int sum;
    
    /* Classic matrix multiplication - benefits from cache blocking */
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            sum = 0;
            for (k = 0; k < size; k++) {
                sum += matrix_a[i][k] * matrix_b[k][j];
            }
            matrix_c[i][j] = sum;
        }
    }
}

/* Non-unit stride access pattern */
void stride_access_pattern(int limit, int step) {
    int i;
    long long accumulator = 0;
    
    /* Access every 'step'th element - tests cache line utilization */
    for (i = 0; i < limit; i += step) {
        accumulator += array1[i];
        array2[i] = accumulator % 1000;
    }
    
    /* Reverse stride pattern */
    for (i = limit - 1; i >= 0; i -= step) {
        accumulator += array2[i];
        array1[i] = accumulator % 1000;
    }
}

/* Test potential cache line aliasing */
void cache_line_aliasing_test(int iterations) {
    int i, j;
    volatile int temp;
    
    /* Access arrays with offsets that might cause cache conflicts */
    for (j = 0; j < iterations; j++) {
        for (i = 0; i < LARGE_SIZE - 64; i += 64) {
            temp = array1[i] + array2[i + 32];
            darray1[i / 2] = temp * 0.5;
            darray2[i / 2] = darray1[i / 2] * 1.5;
        }
    }
}

/* Mixed data type operations */
void mixed_data_type_operations(void) {
    int i;
    double d_acc = 0.0;
    int i_acc = 0;
    
    /* Mix operations on different data types */
    for (i = 0; i < LARGE_SIZE; i++) {
        /* Integer operations */
        i_acc += array1[i] * 2;
        
        /* Floating point operations */
        d_acc += darray1[i] * 1.1;
        
        /* Char operations with type conversion */
        carray1[i] = (char)(i_acc % 256);
        carray2[i] = carray1[i] + (char)(d_acc * 0.01);
        
        /* Store results back */
        array2[i] = i_acc;
        darray2[i] = d_acc;
    }
}

/* Initialize arrays with pseudo-random but deterministic values */
void initialize_arrays(void) {
    int i, j;
    
    for (i = 0; i < LARGE_SIZE; i++) {
        array1[i] = (i * 37) % 1000;
        array2[i] = (i * 73) % 1000;
        darray1[i] = (double)(i % 100) * 0.1;
        darray2[i] = (double)(i % 200) * 0.05;
        carray1[i] = (char)(i % 256);
        carray2[i] = (char)((i * 13) % 256);
    }
    
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = (i + j) % 100;
            matrix_b[i][j] = (i * j) % 100;
            matrix_c[i][j] = 0;
        }
    }
}

/* Main function with architecture-specific code paths */
int main(int argc, char *argv[]) {
    int i, iterations = 10;
    long long total_result = 0;
    clock_t start, end;
    
    /* Initialize with deterministic but non-constant values */
    initialize_arrays();
    
    printf("Starting cache-sensitive computation...\n");
    start = clock();
    
    /* 
     * Architecture-specific code blocks to encourage testing
     * with different -march flags
     */
    
#ifdef __x86_64__
    /* Code optimized for x86-64 with various cache descriptors */
    printf("Compiled for x86-64 architecture\n");
    
    /* Try to use CPU detection builtins if available */
    #ifdef __GNUC__
    /* This may prompt driver to initialize CPU features */
    __builtin_cpu_init();
    #endif
    
    /* Multiple computation patterns to utilize different cache levels */
    for (i = 0; i < iterations; i++) {
        /* Pattern 1: Matrix multiplication (benefits from L1/L2 cache blocking) */
        matrix_multiply(MATRIX_SIZE);
        
        /* Pattern 2: Stride access (tests cache line utilization) */
        stride_access_pattern(outer_limit, stride);
        
        /* Pattern 3: Mixed operations */
        mixed_data_type_operations();
        
        /* Pattern 4: Cache aliasing test */
        if (i % 3 == 0) {
            cache_line_aliasing_test(5);
        }
        
        /* Accumulate some result to prevent dead code elimination */
        total_result += array1[i % LARGE_SIZE] + (int)darray1[i % LARGE_SIZE];
    }
#endif

#ifdef __i386__
    /* Code for 32-bit x86 with potentially different cache descriptors */
    printf("Compiled for i386 architecture\n");
    
    /* Simpler patterns for 32-bit */
    for (i = 0; i < iterations * 2; i++) {
        stride_access_pattern(500, 8);
        mixed_data_type_operations();
        total_result += array2[i % 500];
    }
#endif

#ifdef __SSE2__
    /* Code that might use SSE2 optimizations */
    printf("SSE2 instructions available\n");
#endif

#ifdef __AVX__
    /* Code that might use AVX optimizations */
    printf("AVX instructions available\n");
#endif

    end = clock();
    
    /* Use the computed result */
    printf("Computation completed. Result checksum: %lld\n", total_result);
    printf("Time elapsed: %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Additional print to ensure arrays are used */
    printf("Sample values: array1[100]=%d, darray1[200]=%.2f\n", 
           array1[100], darray1[200]);
    
    return 0;
}
