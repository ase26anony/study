/* cache_detection_test.c
 * Designed to trigger GCC's x86 cache detection logic for uncovered cache descriptor values
 * Compile with various x86-specific tuning flags to exercise the driver's cache detection
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

/* Matrix multiplication style computation */
void matrix_style_computation(int n) {
    volatile int limit = n;
    int i, j, k;
    double sum;
    
    /* Triple nested loop - typical for cache optimization */
    for (i = 0; i < limit; i++) {
        for (j = 0; j < limit; j++) {
            sum = 0.0;
            for (k = 0; k < limit; k++) {
                /* Access with different strides */
                sum += darray1[i * stride + k] * darray2[k * stride + j];
            }
            darray1[i * stride + j] = sum;
        }
    }
}

/* Non-unit stride access pattern */
void stride_access_computation(int n, int stride_val) {
    volatile int limit = n;
    int i;
    long long acc = 0;
    
    /* Access every stride_val-th element */
    for (i = 0; i < limit; i += stride_val) {
        array1[i] = array2[i] * 3 + i;
        acc += array1[i];
    }
    
    /* Prevent dead code elimination */
    if (acc == 0) {
        printf("Impossible\n");
    }
}

/* Cache line aliasing test */
void cache_line_copy(int n) {
    volatile int limit = n;
    int i, j;
    
    /* Copy with potential cache line conflicts */
    for (i = 0; i < limit; i++) {
        for (j = 0; j < 64; j++) {  /* Typical cache line size */
            int idx = (i * 64 + j) % LARGE_SIZE;
            carray2[idx] = carray1[idx] + 1;
        }
    }
}

/* Mixed data type operations */
void mixed_operations(int n) {
    volatile int limit = n;
    int i;
    
    for (i = 0; i < limit; i++) {
        /* Mix operations on different data types */
        array1[i] = (int)(darray1[i] * 100.0);
        darray2[i] = (double)array2[i] / 3.14159;
        carray1[i] = (char)(array1[i] % 256);
    }
}

/* Initialize arrays with pseudo-random data */
void init_arrays(void) {
    int i;
    for (i = 0; i < LARGE_SIZE; i++) {
        array1[i] = i % 100;
        array2[i] = (i * 3) % 100;
        darray1[i] = (double)i / 10.0;
        darray2[i] = (double)(i * 2) / 10.0;
        carray1[i] = (char)(i % 128);
        carray2[i] = 0;
    }
}

int main(int argc, char *argv[]) {
    int iterations = 10;
    int i;
    clock_t start, end;
    double cpu_time_used;
    
    /* Initialize with some data */
    init_arrays();
    
    /* Use command line args to prevent constant propagation */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 10;
    }
    
    printf("Starting cache-sensitive computations...\n");
    start = clock();
    
    /* Execute multiple computation patterns */
    for (i = 0; i < iterations; i++) {
        /* Vary parameters to force different optimization decisions */
        int size = 100 + (i % 50) * 20;
        int current_stride = 8 + (i % 8);
        
        matrix_style_computation(size / 10);
        stride_access_computation(size, current_stride);
        cache_line_copy(size / 2);
        mixed_operations(size);
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Computations completed in %.3f seconds\n", cpu_time_used);
    
    /* Print a result to prevent dead code elimination */
    volatile int checksum = 0;
    for (i = 0; i < 100; i++) {
        checksum += array1[i] + (int)darray1[i] + carray1[i];
    }
    printf("Checksum: %d\n", checksum);
    
    /* Conditional compilation for different x86 architectures */
#ifdef __x86_64__
    printf("x86-64 architecture detected\n");
    /* Code that might benefit from specific cache optimizations */
    #if defined(__AVX__)
        printf("AVX instructions available\n");
    #endif
    #if defined(__SSE4_2__)
        printf("SSE4.2 instructions available\n");
    #endif
#endif
    
#ifdef __i386__
    printf("i386 architecture detected\n");
    /* Different code path for 32-bit */
#endif
    
    return 0;
}
