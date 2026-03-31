/* cache_detection_test.c
 * Designed to trigger GCC driver's CPU cache detection logic
 * for uncovered cache descriptor values (0x0a, 0x0c, 0x21, 0x49, etc.)
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
volatile int outer_limit;
volatile int stride;

/* Large arrays that exceed L1/L2 cache */
static int matrix_a[2048][2048];
static double matrix_b[1024][1024];
static char buffer[L2_SIZE * 2];
static int result_array[4096];

/* Function prototypes to force different optimization contexts */
void matrix_multiply_kernel(int n, volatile int* restrict out);
void stride_access_pattern(int size, int step, volatile double* restrict out);
void cache_line_aliasing_test(int iterations, volatile char* restrict src, 
                              volatile char* restrict dst);

int main(int argc, char** argv) {
    /* Use arguments to prevent constant propagation */
    outer_limit = (argc > 1) ? atoi(argv[1]) : 1000;
    stride = (argc > 2) ? atoi(argv[2]) : 13;
    
    /* Initialize with pseudo-random but deterministic values */
    srand(42);
    for (int i = 0; i < 2048; i++) {
        for (int j = 0; j < 2048; j++) {
            matrix_a[i][j] = rand() % 256;
        }
    }
    
    for (int i = 0; i < 1024; i++) {
        for (int j = 0; j < 1024; j++) {
            matrix_b[i][j] = (rand() % 256) / 255.0;
        }
    }
    
    /* Fill buffers with pattern */
    for (int i = 0; i < sizeof(buffer); i++) {
        buffer[i] = (i % 256);
    }
    
    double total_result = 0.0;
    
    /* Test 1: Matrix multiplication-like pattern (triple nested loop) */
    printf("Running matrix multiplication kernel...\n");
    matrix_multiply_kernel(512, result_array);
    
    /* Test 2: Non-unit stride access pattern */
    printf("Running stride access pattern...\n");
    volatile double stride_results[1024];
    stride_access_pattern(1000000, stride, stride_results);
    
    /* Test 3: Cache line aliasing test */
    printf("Running cache line aliasing test...\n");
    char dest_buffer[sizeof(buffer)];
    cache_line_aliasing_test(10000, buffer, dest_buffer);
    
    /* Test 4: Mixed data type operations */
    printf("Running mixed data type operations...\n");
    volatile int mixed_result = 0;
    for (int i = 0; i < outer_limit; i += CACHE_LINE) {
        for (int j = 0; j < 1024; j += 8) {
            /* Mix int and double operations */
            mixed_result += matrix_a[i % 2048][j % 2048];
            total_result += matrix_b[i % 1024][j % 1024];
            
            /* Force potential cache conflicts */
            if ((i * j) % 7 == 0) {
                buffer[(i * j) % sizeof(buffer)] = mixed_result % 256;
            }
        }
    }
    
    /* Test 5: Blocked matrix transposition (cache-aware algorithm) */
    printf("Running blocked matrix transposition...\n");
    int block_size = 32; /* Should be tuned based on cache line */
    for (int i0 = 0; i0 < 512; i0 += block_size) {
        for (int j0 = 0; j0 < 512; j0 += block_size) {
            for (int i = i0; i < i0 + block_size && i < 512; i++) {
                for (int j = j0; j < j0 + block_size && j < 512; j++) {
                    /* Simulate transpose operation */
                    int temp = matrix_a[i][j];
                    matrix_a[j][i] = temp;
                }
            }
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Mixed result: %d\n", mixed_result);
    printf("Total result: %f\n", total_result);
    
    /* Conditional compilation for different x86 architectures */
    #ifdef __x86_64__
    printf("x86_64 architecture detected\n");
    
    /* Code that benefits from different cache configurations */
    #if defined(__tune_core2__) || defined(__tune_nocona__)
    /* Optimize for Core2/Nocona cache hierarchy */
    for (int i = 0; i < 100000; i += 64) {
        for (int j = 0; j < 64; j++) {
            result_array[(i + j) % 4096] = buffer[i % sizeof(buffer)] * j;
        }
    }
    #endif
    
    #if defined(__tune_nehalem__) || defined(__tune_westmere__)
    /* Optimize for Nehalem/Westmere (includes case 0x49) */
    for (int i = 0; i < 200000; i += 128) {
        double acc = 0.0;
        for (int j = 0; j < 128; j += 8) {
            acc += matrix_b[i % 1024][j % 1024];
        }
        result_array[i % 4096] = (int)acc;
    }
    #endif
    
    #if defined(__tune_generic__)
    /* Generic x86_64 tuning - should query many cache descriptors */
    for (int i = 0; i < 300000; i++) {
        /* Access pattern that benefits from knowing cache size */
        int idx = (i * 17) % 4096;
        result_array[idx] = matrix_a[idx % 2048][(idx * 3) % 2048];
    }
    #endif
    
    #endif /* __x86_64__ */
    
    return 0;
}

/* Matrix multiplication kernel - triple nested loop */
void matrix_multiply_kernel(int n, volatile int* restrict out) {
    int temp[512];
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int sum = 0;
            for (int k = 0; k < n; k++) {
                sum += matrix_a[i][k] * matrix_a[k][j];
            }
            temp[j] = sum;
        }
        /* Store result with stride */
        if (i % 4 == 0) {
            out[i / 4] = temp[n - 1];
        }
    }
}

/* Non-unit stride access pattern */
void stride_access_pattern(int size, int step, volatile double* restrict out) {
    double accumulator = 0.0;
    
    for (int i = 0; i < size; i += step) {
        /* Access with large stride - depends on cache line size */
        accumulator += matrix_b[i % 1024][(i * 2) % 1024];
        
        /* Every 8th iteration, access different array */
        if (i % 8 == 0) {
            out[(i / step) % 1024] = accumulator;
            accumulator = matrix_a[i % 2048][(i + 1) % 2048];
        }
    }
}

/* Cache line aliasing test */
void cache_line_aliasing_test(int iterations, volatile char* restrict src, 
                              volatile char* restrict dst) {
    /* Copy with potential cache line conflicts */
    for (int i = 0; i < iterations; i++) {
        int src_idx = (i * CACHE_LINE) % (L2_SIZE * 2);
        int dst_idx = ((i * CACHE_LINE) + 32) % (L2_SIZE * 2);
        
        /* Copy entire cache line */
        for (int j = 0; j < CACHE_LINE; j++) {
            dst[dst_idx + j] = src[src_idx + j];
        }
        
        /* Every 16 iterations, flush pattern */
        if (i % 16 == 0) {
            for (int j = 0; j < 64; j += 8) {
                dst[(dst_idx + j) % (L2_SIZE * 2)] = j;
            }
        }
    }
}
