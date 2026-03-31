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

/* Force compiler to consider cache characteristics by using
 * arrays that exceed typical L1/L2 cache sizes */
#define LARGE_SIZE 10000
#define MEDIUM_SIZE 5000
#define SMALL_SIZE 1000

/* Use volatile to prevent compile-time optimization */
volatile int outer_limit = LARGE_SIZE;
volatile int inner_limit = MEDIUM_SIZE;

/* Different array types to test various access patterns */
static int matrix_a[LARGE_SIZE][MEDIUM_SIZE];
static double matrix_b[MEDIUM_SIZE][SMALL_SIZE];
static char buffer[LARGE_SIZE * 4];
static long results[SMALL_SIZE];

/* Matrix multiplication-like triple nested loop
 * This pattern heavily depends on cache blocking optimizations */
void matrix_operations(int n, int m, int p) {
    volatile int i, j, k;
    double sum;
    
    /* Initialize matrices */
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            matrix_a[i][j] = (i + j) % 256;
        }
    }
    
    for (i = 0; i < m; i++) {
        for (j = 0; j < p; j++) {
            matrix_b[i][j] = (double)((i * j) % 256) / 256.0;
        }
    }
    
    /* Triple nested loop - compiler may apply cache-aware optimizations */
    for (i = 0; i < n; i++) {
        for (j = 0; j < p; j++) {
            sum = 0.0;
            for (k = 0; k < m; k++) {
                sum += matrix_a[i][k] * matrix_b[k][j];
            }
            /* Store result in a way that prevents dead code elimination */
            results[j % SMALL_SIZE] += (long)(sum * 1000);
        }
    }
}

/* Non-unit stride access pattern
 * Tests cache line utilization and prefetching */
void stride_access(int size, int stride) {
    volatile int i;
    long accumulator = 0;
    
    for (i = 0; i < size; i += stride) {
        buffer[i] = (char)(i % 256);
        accumulator += buffer[i];
    }
    
    /* Use result to prevent optimization */
    results[0] += accumulator;
}

/* Array copy with potential cache line aliasing */
void cache_line_copy(int size) {
    volatile int i;
    char temp_buffer[LARGE_SIZE * 4];
    
    /* Copy with different alignments to test cache behavior */
    for (i = 0; i < size; i++) {
        temp_buffer[i] = buffer[(i + 64) % size];  /* Offset by typical cache line */
    }
    
    /* Use the copied data */
    for (i = 0; i < size; i += 128) {
        results[i % SMALL_SIZE] += temp_buffer[i];
    }
}

/* Mixed data type operations to test various access sizes */
void mixed_operations(int iterations) {
    volatile int i, j;
    int int_array[MEDIUM_SIZE];
    double double_array[MEDIUM_SIZE];
    
    for (i = 0; i < iterations; i++) {
        /* Integer operations */
        for (j = 0; j < MEDIUM_SIZE; j++) {
            int_array[j] = (j * i) % 1024;
        }
        
        /* Floating point operations */
        for (j = 0; j < MEDIUM_SIZE; j++) {
            double_array[j] = int_array[j] * 0.5;
        }
        
        /* Combined access pattern */
        for (j = 0; j < MEDIUM_SIZE - 1; j++) {
            results[j % SMALL_SIZE] += (long)(double_array[j] + int_array[j+1]);
        }
    }
}

int main(int argc, char *argv[]) {
    volatile int test_size, stride;
    long total_result = 0;
    int i;
    
    /* Initialize random seed */
    srand(time(NULL));
    
    /* Use command line args to prevent constant propagation */
    test_size = (argc > 1) ? atoi(argv[1]) : LARGE_SIZE;
    stride = (argc > 2) ? atoi(argv[2]) : 13;  /* Prime number for non-uniform stride */
    
    printf("Starting cache-sensitive operations...\n");
    printf("Test size: %d, Stride: %d\n", test_size, stride);
    
    /* Execute various cache-sensitive patterns */
    
    /* Pattern 1: Matrix operations (triple nested loops) */
    matrix_operations(test_size / 10, MEDIUM_SIZE, SMALL_SIZE);
    
    /* Pattern 2: Non-unit stride access */
    stride_access(test_size, stride);
    
    /* Pattern 3: Cache line copying */
    cache_line_copy(test_size);
    
    /* Pattern 4: Mixed data type operations */
    mixed_operations(10);
    
    /* Additional architecture-specific patterns */
#ifdef __x86_64__
    /* x86-64 specific optimizations */
    printf("x86-64 architecture detected\n");
    
    /* Use different patterns for different march scenarios */
#if defined(__AVX__) || defined(__SSE4_2__)
    /* For newer architectures with specific cache descriptors */
    for (i = 0; i < test_size; i += 64) {  /* Cache line sized accesses */
        buffer[i] = buffer[i] ^ 0x55;  /* Simple transformation */
    }
#endif
    
#elif defined(__i386__)
    /* 32-bit x86 specific patterns */
    printf("i386 architecture detected\n");
    
    /* Different access pattern for 32-bit */
    for (i = 0; i < test_size && i < LARGE_SIZE; i += 32) {
        buffer[i] = buffer[i] + 1;
    }
#endif
    
    /* Calculate final result to ensure all computations are used */
    for (i = 0; i < SMALL_SIZE; i++) {
        total_result += results[i];
    }
    
    printf("Total result: %ld\n", total_result);
    printf("Operations completed.\n");
    
    /* Explicit CPU feature detection (optional) */
#ifdef __GNUC__
    /* This may prompt CPUID queries */
    __builtin_cpu_init();
    
    /* Check for specific features that might influence cache detection */
    if (__builtin_cpu_supports("sse2")) {
        printf("SSE2 supported\n");
    }
    
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported\n");
    }
#endif
    
    return (int)(total_result % 1000);
}
