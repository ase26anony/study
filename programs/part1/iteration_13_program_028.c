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
#define CACHE_LINE 64
#define L1_SIZE 32768
#define L2_SIZE 262144

/* Volatile variables to prevent compile-time optimization */
volatile int outer_limit = 1000;
volatile int inner_limit = 500;
volatile int stride = 16;

/* Large arrays that exceed typical cache sizes */
static int matrix_a[2048][2048];  /* ~16MB */
static int matrix_b[2048][2048];
static int matrix_c[2048][2048];
static double dbl_array[100000];  /* ~800KB */
static char char_array[500000];   /* ~500KB */

/* Matrix multiplication - classic cache-sensitive operation */
void matrix_multiply(int n) {
    volatile int i, j, k;
    int sum;
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            sum = 0;
            for (k = 0; k < n; k++) {
                /* Access patterns that benefit from cache line awareness */
                sum += matrix_a[i][k] * matrix_b[k][j];
            }
            matrix_c[i][j] = sum;
        }
    }
}

/* Non-unit stride access pattern */
void stride_access(int size, int stride_val) {
    volatile int i;
    int acc = 0;
    
    for (i = 0; i < size; i += stride_val) {
        /* Access every Nth element - tests cache line utilization */
        acc += dbl_array[i] * 2;
        char_array[i % 500000] = (char)(acc & 0xFF);
    }
    
    /* Use result to prevent dead code elimination */
    if (acc > 1000000) {
        printf("Stride result: %d\n", acc);
    }
}

/* Cache line aliasing test */
void cache_line_copy(int size) {
    volatile int i;
    int *src = (int*)malloc(size * sizeof(int));
    int *dst = (int*)malloc(size * sizeof(int));
    
    if (!src || !dst) return;
    
    /* Initialize with pattern */
    for (i = 0; i < size; i++) {
        src[i] = i * 3;
    }
    
    /* Copy with potential cache line conflicts */
    for (i = 0; i < size; i += CACHE_LINE/sizeof(int)) {
        dst[i] = src[i] + src[(i + 8) % size];
    }
    
    /* Verify copy */
    int check = 0;
    for (i = 0; i < size; i += 64) {
        check += dst[i];
    }
    
    printf("Cache copy check: %d\n", check);
    
    free(src);
    free(dst);
}

/* Mixed data type operations */
void mixed_operations(int iterations) {
    volatile int i, j;
    double sum_dbl = 0.0;
    int sum_int = 0;
    
    for (i = 0; i < iterations; i++) {
        for (j = 0; j < 100; j++) {
            /* Mix operations on different data types */
            sum_dbl += dbl_array[(i * 17 + j) % 100000] * 1.5;
            sum_int += matrix_a[i % 64][j % 64] * 2;
            char_array[(i * 23 + j) % 500000] = (char)((sum_int + (int)sum_dbl) & 0xFF);
        }
        
        /* Conditional to prevent loop unrolling from eliminating cache needs */
        if (i % 100 == 0) {
            stride_access(10000, stride);
        }
    }
    
    printf("Mixed ops result: %d, %.2f\n", sum_int, sum_dbl);
}

/* Explicit CPU feature detection (if available) */
#ifdef __x86_64__
void check_cpu_features() {
    /* These builtins may trigger CPUID queries */
    #ifdef __builtin_cpu_init
    __builtin_cpu_init();
    #endif
    
    #ifdef __builtin_cpu_supports
    if (__builtin_cpu_supports("sse2")) {
        printf("SSE2 supported\n");
    }
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported\n");
    }
    #endif
}
#endif

int main(int argc, char **argv) {
    int i, j;
    clock_t start, end;
    
    printf("Cache detection test program\n");
    
    /* Initialize arrays with non-zero values */
    srand(time(NULL));
    for (i = 0; i < 2048; i++) {
        for (j = 0; j < 2048; j++) {
            matrix_a[i][j] = rand() % 100;
            matrix_b[i][j] = rand() % 100;
        }
    }
    
    for (i = 0; i < 100000; i++) {
        dbl_array[i] = (double)(rand() % 1000) / 10.0;
    }
    
    for (i = 0; i < 500000; i++) {
        char_array[i] = (char)(rand() % 256);
    }
    
    /* Conditional compilation for different architectures */
    #ifdef __x86_64__
    check_cpu_features();
    
    /* Different code paths for different -march targets */
    #ifdef __tune_core2__
    printf("Compiled for Core2 microarchitecture\n");
    outer_limit = 800;
    #elif defined(__tune_nehalem__)
    printf("Compiled for Nehalem microarchitecture\n");
    outer_limit = 1200;
    #elif defined(__tune_nocona__)
    printf("Compiled for Nocona microarchitecture\n");
    outer_limit = 600;
    #else
    printf("Generic x86-64 compilation\n");
    #endif
    #endif
    
    /* Execute cache-sensitive operations */
    start = clock();
    
    /* Matrix multiplication - tests L1/L2 cache behavior */
    matrix_multiply(512);
    
    /* Various access patterns */
    stride_access(100000, stride);
    
    /* Cache line operations */
    cache_line_copy(10000);
    
    /* Mixed operations */
    mixed_operations(outer_limit);
    
    /* Additional loop with variable bounds */
    int result = 0;
    for (i = 0; i < inner_limit; i++) {
        for (j = 0; j < 1000; j++) {
            result += matrix_c[i % 512][j % 512] * dbl_array[(i * j) % 100000];
        }
    }
    
    end = clock();
    
    printf("Final result: %d\n", result);
    printf("Execution time: %.2f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    
    return 0;
}
