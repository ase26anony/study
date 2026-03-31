/* test_cache_detection.c
 * Designed to trigger GCC driver's CPU cache detection logic
 * Compile with various x86-specific flags to exercise cache descriptor cases
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Volatile variables to prevent compile-time optimization */
volatile int outer_limit = 1000;
volatile int inner_limit = 500;
volatile int stride_val = 16;

/* Large arrays that exceed typical L1 cache sizes */
#define LARGE_SIZE 10000
static int array1[LARGE_SIZE];
static double array2[LARGE_SIZE];
static char array3[LARGE_SIZE * 2];

/* Heap allocated arrays for dynamic access patterns */
int *heap_array1;
double *heap_array2;

/* Matrix multiplication style computation */
void matrix_style_computation(int n) {
    volatile int i, j, k;
    int sum = 0;
    
    /* Triple nested loop - common pattern for cache optimization */
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            int temp = 0;
            for (k = 0; k < n; k++) {
                /* Access with different strides to test various cache behaviors */
                temp += array1[(i * n + k) % LARGE_SIZE] * 
                        array1[(k * n + j) % LARGE_SIZE];
            }
            array2[(i * n + j) % LARGE_SIZE] = temp;
            sum += temp;
        }
    }
    
    printf("Matrix computation result: %d\n", sum);
}

/* Non-unit stride access pattern */
void stride_access_computation(int limit, int stride) {
    volatile int i;
    double acc = 0.0;
    
    /* Access every Nth element - tests cache line utilization */
    for (i = 0; i < limit; i += stride) {
        array2[i % LARGE_SIZE] = array1[i % LARGE_SIZE] * 1.5;
        acc += array2[i % LARGE_SIZE];
        
        /* Additional computation to make optimization worthwhile */
        if (i % 64 == 0) {
            array3[i % (LARGE_SIZE * 2)] = (char)(acc * 0.01);
        }
    }
    
    printf("Stride access result: %f\n", acc);
}

/* Cache line aliasing test */
void cache_line_aliasing_test(int size) {
    volatile int i;
    int *src = heap_array1;
    double *dst = heap_array2;
    
    /* Copy between differently typed arrays with potential aliasing */
    for (i = 0; i < size; i++) {
        dst[i] = (double)src[i] * 0.5;
        
        /* Interleave with char array access */
        array3[i % (LARGE_SIZE * 2)] = (char)(dst[i] * 100);
        
        /* Additional computation to prevent dead code elimination */
        if (i % 128 == 0) {
            src[i] = (int)(dst[i] * 2.0);
        }
    }
    
    /* Verify some results */
    double check_sum = 0.0;
    for (i = 0; i < 100; i++) {
        check_sum += dst[i * 10];
    }
    printf("Aliasing test check sum: %f\n", check_sum);
}

/* Mixed data type computation */
void mixed_type_computation(int iterations) {
    volatile int i, j;
    long total = 0;
    
    for (i = 0; i < iterations; i++) {
        /* Mix int, double, and char operations */
        int idx = i % LARGE_SIZE;
        double temp = array2[idx] * 2.0;
        array1[idx] = (int)temp;
        array3[i % (LARGE_SIZE * 2)] = (char)(temp * 0.1);
        
        /* Nested loop with varying access patterns */
        for (j = 0; j < 8; j++) {
            total += array1[(idx + j) % LARGE_SIZE];
            array2[(idx + j) % LARGE_SIZE] += 0.01;
        }
    }
    
    printf("Mixed type total: %ld\n", total);
}

/* Conditional compilation for different x86 architectures */
#ifdef __x86_64__
void x86_64_specific_code(void) {
    printf("x86-64 architecture detected\n");
    
    /* Code that might benefit from specific cache optimizations */
    volatile int i;
    double sum = 0.0;
    
    /* Large stride access to test L2 cache behavior */
    for (i = 0; i < LARGE_SIZE * 4; i += 256) {
        int idx = i % LARGE_SIZE;
        sum += array1[idx] * array2[idx];
        
        /* Access pattern that might benefit from prefetching */
        if (i % 512 == 0) {
            array3[(i / 2) % (LARGE_SIZE * 2)] = (char)(sum * 0.001);
        }
    }
    
    printf("x86-64 specific sum: %f\n", sum);
}
#endif

#ifdef __i386__
void i386_specific_code(void) {
    printf("i386 architecture detected\n");
    
    /* Different access pattern for 32-bit */
    volatile int i;
    int total = 0;
    
    for (i = 0; i < LARGE_SIZE; i += 4) {
        total += array1[i] + array1[i + 1] + array1[i + 2] + array1[i + 3];
        array2[i / 4] = total * 0.25;
    }
    
    printf("i386 specific total: %d\n", total);
}
#endif

int main(int argc, char *argv[]) {
    volatile int i;
    
    /* Initialize arrays with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < LARGE_SIZE; i++) {
        array1[i] = rand() % 100;
        array2[i] = (double)(rand() % 100) / 10.0;
    }
    
    for (i = 0; i < LARGE_SIZE * 2; i++) {
        array3[i] = (char)(rand() % 256);
    }
    
    /* Allocate heap arrays */
    heap_array1 = (int*)malloc(LARGE_SIZE * sizeof(int));
    heap_array2 = (double*)malloc(LARGE_SIZE * sizeof(double));
    
    if (!heap_array1 || !heap_array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize heap arrays */
    for (i = 0; i < LARGE_SIZE; i++) {
        heap_array1[i] = rand() % 200;
        heap_array2[i] = (double)(rand() % 200) / 20.0;
    }
    
    printf("Starting cache-sensitive computations...\n");
    
    /* Execute various computation patterns that benefit from cache optimization */
    matrix_style_computation(outer_limit % 100);
    stride_access_computation(inner_limit * 10, stride_val);
    cache_line_aliasing_test(LARGE_SIZE / 2);
    mixed_type_computation(outer_limit * 5);
    
    /* Architecture-specific code paths */
#ifdef __x86_64__
    x86_64_specific_code();
#endif
    
#ifdef __i386__
    i386_specific_code();
#endif
    
    /* Additional computation to ensure all arrays are used */
    double final_sum = 0.0;
    for (i = 0; i < 1000; i++) {
        int idx = (i * 7) % LARGE_SIZE;
        final_sum += array1[idx] + array2[idx] + heap_array1[idx] + heap_array2[idx];
    }
    
    printf("Final verification sum: %f\n", final_sum);
    
    /* Cleanup */
    free(heap_array1);
    free(heap_array2);
    
    return 0;
}
