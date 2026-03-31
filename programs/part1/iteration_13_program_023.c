/* cache_detection_test.c
 * 
 * This program is designed to trigger GCC's internal CPU cache detection
 * logic by using computational patterns that benefit from cache-aware
 * optimizations and compiling with x86-specific tuning options.
 * 
 * Compile with various x86 march/mtune options to exercise different
 * cache descriptor cases in driver-i386.cc:
 *   gcc -O3 -march=nocona -ftree-loop-distribution -fprefetch-loop-arrays cache_detection_test.c -o test_nocona
 *   gcc -O2 -march=nehalem -mtune=generic cache_detection_test.c -o test_nehalem
 *   gcc -O1 -march=x86-64 -mtune=core2 -fverbose-asm cache_detection_test.c -o test_core2
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Use volatile to prevent compile-time optimization of loop bounds */
volatile int outer_limit = 1000;
volatile int inner_limit = 500;
volatile int stride_val = 16;

/* Large arrays that exceed typical L1 cache sizes */
#define LARGE_SIZE 10000
static int array1[LARGE_SIZE];
static double array2[LARGE_SIZE];
static char array3[LARGE_SIZE * 2];

/* Heap allocated arrays for additional cache pressure */
int *heap_array1;
double *heap_array2;

/* Matrix multiplication style computation - benefits from cache blocking */
void matrix_style_computation(int n) {
    volatile int i, j, k;
    int sum = 0;
    
    /* Triple nested loop - compiler may apply cache-aware transformations */
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
    
    printf("Matrix computation result: %d\n", sum % 1000);
}

/* Non-unit stride access pattern - tests cache line utilization */
void stride_access_computation(int limit, int stride) {
    volatile int i;
    double acc = 0.0;
    
    /* Access every stride-th element - may trigger prefetch optimizations */
    for (i = 0; i < limit; i += stride) {
        acc += array2[i % LARGE_SIZE] * 1.5;
        array3[i % (LARGE_SIZE * 2)] = (char)(acc * 0.01);
    }
    
    /* Additional loop with reverse stride */
    for (i = limit - 1; i >= 0; i -= stride / 2) {
        acc -= array2[i % LARGE_SIZE] * 0.5;
    }
    
    printf("Stride access result: %.2f\n", acc);
}

/* Array copy with potential cache line aliasing */
void cache_line_copy(int size) {
    volatile int i, j;
    
    /* Copy with potential cache conflicts */
    for (i = 0; i < size; i++) {
        for (j = 0; j < 8; j++) {  /* 8 ints per 64-byte cache line */
            int idx = (i * 8 + j) % LARGE_SIZE;
            heap_array1[idx] = array1[idx] + j;
        }
    }
    
    /* Reverse copy pattern */
    for (i = size - 1; i >= 0; i--) {
        heap_array2[i % LARGE_SIZE] = array2[i % LARGE_SIZE] * 0.25;
    }
}

/* Mixed data type operations to test various cache behaviors */
void mixed_data_computation(int iterations) {
    volatile int i;
    char *char_ptr = array3;
    double *double_ptr = array2;
    int *int_ptr = array1;
    
    for (i = 0; i < iterations; i++) {
        /* Interleave operations on different data types */
        int idx = i % LARGE_SIZE;
        int_ptr[idx] = (int)(double_ptr[idx] * 100.0);
        char_ptr[idx * 2] = (char)(int_ptr[idx] % 256);
        char_ptr[idx * 2 + 1] = (char)((int_ptr[idx] / 256) % 256);
        
        /* Periodic larger stride access */
        if (i % 100 == 0) {
            double_ptr[(i * 7) % LARGE_SIZE] = int_ptr[idx] * 0.01;
        }
    }
}

/* Initialize arrays with pseudo-random but deterministic values */
void initialize_arrays(void) {
    volatile int i;
    for (i = 0; i < LARGE_SIZE; i++) {
        array1[i] = (i * 37) % 7919;  /* Prime number for "random" distribution */
        array2[i] = (i * 0.01) + 0.5;
    }
    for (i = 0; i < LARGE_SIZE * 2; i++) {
        array3[i] = (char)((i * 13) % 256);
    }
}

/* Conditional compilation for different x86 architectures */
#ifdef __x86_64__
void x86_64_specific_workload(void) {
    printf("x86-64 architecture detected\n");
    
    /* Use CPU builtins to explicitly trigger feature detection */
    #ifdef __builtin_cpu_init
    __builtin_cpu_init();
    #endif
    
    /* Additional workload specific to 64-bit */
    volatile long i;
    long long_result = 0;
    for (i = 0; i < outer_limit * 2; i++) {
        long_result += array1[i % LARGE_SIZE] * 3;
    }
    printf("64-bit specific result: %ld\n", long_result);
}
#endif

#ifdef __i386__
void i386_specific_workload(void) {
    printf("i386 architecture detected\n");
    
    /* 32-bit specific patterns */
    volatile int i;
    int result = 0;
    for (i = 0; i < outer_limit; i++) {
        result += array1[i % LARGE_SIZE] - array1[(i + 1) % LARGE_SIZE];
    }
    printf("32-bit specific result: %d\n", result);
}
#endif

int main(int argc, char *argv[]) {
    /* Use command line args to vary computation sizes */
    int compute_size = (argc > 1) ? atoi(argv[1]) : 250;
    if (compute_size < 10) compute_size = 10;
    if (compute_size > 1000) compute_size = 1000;
    
    /* Initialize data */
    initialize_arrays();
    
    /* Allocate heap arrays */
    heap_array1 = (int*)malloc(LARGE_SIZE * sizeof(int));
    heap_array2 = (double*)malloc(LARGE_SIZE * sizeof(double));
    
    if (!heap_array1 || !heap_array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    printf("Starting cache-sensitive computations...\n");
    printf("Compute size: %d\n", compute_size);
    
    /* Execute various computation patterns */
    matrix_style_computation(compute_size / 2);
    stride_access_computation(compute_size * 10, stride_val);
    cache_line_copy(compute_size);
    mixed_data_computation(compute_size * 20);
    
    /* Architecture-specific workloads */
    #ifdef __x86_64__
    x86_64_specific_workload();
    #endif
    
    #ifdef __i386__
    i386_specific_workload();
    #endif
    
    /* Final computation combining results */
    volatile int i;
    double final_result = 0.0;
    for (i = 0; i < compute_size * 5; i++) {
        int idx1 = (i * 3) % LARGE_SIZE;
        int idx2 = (i * 7) % LARGE_SIZE;
        final_result += heap_array1[idx1] * 0.5 + heap_array2[idx2];
        
        /* Occasional larger stride to break locality */
        if (i % 50 == 0) {
            idx1 = (i * 17) % LARGE_SIZE;
            final_result -= array1[idx1] * 0.25;
        }
    }
    
    printf("Final result: %.4f\n", final_result);
    
    /* Cleanup */
    free(heap_array1);
    free(heap_array2);
    
    return 0;
}
