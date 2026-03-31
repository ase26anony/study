/* 
 * cache_detection_test.c
 * 
 * This program is designed to trigger GCC's internal CPU cache detection
 * logic by using computational patterns that benefit from cache-aware
 * optimizations and compiling with x86-specific tuning options.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Use volatile to prevent compile-time optimization of loop bounds */
volatile int outer_limit = 1000;
volatile int inner_limit = 1000;
volatile int stride = 16;

/* Large arrays that exceed typical L1 cache sizes */
#define LARGE_SIZE 10000
#define MEDIUM_SIZE 5000
#define SMALL_SIZE 1000

static int array1[LARGE_SIZE];
static double array2[LARGE_SIZE];
static char array3[LARGE_SIZE * 4];  /* Larger char array for different access patterns */
static int matrix_a[MEDIUM_SIZE][MEDIUM_SIZE];
static int matrix_b[MEDIUM_SIZE][MEDIUM_SIZE];
static int matrix_c[MEDIUM_SIZE][MEDIUM_SIZE];

/* Function prototypes to force different optimization considerations */
void matrix_multiply(int size);
void stride_access_pattern(void);
void cache_line_test(void);
void mixed_data_type_operations(void);

int main(int argc, char *argv[]) {
    int i, j, k;
    double result = 0.0;
    clock_t start, end;
    
    /* Initialize arrays with pseudo-random but deterministic values */
    srand(42);
    for (i = 0; i < LARGE_SIZE; i++) {
        array1[i] = rand() % 100;
        array2[i] = (double)(rand() % 100) / 3.14;
        array3[i] = (char)(rand() % 256);
    }
    
    /* Initialize matrices */
    for (i = 0; i < MEDIUM_SIZE; i++) {
        for (j = 0; j < MEDIUM_SIZE; j++) {
            matrix_a[i][j] = rand() % 10;
            matrix_b[i][j] = rand() % 10;
            matrix_c[i][j] = 0;
        }
    }
    
    printf("Starting cache-sensitive computations...\n");
    
    /* Pattern 1: Matrix multiplication (triple nested loop) */
    start = clock();
    matrix_multiply(SMALL_SIZE);
    end = clock();
    printf("Matrix multiply (size %d): %f seconds\n", 
           SMALL_SIZE, (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Pattern 2: Non-unit stride access */
    start = clock();
    stride_access_pattern();
    end = clock();
    printf("Stride access pattern: %f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Pattern 3: Mixed data type operations */
    start = clock();
    mixed_data_type_operations();
    end = clock();
    printf("Mixed data type operations: %f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Pattern 4: Cache line aliasing test */
    start = clock();
    cache_line_test();
    end = clock();
    printf("Cache line test: %f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Final computation that uses all results to prevent dead code elimination */
    for (i = 0; i < LARGE_SIZE; i += stride) {
        result += array1[i] * array2[i] + array3[i];
    }
    
    printf("Final accumulated result: %f\n", result);
    
    /* 
     * Architecture-specific code blocks to encourage compilation 
     * with different -march and -mtune options
     */
#ifdef __x86_64__
    /* Code that might benefit from specific x86-64 optimizations */
    __asm__ volatile ("# x86_64 specific section");
    
    /* Try to use CPUID-related builtins if available */
    #ifdef __GNUC__
    /* This may prompt CPU feature detection */
    if (__builtin_cpu_supports("sse2")) {
        printf("SSE2 supported\n");
    }
    #endif
    
#elif defined(__i386__)
    /* Code for 32-bit x86 */
    __asm__ volatile ("# i386 specific section");
#endif
    
    /* Additional architecture hints */
#if defined(__tune_core2__) || defined(__tune_nocona__) || defined(__tune_nehalem__)
    /* This section is optimized for specific microarchitectures */
    printf("Compiled with specific x86 tuning\n");
#endif
    
    return 0;
}

/* 
 * Matrix multiplication - classic triple loop pattern
 * Compiler may apply cache blocking/tiling optimizations
 */
void matrix_multiply(int size) {
    int i, j, k;
    volatile int n = size;  /* Prevent constant propagation */
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            int sum = 0;
            for (k = 0; k < n; k++) {
                /* Access with different strides in different matrices */
                sum += matrix_a[i][k] * matrix_b[k][j];
            }
            matrix_c[i][j] = sum;
        }
    }
}

/* 
 * Access pattern with non-unit stride
 * Tests compiler's ability to optimize for cache line size
 */
void stride_access_pattern(void) {
    int i;
    volatile int limit = outer_limit * 10;
    volatile int s = stride;
    double sum = 0.0;
    
    /* Access every stride-th element */
    for (i = 0; i < limit; i += s) {
        sum += array2[i % LARGE_SIZE];
    }
    
    /* Reverse direction access */
    for (i = limit - 1; i >= 0; i -= s) {
        sum += array1[i % LARGE_SIZE];
    }
    
    /* Prevent dead code elimination */
    array1[0] = (int)sum;
}

/* 
 * Operations designed to test cache line boundaries
 * and potential aliasing issues
 */
void cache_line_test(void) {
    int i, j;
    volatile int iterations = inner_limit;
    
    /* Copy with potential cache line conflicts */
    for (i = 0; i < iterations; i++) {
        int idx = (i * 17) % LARGE_SIZE;  /* Non-linear access pattern */
        array3[idx] = (char)(array1[idx] & 0xFF);
    }
    
    /* Block copy pattern that might benefit from prefetching */
    for (i = 0; i < LARGE_SIZE - 64; i += 64) {
        for (j = 0; j < 64; j++) {
            array3[i + j] = array3[i + j] ^ (char)j;  /* Simple transformation */
        }
    }
}

/* 
 * Mixed data type operations to test different access sizes
 * and alignment considerations
 */
void mixed_data_type_operations(void) {
    int i;
    volatile int limit = outer_limit * 5;
    double acc_double = 0.0;
    int acc_int = 0;
    
    for (i = 0; i < limit; i++) {
        int idx = i % LARGE_SIZE;
        
        /* Mix operations on different data types */
        acc_double += array2[idx];
        acc_int += array1[idx];
        
        /* Conditional that depends on both */
        if (acc_int % 100 == 0) {
            array2[idx] = acc_double / (acc_int + 1);
        }
        
        /* Char array access with different stride */
        array3[(i * 3) % (LARGE_SIZE * 4)] = (char)(acc_int & 0xFF);
    }
    
    /* Cross-store pattern */
    for (i = 0; i < LARGE_SIZE && i < limit; i++) {
        array1[i] = (int)array2[i] + array3[i * 4];
    }
}
