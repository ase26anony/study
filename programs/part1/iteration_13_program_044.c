/* cache_detection_test.c
 * Designed to trigger CPU cache detection logic in GCC driver
 * Compile with various x86-specific flags to exercise cache descriptor cases
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
#define MEDIUM_SIZE 5000
#define SMALL_SIZE 2000

/* Different data types to vary access patterns */
static int int_array1[LARGE_SIZE];
static int int_array2[LARGE_SIZE];
static double double_array1[MEDIUM_SIZE];
static double double_array2[MEDIUM_SIZE];
static char char_array[LARGE_SIZE * 4]; /* Larger byte array */

/* Function prototypes to force different optimization decisions */
void matrix_multiply_style(int n);
void stride_access_pattern(int limit, int step);
void cache_line_copy(int size);
void mixed_operations(void);

/* Matrix multiplication style triple nested loop */
void matrix_multiply_style(int n) {
    volatile int i, j, k;
    double sum;
    
    /* Initialize arrays */
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            double_array1[i * n + j] = (double)(i + j) * 0.5;
            double_array2[i * n + j] = (double)(i * j) * 0.3;
        }
    }
    
    /* Triple nested loop - classic cache challenge */
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            sum = 0.0;
            for (k = 0; k < n; k++) {
                sum += double_array1[i * n + k] * double_array2[k * n + j];
            }
            /* Store result back */
            if (i == j) {
                int_array1[i] = (int)sum;
            }
        }
    }
}

/* Non-unit stride access pattern */
void stride_access_pattern(int limit, int step) {
    volatile int i, j;
    long long accumulator = 0;
    
    for (i = 0; i < limit; i++) {
        /* Access every 'step'th element */
        for (j = i; j < LARGE_SIZE; j += step) {
            int_array1[j] = int_array1[j] * 3 + int_array2[j];
            accumulator += int_array1[j];
        }
        
        /* Also access char array with different stride */
        for (j = 0; j < SMALL_SIZE; j += (step / 2)) {
            char_array[j] = (char)((int_array1[j] + j) & 0xFF);
        }
    }
    
    /* Use accumulator to prevent dead code elimination */
    if (accumulator > 1000000) {
        printf("Stride pattern accumulator: %lld\n", accumulator % 1000);
    }
}

/* Copy between arrays with potential cache line issues */
void cache_line_copy(int size) {
    volatile int i, j;
    
    /* Initialize source array with pattern */
    for (i = 0; i < size; i++) {
        int_array2[i] = i * 3 + 7;
    }
    
    /* Copy with potential aliasing */
    for (i = 0; i < size - 64; i += 64) {
        for (j = 0; j < 64; j++) {
            int_array1[i + j] = int_array2[i + j] + int_array1[(i + j + 32) % size];
        }
    }
    
    /* Reverse copy */
    for (i = size - 1; i >= 64; i -= 64) {
        for (j = 0; j < 64; j++) {
            int_array2[i - j] = int_array1[i - j] - int_array2[(i - j + 16) % size];
        }
    }
}

/* Mixed operations on different data types */
void mixed_operations(void) {
    volatile int i;
    double dsum = 0.0;
    int isum = 0;
    
    for (i = 0; i < MEDIUM_SIZE; i++) {
        /* Mix double and int operations */
        double_array1[i] = double_array1[i] * 1.1 + (double)int_array1[i % SMALL_SIZE];
        dsum += double_array1[i];
        
        if (i % 4 == 0) {
            int_array1[i / 4] = (int)(double_array1[i] * 10.0);
            isum += int_array1[i / 4];
        }
        
        /* Access char array occasionally */
        if (i % 8 == 0) {
            char_array[i] = (char)(((int)dsum + isum) & 0xFF);
        }
    }
    
    /* Use results */
    printf("Mixed ops - dsum: %.2f, isum: %d\n", dsum / MEDIUM_SIZE, isum % 1000);
}

int main(int argc, char *argv[]) {
    volatile int iterations = 5;
    volatile int matrix_size = 64; /* Smaller for testing, but still exercises cache */
    int i;
    
    /* Initialize arrays with some data */
    srand(time(NULL));
    for (i = 0; i < LARGE_SIZE; i++) {
        int_array1[i] = rand() % 100;
        int_array2[i] = rand() % 100;
    }
    
    for (i = 0; i < MEDIUM_SIZE; i++) {
        double_array1[i] = (double)rand() / RAND_MAX;
        double_array2[i] = (double)rand() / RAND_MAX;
    }
    
    memset(char_array, 0, sizeof(char_array));
    
    printf("Starting cache-intensive computations...\n");
    
    /* Execute different cache-challenging patterns */
    for (i = 0; i < iterations; i++) {
        matrix_multiply_style(matrix_size);
        stride_access_pattern(outer_limit / 10, stride);
        cache_line_copy(MEDIUM_SIZE);
        mixed_operations();
        
        /* Vary parameters slightly each iteration */
        matrix_size += 2;
        stride = (stride % 32) + 4;
    }
    
    /* Final accumulation and output to prevent optimization */
    long long final_sum = 0;
    for (i = 0; i < LARGE_SIZE; i += 128) {
        final_sum += int_array1[i] + int_array2[i];
    }
    
    printf("Final checksum: %lld\n", final_sum % 1000000);
    
    /* Conditional compilation for different architectures */
#ifdef __x86_64__
    printf("Compiled for x86_64 architecture\n");
    /* Additional x86_64 specific code */
    for (i = 0; i < 100; i++) {
        __asm__ volatile ("nop" : : : "memory");
    }
#endif
    
#ifdef __i386__
    printf("Compiled for i386 architecture\n");
#endif
    
    /* Explicit CPU feature detection if available */
#ifdef __GNUC__
    /* These builtins may trigger CPUID queries */
    __builtin_cpu_init();
    if (__builtin_cpu_supports("sse2")) {
        printf("SSE2 supported\n");
    }
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported\n");
    }
#endif
    
    return 0;
}
