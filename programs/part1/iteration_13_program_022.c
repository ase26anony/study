/* cache_detection_test.c
 * Designed to trigger GCC driver cache detection for x86 cache descriptor values
 * Compile with: gcc -O3 -march=nocona -ftree-loop-distribution -fprefetch-loop-arrays cache_detection_test.c -o test
 * Also try: gcc -O2 -march=nehalem -mtune=generic cache_detection_test.c -o test2
 * And: gcc -O1 -march=x86-64 -mtune=core2 -fverbose-asm cache_detection_test.c -o test3
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force compiler to consider cache characteristics by using volatile inputs */
volatile int outer_limit = 1000;
volatile int inner_limit = 500;
volatile int stride = 16;

/* Large arrays that exceed typical L1 cache sizes */
#define LARGE_SIZE 10000
static int array1[LARGE_SIZE];
static double array2[LARGE_SIZE];
static char array3[LARGE_SIZE * 2];  /* Non-power-of-two size for alignment variations */

/* Heap arrays for dynamic allocation patterns */
int *heap_array1;
int *heap_array2;
double *heap_array3;

/* Matrix multiplication style triple nested loop */
void matrix_style_loop(int n, int m, int p) {
    volatile int i, j, k;
    int result = 0;
    
    /* Triple nested loop - compiler may optimize based on cache size */
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            int temp = 0;
            for (k = 0; k < p; k++) {
                /* Access with different strides and patterns */
                temp += array1[(i * stride + k) % LARGE_SIZE] * 
                        array1[(j * stride + k) % LARGE_SIZE];
            }
            array2[(i * m + j) % LARGE_SIZE] = temp;
            result += temp;
        }
    }
    
    printf("Matrix result: %d\n", result);
}

/* Non-unit stride access pattern */
void stride_access_loop(int limit, int step) {
    volatile int i;
    double sum = 0.0;
    
    /* Access every 'step'th element - tests cache line utilization */
    for (i = 0; i < limit; i += step) {
        array2[i % LARGE_SIZE] = array1[i % LARGE_SIZE] * 1.5;
        sum += array2[i % LARGE_SIZE];
        
        /* Additional access with different stride */
        if (i % (step * 2) == 0) {
            array3[(i * 3) % (LARGE_SIZE * 2)] = (char)(array1[i % LARGE_SIZE] & 0xFF);
        }
    }
    
    printf("Stride sum: %f\n", sum);
}

/* Copy with potential cache line aliasing */
void copy_with_aliasing(int size, int offset) {
    volatile int i;
    
    /* Copy between arrays with offset that might cause cache conflicts */
    for (i = 0; i < size; i++) {
        int src_idx = i % LARGE_SIZE;
        int dst_idx = (i + offset) % LARGE_SIZE;
        heap_array2[dst_idx] = heap_array1[src_idx] + heap_array3[src_idx % (LARGE_SIZE/2)];
    }
    
    /* Verify by computing checksum */
    long checksum = 0;
    for (i = 0; i < 100; i++) {
        checksum += heap_array2[i % LARGE_SIZE];
    }
    printf("Checksum: %ld\n", checksum);
}

/* Mixed data type operations */
void mixed_operations(int iterations) {
    volatile int i, j;
    double accumulator = 0.0;
    
    for (i = 0; i < iterations; i++) {
        /* Mix int and double operations */
        int idx = (i * 7) % LARGE_SIZE;  /* Non-linear access pattern */
        double val = array1[idx] * 0.5;
        
        /* Conditional that prevents simple optimization */
        if (idx % 3 == 0) {
            val += array2[idx] * 2.0;
        } else if (idx % 3 == 1) {
            val -= array3[idx % (LARGE_SIZE * 2)] * 0.1;
        }
        
        array2[idx] = val;
        accumulator += val;
        
        /* Nested short loop */
        for (j = 0; j < 4; j++) {
            array3[(idx + j) % (LARGE_SIZE * 2)] = (char)((int)val + j);
        }
    }
    
    printf("Accumulator: %f\n", accumulator);
}

/* Conditional compilation for different x86 architectures */
#ifdef __x86_64__
void x86_64_specific_code(void) {
    printf("x86-64 architecture detected\n");
    
    /* Code that might benefit from specific cache optimizations */
    volatile int i;
    for (i = 0; i < 1000; i += 64) {  /* Cache line sized steps */
        array1[i % LARGE_SIZE] = i;
    }
}
#endif

#ifdef __i386__
void i386_specific_code(void) {
    printf("i386 architecture detected\n");
    
    /* Different access pattern for 32-bit */
    volatile int i;
    for (i = 0; i < 800; i += 32) {  /* Smaller steps for 32-bit */
        array1[i % LARGE_SIZE] = i * 2;
    }
}
#endif

int main(int argc, char *argv[]) {
    /* Initialize arrays with pseudo-random but deterministic values */
    srand(42);
    for (int i = 0; i < LARGE_SIZE; i++) {
        array1[i] = rand() % 100;
        array2[i] = (double)(rand() % 100) / 10.0;
    }
    for (int i = 0; i < LARGE_SIZE * 2; i++) {
        array3[i] = (char)(rand() % 256);
    }
    
    /* Dynamic allocation - pattern unknown at compile time */
    heap_array1 = (int*)malloc(LARGE_SIZE * sizeof(int));
    heap_array2 = (int*)malloc(LARGE_SIZE * sizeof(int));
    heap_array3 = (double*)malloc(LARGE_SIZE * sizeof(double));
    
    if (!heap_array1 || !heap_array2 || !heap_array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize heap arrays */
    for (int i = 0; i < LARGE_SIZE; i++) {
        heap_array1[i] = rand() % 200;
        heap_array3[i] = (double)(rand() % 200) / 20.0;
    }
    
    printf("Starting cache-sensitive computations...\n");
    
    /* Execute various loop patterns that should trigger cache-aware optimizations */
    matrix_style_loop(outer_limit % 100, inner_limit % 50, stride % 20);
    stride_access_loop(outer_limit * 2, stride);
    copy_with_aliasing(inner_limit * 10, stride * 4);
    mixed_operations(outer_limit % 200);
    
    /* Architecture-specific code paths */
#ifdef __x86_64__
    x86_64_specific_code();
#endif
    
#ifdef __i386__
    i386_specific_code();
#endif
    
    /* Explicit CPU feature detection if available */
#ifdef __GNUC__
    /* These builtins might trigger CPUID queries */
    __builtin_cpu_init();
    if (__builtin_cpu_supports("sse2")) {
        printf("SSE2 supported\n");
    }
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported\n");
    }
#endif
    
    /* Final computation using all arrays */
    double final_result = 0.0;
    for (volatile int i = 0; i < 500; i++) {
        int idx = (i * 13) % LARGE_SIZE;  /* Prime stride */
        final_result += array1[idx] + array2[idx] + heap_array1[idx] + heap_array3[idx];
    }
    
    printf("Final result: %f\n", final_result);
    
    /* Cleanup */
    free(heap_array1);
    free(heap_array2);
    free(heap_array3);
    
    return 0;
}
