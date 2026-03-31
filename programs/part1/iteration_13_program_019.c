/* cache_detection_test.c
 * Designed to trigger GCC driver's CPU cache detection logic
 * for x86 cache descriptor values: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c,
 * 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x41-0x45, 0x48, 0x49, 0x4e, 0x60,
 * 0x66-0x68, 0x78-0x80, 0x82-0x87
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Use volatile to prevent compile-time optimization */
volatile int outer_limit = 1000;
volatile int inner_limit = 500;
volatile int stride = 8;

/* Large arrays to exceed L1 cache */
#define LARGE_SIZE 10000
static int array1[LARGE_SIZE];
static double array2[LARGE_SIZE];
static char array3[LARGE_SIZE * 2];

/* Matrix for multiplication-like operations */
#define MATRIX_SIZE 128
static int matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static int matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static int matrix_c[MATRIX_SIZE][MATRIX_SIZE];

/* Function with varying access patterns to encourage cache-aware optimizations */
double process_arrays(int limit1, int limit2, int step) {
    double sum = 0.0;
    int i, j, k;
    
    /* Pattern 1: Matrix multiplication-like triple nested loop */
    for (i = 0; i < limit1 && i < MATRIX_SIZE; i++) {
        for (j = 0; j < limit2 && j < MATRIX_SIZE; j++) {
            int temp = 0;
            for (k = 0; k < MATRIX_SIZE; k++) {
                temp += matrix_a[i][k] * matrix_b[k][j];
            }
            matrix_c[i][j] = temp;
            sum += temp;
        }
    }
    
    /* Pattern 2: Non-unit stride access */
    for (i = 0; i < LARGE_SIZE; i += step) {
        array2[i] = array1[i] * 1.5;
        sum += array2[i];
    }
    
    /* Pattern 3: Copy with potential cache line aliasing */
    for (i = 0; i < LARGE_SIZE * 2; i++) {
        array3[i] = (char)(array1[i % LARGE_SIZE] & 0xFF);
        sum += array3[i];
    }
    
    /* Pattern 4: Blocked matrix transposition (cache-aware pattern) */
    int block_size = 32; /* Common cache line aware block size */
    for (i = 0; i < MATRIX_SIZE; i += block_size) {
        for (j = 0; j < MATRIX_SIZE; j += block_size) {
            for (int ii = i; ii < i + block_size && ii < MATRIX_SIZE; ii++) {
                for (int jj = j; jj < j + block_size && jj < MATRIX_SIZE; jj++) {
                    int temp = matrix_b[jj][ii];
                    matrix_b[jj][ii] = matrix_a[ii][jj];
                    matrix_a[ii][jj] = temp;
                    sum += temp;
                }
            }
        }
    }
    
    return sum;
}

/* Another function with different memory access pattern */
void init_arrays(void) {
    int i, j;
    
    /* Initialize with pseudo-random but deterministic values */
    for (i = 0; i < LARGE_SIZE; i++) {
        array1[i] = (i * 37) % 100;
        array2[i] = (i * 51) % 200 * 0.5;
    }
    
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = (i * MATRIX_SIZE + j) % 100;
            matrix_b[i][j] = (j * MATRIX_SIZE + i) % 100;
        }
    }
}

/* Conditional compilation for different x86 architectures */
#ifdef __x86_64__
void x86_64_specific_code(void) {
    /* Code that might benefit from specific x86-64 cache optimizations */
    volatile long long large_array[5000];
    for (int i = 0; i < 5000; i++) {
        large_array[i] = i * 2;
    }
    
    /* Use built-in CPU detection if available */
    #ifdef __GNUC__
    /* This may prompt CPU feature detection in the driver */
    asm volatile("" ::: "memory");
    #endif
}
#endif

#ifdef __i386__
void i386_specific_code(void) {
    /* 32-bit specific memory access patterns */
    volatile int small_matrix[32][32];
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            small_matrix[i][j] = i + j;
        }
    }
}
#endif

int main(int argc, char *argv[]) {
    double total_sum = 0.0;
    int iterations = 10;
    
    /* Use command line args to vary behavior */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 10;
    }
    
    if (argc > 2) {
        stride = atoi(argv[2]);
        if (stride < 1) stride = 8;
    }
    
    /* Initialize data */
    init_arrays();
    
    /* Architecture-specific code paths */
    #ifdef __x86_64__
    x86_64_specific_code();
    printf("Running x86_64 optimized path\n");
    #elif defined(__i386__)
    i386_specific_code();
    printf("Running i386 optimized path\n");
    #endif
    
    /* Main computational loop */
    for (int iter = 0; iter < iterations; iter++) {
        /* Vary parameters slightly each iteration */
        int current_limit = outer_limit + (iter % 5);
        int current_inner = inner_limit + (iter % 3);
        
        total_sum += process_arrays(current_limit, current_inner, stride + (iter % 4));
        
        /* Modify data between iterations */
        for (int i = 0; i < LARGE_SIZE; i += 100) {
            array1[i] += iter;
        }
    }
    
    printf("Result: %f\n", total_sum);
    
    /* Additional loop with prefetch-friendly pattern */
    {
        int sum_int = 0;
        /* Pattern that might trigger prefetch optimizations */
        for (int i = 0; i < LARGE_SIZE - 4; i += 4) {
            sum_int += array1[i] + array1[i+1] + array1[i+2] + array1[i+3];
        }
        printf("Integer sum: %d\n", sum_int);
    }
    
    return 0;
}
