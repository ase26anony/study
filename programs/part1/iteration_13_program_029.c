/* cache_detection_test.c
 * Designed to trigger GCC driver cache detection logic for x86 targets
 * Compile with various -march and -mtune options to exercise different cache descriptor cases
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
    int result = 0;
    
    /* Triple nested loop - typical for cache optimization */
    for (int i = 0; i < limit; i++) {
        for (int j = 0; j < limit; j++) {
            for (int k = 0; k < limit; k++) {
                /* Mix operations to vary access patterns */
                array1[(i * limit + j) % LARGE_SIZE] += 
                    array2[(j * limit + k) % LARGE_SIZE] * 
                    (k % 8);
                darray1[(i + k) % LARGE_SIZE] = 
                    darray2[(j + k) % LARGE_SIZE] * 1.5;
            }
        }
    }
}

/* Non-unit stride access pattern */
void stride_access_computation(int n, int stride_val) {
    volatile int limit = n;
    volatile int step = stride_val;
    long sum = 0;
    
    for (int i = 0; i < limit; i += step) {
        for (int j = 0; j < limit; j++) {
            /* Access every Nth element to test cache line utilization */
            sum += array1[(i * j) % LARGE_SIZE];
            carray1[(j * step) % LARGE_SIZE] = 
                carray2[(i * step) % LARGE_SIZE] + (j % 256);
        }
    }
    
    printf("Stride sum: %ld\n", sum);
}

/* Cache line aliasing test */
void aliasing_test(int n) {
    volatile int limit = n;
    int temp[1024];  /* Sized to potentially conflict with cache */
    
    /* Copy with potential cache line conflicts */
    for (int i = 0; i < limit; i++) {
        for (int j = 0; j < 1024; j++) {
            temp[j] = array1[(i + j) % LARGE_SIZE];
            array2[(i * 16 + j) % LARGE_SIZE] = temp[j] * 2;
        }
    }
}

/* Mixed data type operations */
void mixed_operations(int n) {
    volatile int limit = n;
    double acc = 0.0;
    
    for (int i = 0; i < limit; i++) {
        /* Mix int and double operations */
        int idx = (i * 7) % LARGE_SIZE;
        darray1[idx] = (double)array1[idx] * 1.414;
        acc += darray1[idx];
        
        /* Char array manipulation */
        carray1[i % LARGE_SIZE] = (char)(array1[idx] % 256);
    }
    
    printf("Accumulated: %f\n", acc);
}

/* Conditional compilation for different architectures */
#ifdef __x86_64__
void x86_64_specific_code(void) {
    printf("x86_64 architecture detected\n");
    
    /* Try to explicitly trigger CPU feature detection if builtins available */
    #ifdef __GNUC__
    /* These builtins may prompt cache detection */
    __builtin_cpu_init();
    if (__builtin_cpu_supports("sse")) {
        printf("SSE supported\n");
    }
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported\n");
    }
    #endif
    
    /* Architecture-specific optimization hints */
    asm volatile("" ::: "memory");
}
#endif

#ifdef __i386__
void i386_specific_code(void) {
    printf("i386 architecture detected\n");
    
    /* Different loop patterns for 32-bit */
    volatile int small_limit = 500;
    for (int i = 0; i < small_limit; i++) {
        array1[i * 2 % LARGE_SIZE] = i * 3;
    }
}
#endif

int main(int argc, char *argv[]) {
    int iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    
    printf("Cache detection test - iterations: %d\n", iterations);
    
    /* Initialize arrays with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < LARGE_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        darray1[i] = (double)(rand() % 1000) / 3.0;
        darray2[i] = (double)(rand() % 1000) / 3.0;
        carray1[i] = (char)(rand() % 256);
        carray2[i] = (char)(rand() % 256);
    }
    
    /* Execute different computation patterns */
    matrix_style_computation(iterations / 10);
    stride_access_computation(iterations, stride);
    aliasing_test(iterations / 2);
    mixed_operations(iterations);
    
    /* Architecture-specific code paths */
    #ifdef __x86_64__
    x86_64_specific_code();
    #endif
    
    #ifdef __i386__
    i386_specific_code();
    #endif
    
    /* Final computation and output to prevent dead code elimination */
    long final_sum = 0;
    for (int i = 0; i < LARGE_SIZE; i += 64) {  /* Cache line sized steps */
        final_sum += array1[i] + (int)darray1[i] + carray1[i];
    }
    
    printf("Final result: %ld\n", final_sum);
    return 0;
}
