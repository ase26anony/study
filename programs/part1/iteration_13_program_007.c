/* cache_detection_test.c
 * Designed to trigger GCC's x86 cache detection logic for uncovered cache descriptor cases
 * Compile with: gcc -O3 -march=nocona -ftree-loop-distribution -fprefetch-loop-arrays cache_detection_test.c -o test
 * Also try: gcc -O2 -march=nehalem -mtune=generic cache_detection_test.c -o test2
 * And: gcc -O1 -march=x86-64 -mtune=core2 -fverbose-asm cache_detection_test.c -o test3
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force compiler to consider cache characteristics by using volatile variables
 * to prevent compile-time optimization of loop bounds */
volatile int outer_limit = 1000;
volatile int inner_limit = 1000;
volatile int stride_val = 16;

/* Large arrays that exceed typical L1 cache sizes */
#define LARGE_SIZE 10000
static int array1[LARGE_SIZE];
static double array2[LARGE_SIZE];
static char array3[LARGE_SIZE * 2];  /* Larger char array for different access patterns */

/* Matrix multiplication style computation - benefits from cache-aware optimization */
void matrix_style_computation(int limit1, int limit2) {
    int i, j, k;
    double sum;
    
    /* Triple nested loop - typical pattern for cache optimization */
    for (i = 0; i < limit1; i++) {
        for (j = 0; j < limit2; j++) {
            sum = 0.0;
            for (k = 0; k < 100; k++) {
                /* Mix array accesses with different strides and types */
                sum += array2[(i * 100 + k) % LARGE_SIZE] * 
                       array2[(j * 50 + k) % LARGE_SIZE];
            }
            array1[(i * limit2 + j) % LARGE_SIZE] = (int)sum;
        }
    }
}

/* Non-unit stride access pattern - tests cache line utilization */
void stride_access_computation(int limit, int stride) {
    int i;
    double acc = 0.0;
    
    /* Access every Nth element - challenges cache prefetching */
    for (i = 0; i < limit; i += stride) {
        array2[i % LARGE_SIZE] = array2[i % LARGE_SIZE] * 1.01 + 0.5;
        acc += array2[i % LARGE_SIZE];
        
        /* Also access char array with different alignment */
        array3[(i * 2) % (LARGE_SIZE * 2)] = (char)(i & 0xFF);
    }
    
    /* Use result to prevent dead code elimination */
    if (acc > 1e10) {
        printf("Stride result: %f\n", acc);  /* This should never print */
    }
}

/* Copy with potential cache line aliasing */
void cache_line_copy(int limit) {
    int i;
    
    /* Copy between arrays with potential aliasing in cache */
    for (i = 0; i < limit; i++) {
        int idx = (i * 17) % LARGE_SIZE;  /* Non-linear access pattern */
        array1[idx] = (int)array2[idx] + array3[i % (LARGE_SIZE * 2)];
    }
}

/* Initialize arrays with pseudo-random data */
void initialize_arrays(void) {
    int i;
    for (i = 0; i < LARGE_SIZE; i++) {
        array1[i] = i % 100;
        array2[i] = (i % 100) * 0.5;
    }
    for (i = 0; i < LARGE_SIZE * 2; i++) {
        array3[i] = (char)(i % 256);
    }
}

/* Conditional compilation for different x86 architectures */
#ifdef __x86_64__
#define ARCH_SPECIFIC_CODE 1
#endif

#ifdef __i386__
#define ARCH_SPECIFIC_CODE 2
#endif

int main(int argc, char *argv[]) {
    int i, iterations;
    double total = 0.0;
    
    /* Use command line args to prevent constant propagation */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 5;
    } else {
        iterations = 5;
    }
    
    initialize_arrays();
    
    /* Architecture-specific code blocks to encourage testing with different -march flags */
#if defined(__x86_64__) && defined(ARCH_SPECIFIC_CODE)
    printf("Running x86_64 optimized version\n");
    /* Additional x86_64 specific operations */
    for (i = 0; i < iterations; i++) {
        matrix_style_computation(outer_limit % 500, inner_limit % 500);
    }
#elif defined(__i386__) && defined(ARCH_SPECIFIC_CODE)
    printf("Running i386 optimized version\n");
    /* i386 specific variant */
    for (i = 0; i < iterations; i++) {
        matrix_style_computation(outer_limit % 300, inner_limit % 300);
    }
#else
    printf("Running generic version\n");
#endif
    
    /* Main computation loops - designed to benefit from cache-aware optimizations */
    for (i = 0; i < iterations; i++) {
        /* Vary parameters slightly each iteration */
        int current_limit = (outer_limit + i) % 800;
        int current_stride = (stride_val + i) % 32 + 1;
        
        matrix_style_computation(current_limit, current_limit / 2);
        stride_access_computation(current_limit * 2, current_stride);
        cache_line_copy(current_limit * 3);
        
        /* Accumulate some result from arrays */
        total += array1[i % LARGE_SIZE] + array2[(i * 7) % LARGE_SIZE];
    }
    
    /* Final computation that uses all arrays */
    for (i = 0; i < LARGE_SIZE; i += 64) {  /* Cache line sized steps */
        total += array1[i] * 0.01;
        total += array2[i] * 2.0;
        if (i < LARGE_SIZE * 2) {
            total += array3[i];
        }
    }
    
    printf("Final result: %f\n", total);
    
    /* Explicit CPU feature detection if available - may prompt driver initialization */
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
    
    return (int)total % 100;
}
