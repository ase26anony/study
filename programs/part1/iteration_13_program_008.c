/* cache_detection_test.c
 * Designed to trigger GCC driver's CPU cache detection logic
 * for x86 cache descriptor values: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c,
 * 0x39-0x3e, 0x41-0x45, 0x48, 0x49, 0x4e, 0x60, 0x66-0x68, 0x78-0x80,
 * 0x82-0x87
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force compiler to consider cache characteristics by using
 * arrays that exceed typical L1/L2 cache sizes */
#define LARGE_SIZE 10000
#define MEDIUM_SIZE 5000
#define SMALL_SIZE 1000

/* Volatile variables to prevent compile-time optimization */
volatile int outer_limit = LARGE_SIZE;
volatile int inner_limit = MEDIUM_SIZE;

/* Different data types to test various cache line behaviors */
static int matrix_a[LARGE_SIZE][MEDIUM_SIZE];
static double matrix_b[MEDIUM_SIZE][SMALL_SIZE];
static char buffer[LARGE_SIZE * 4];
static long results[SMALL_SIZE];

/* Function prototypes to force separate compilation consideration */
void matrix_multiply_pattern(int size);
void stride_access_pattern(int stride);
void cache_line_aliasing_test(void);
void mixed_operations(void);

/* Matrix multiplication-like triple nested loop */
void matrix_multiply_pattern(int size) {
    volatile int n = size;
    double sum;
    
    /* Triple nested loop - classic cache challenge */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            sum = 0.0;
            for (int k = 0; k < n; k++) {
                /* Access with different strides to test cache behavior */
                sum += matrix_a[i][k] * matrix_b[k][j];
            }
            results[i % SMALL_SIZE] = (long)sum;
        }
    }
}

/* Non-unit stride access pattern */
void stride_access_pattern(int stride) {
    volatile int limit = outer_limit;
    volatile int step = stride;
    long accumulator = 0;
    
    /* Access every Nth element to test cache line utilization */
    for (int i = 0; i < limit; i += step) {
        for (int j = 0; j < inner_limit; j++) {
            /* Mix array types to vary access sizes */
            accumulator += matrix_a[i][j % MEDIUM_SIZE];
            accumulator += (long)(matrix_b[j % MEDIUM_SIZE][i % SMALL_SIZE] * 100);
        }
        buffer[i % (LARGE_SIZE * 4)] = (char)(accumulator & 0xFF);
    }
}

/* Potential cache line aliasing scenario */
void cache_line_aliasing_test(void) {
    volatile int iterations = SMALL_SIZE * 10;
    int temp[64]; /* Typical cache line size */
    
    for (int i = 0; i < iterations; i++) {
        /* Copy between arrays with potential aliasing */
        for (int j = 0; j < 64; j++) {
            temp[j] = matrix_a[i % LARGE_SIZE][j % MEDIUM_SIZE];
            buffer[(i * 64 + j) % (LARGE_SIZE * 4)] = (char)temp[j];
        }
        
        /* Reverse copy to ensure data dependency */
        for (int j = 63; j >= 0; j--) {
            matrix_a[i % LARGE_SIZE][j % MEDIUM_SIZE] = temp[63 - j];
        }
    }
}

/* Mixed operations to force various optimization considerations */
void mixed_operations(void) {
    volatile int limit = MEDIUM_SIZE;
    double fp_accumulator = 0.0;
    int int_accumulator = 0;
    
    for (int i = 0; i < limit; i++) {
        /* Integer operations */
        for (int j = 0; j < SMALL_SIZE; j++) {
            int_accumulator += matrix_a[i % LARGE_SIZE][j] * j;
        }
        
        /* Floating point operations */
        for (int j = 0; j < SMALL_SIZE; j++) {
            fp_accumulator += matrix_b[i][j] * 0.01;
        }
        
        /* Character/buffer operations */
        for (int j = 0; j < 256; j++) {
            buffer[(i * 256 + j) % (LARGE_SIZE * 4)] = 
                (char)((int_accumulator + (int)fp_accumulator) & 0xFF);
        }
    }
    
    /* Store final results to prevent dead code elimination */
    results[0] = (long)int_accumulator;
    results[1] = (long)fp_accumulator;
}

/* Conditional compilation for different x86 architectures */
#ifdef __x86_64__
void x86_64_specific_workload(void) {
    /* This code encourages testing with different -march flags */
    printf("x86-64 architecture detected\n");
    
    /* Use CPUID-like intrinsic if available to trigger detection */
    #ifdef __GNUC__
    /* These builtins may prompt CPU feature detection */
    __builtin_cpu_init();
    
    /* Test for various features that might correlate with cache types */
    if (__builtin_cpu_supports("sse2")) {
        printf("SSE2 supported\n");
    }
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported\n");
    }
    #endif
    
    /* Additional workload specific to 64-bit */
    volatile int extra_work = 1000000;
    double extra_sum = 0.0;
    
    for (int i = 0; i < extra_work; i++) {
        extra_sum += 1.0 / (i + 1);
        if (i % 1000 == 0) {
            buffer[i % 1000] = (char)((int)extra_sum & 0xFF);
        }
    }
    results[2] = (long)extra_sum;
}
#endif

#ifdef __i386__
void i386_specific_workload(void) {
    printf("i386 architecture detected\n");
    
    /* Different loop pattern for 32-bit */
    volatile int count = 500000;
    int local_accum = 0;
    
    for (int i = 0; i < count; i++) {
        local_accum += (i * 3) / 2;
        matrix_a[i % 100][i % 50] = local_accum;
    }
    results[3] = local_accum;
}
#endif

int main(int argc, char *argv[]) {
    /* Initialize with pseudo-random but deterministic values */
    srand(42);
    
    for (int i = 0; i < LARGE_SIZE; i++) {
        for (int j = 0; j < MEDIUM_SIZE; j++) {
            matrix_a[i][j] = rand() % 100;
        }
    }
    
    for (int i = 0; i < MEDIUM_SIZE; i++) {
        for (int j = 0; j < SMALL_SIZE; j++) {
            matrix_b[i][j] = (rand() % 100) / 100.0;
        }
    }
    
    printf("Starting cache-intensive workload...\n");
    
    /* Execute various patterns to exercise different cache behaviors */
    matrix_multiply_pattern(200);
    printf("Completed matrix pattern\n");
    
    stride_access_pattern(7);  /* Prime number stride */
    printf("Completed stride access pattern\n");
    
    cache_line_aliasing_test();
    printf("Completed cache line aliasing test\n");
    
    mixed_operations();
    printf("Completed mixed operations\n");
    
    /* Architecture-specific workloads */
    #ifdef __x86_64__
    x86_64_specific_workload();
    #endif
    
    #ifdef __i386__
    i386_specific_workload();
    #endif
    
    /* Final accumulation and output to prevent optimization */
    long final_result = 0;
    for (int i = 0; i < SMALL_SIZE; i++) {
        final_result += results[i];
    }
    
    /* Also use array contents */
    for (int i = 0; i < 100; i++) {
        final_result += buffer[i * 13];
    }
    
    printf("Final result: %ld\n", final_result);
    printf("Workload completed successfully.\n");
    
    return (int)(final_result % 100);
}
