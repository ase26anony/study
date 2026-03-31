/* cache_detection_test.c
 * 
 * This program is designed to trigger GCC's internal CPU cache detection
 * logic by using computational patterns that benefit from cache-aware
 * optimizations and compiling with specific x86 tuning options.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Use volatile to prevent compile-time optimization of loop bounds */
volatile int outer_limit = 1000;
volatile int inner_limit = 500;
volatile int stride = 16;

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
    volatile int limit = n;
    int i, j, k;
    double sum;
    
    /* Triple nested loop - common pattern for cache optimization */
    for (i = 0; i < limit; i++) {
        for (j = 0; j < limit; j++) {
            sum = 0.0;
            for (k = 0; k < limit; k++) {
                /* Mix int and double operations */
                sum += array2[i * limit + k] * array2[k * limit + j];
            }
            array1[i * limit + j] = (int)sum;
        }
    }
}

/* Non-unit stride access pattern */
void stride_access_computation(int size, int stride_val) {
    volatile int s = stride_val;
    int i;
    double acc = 0.0;
    
    /* Access every Nth element - tests cache line utilization */
    for (i = 0; i < size; i += s) {
        array2[i] = array2[i] * 1.01 + (double)array1[i];
        acc += array2[i];
    }
    
    /* Use the result to prevent dead code elimination */
    if (acc < 0) {
        printf("Unexpected negative accumulation\n");
    }
}

/* Copy with potential cache line aliasing */
void copy_with_aliasing(char *dest, char *src, int size) {
    volatile int s = size;
    int i;
    
    /* Copy with different alignments */
    for (i = 0; i < s; i++) {
        dest[i] = src[i] ^ 0x55;  /* Simple transformation */
    }
}

/* Mixed data type operations */
void mixed_type_operations(int iterations) {
    int i, j;
    double temp;
    
    for (i = 0; i < iterations; i++) {
        for (j = 0; j < LARGE_SIZE; j++) {
            /* Mix operations on different data types */
            temp = (double)array1[j];
            array2[j] = temp * 0.5 + (j % 100);
            array3[j] = (char)((int)array2[j] & 0xFF);
        }
    }
}

/* Initialize arrays with pseudo-random data */
void initialize_arrays(void) {
    int i;
    
    srand(time(NULL));
    
    for (i = 0; i < LARGE_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = (double)(rand() % 1000) / 3.0;
    }
    
    for (i = 0; i < LARGE_SIZE * 2; i++) {
        array3[i] = (char)(rand() % 256);
    }
}

int main(int argc, char *argv[]) {
    int i, result = 0;
    volatile int compute_size = 100;
    
    /* Initialize data */
    initialize_arrays();
    
    /* Allocate heap arrays */
    heap_array1 = (int*)malloc(LARGE_SIZE * sizeof(int));
    heap_array2 = (double*)malloc(LARGE_SIZE * sizeof(double));
    
    if (!heap_array1 || !heap_array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize heap arrays */
    for (i = 0; i < LARGE_SIZE; i++) {
        heap_array1[i] = array1[i];
        heap_array2[i] = array2[i];
    }
    
    /* 
     * Use preprocessor directives to conditionally include code
     * for different x86 architectures
     */
    
#ifdef __x86_64__
    /* Code optimized for x86-64 with various cache descriptor cases */
    printf("x86-64 architecture detected\n");
    
    /* Force cache detection by using CPUID intrinsics if available */
    #ifdef __GNUC__
        /* These builtins may trigger CPU feature detection */
        __builtin_cpu_init();
        
        /* Check for specific features that require cache info */
        if (__builtin_cpu_supports("sse2")) {
            printf("SSE2 supported\n");
        }
        if (__builtin_cpu_supports("avx")) {
            printf("AVX supported\n");
        }
    #endif
    
    /* Perform computations that benefit from cache optimization */
    matrix_style_computation(compute_size);
    
#elif defined(__i386__)
    /* Code for 32-bit x86 */
    printf("i386 architecture detected\n");
    stride_access_computation(LARGE_SIZE, stride);
    
#else
    /* Generic fallback */
    printf("Non-x86 architecture\n");
#endif
    
    /* Always execute these computations */
    copy_with_aliasing((char*)heap_array1, array3, LARGE_SIZE * sizeof(int));
    mixed_type_operations(5);
    
    /* Additional loop with data-dependent bounds */
    for (i = 0; i < outer_limit; i++) {
        int j;
        for (j = 0; j < inner_limit; j++) {
            /* Complex addressing pattern */
            int idx = (i * 17 + j * 23) % LARGE_SIZE;
            heap_array2[idx] = heap_array2[idx] * 1.1 + (double)heap_array1[idx];
            result += (int)heap_array2[idx];
        }
    }
    
    /* Use the result to prevent optimization */
    printf("Computation result: %d\n", result % 1000);
    
    /* Cleanup */
    free(heap_array1);
    free(heap_array2);
    
    return 0;
}
