/* sel-sched-trigger.c
 * Program designed to trigger selective scheduler debug dumping
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 -march=native sel-sched-trigger.c -o sel-sched-trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_bound = 1000;
volatile int g_volatile_seed = 42;

/* Simple PRNG to avoid libc rand() overhead in analysis */
static uint32_t prng_state = 123456789;
static inline uint32_t fast_rand(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Function with mixed-width operations to create register pressure */
static int64_t mixed_width_computation(int32_t a, int64_t b, int16_t c) {
    int64_t r1 = (int64_t)a * b;
    int32_t r2 = (int32_t)(r1 >> 16);
    int64_t r3 = r1 + (int64_t)c * 7;
    return (r3 ^ r2) & 0xFFFFFFFF;
}

/* Complex kernel with data dependencies and control flow */
static void compute_kernel_1(float* data_f, int32_t* data_i, int size) {
    volatile int v_bound = g_volatile_bound; /* Prevent optimization */
    float sum_f = 0.0f;
    int64_t sum_i = 0;
    
    /* Nested loops with carried dependencies */
    for (int i = 1; i < size; i++) {
        #pragma GCC unroll 4
        for (int j = 0; j < v_bound; j++) {
            /* Data-dependent computation with cross-iteration dependency */
            float temp_f = data_f[i] * data_f[i-1] + (float)j * 0.1f;
            
            /* Conditional operations creating control flow */
            if (data_i[i] > data_i[i-1]) {
                sum_f += temp_f * 2.0f;
                /* Mixed integer operations */
                sum_i += mixed_width_computation(data_i[i], sum_i, (int16_t)j);
            } else {
                sum_f += temp_f / 1.5f;
                /* Division with non-constant divisor */
                sum_i += (sum_i != 0) ? (data_i[i] / (int32_t)(sum_i & 0xFF)) : data_i[i];
            }
            
            /* Pointer chasing pattern */
            int idx = data_i[i] & 0xFF;
            if (idx < size) {
                data_f[idx] = data_f[idx] * 0.99f + sum_f * 0.01f;
            }
            
            /* Inline assembly to create fixed RTL instructions */
            asm volatile("" : : : "memory");
        }
        
        /* Switch statement for additional control flow */
        switch (i & 3) {
            case 0:
                data_i[i] = (int32_t)(sum_f * 100.0f);
                break;
            case 1:
                data_i[i] = (int32_t)sum_i;
                break;
            case 2:
                data_i[i] = data_i[i] * 2 - data_i[i-1];
                break;
            default:
                data_i[i] = fast_rand() & 0x7FFFFFFF;
                break;
        }
    }
    
    /* Final reduction with side effect */
    printf("Kernel1 sum_f: %.6f, sum_i: %ld\n", sum_f, sum_i);
}

/* Second computation kernel with different patterns */
static void compute_kernel_2(double* matrix, double* vector, double* result, int n) {
    volatile int outer_bound = g_volatile_bound / 10;
    
    for (int iter = 0; iter < outer_bound; iter++) {
        /* Matrix-vector multiplication pattern */
        for (int i = 0; i < n; i++) {
            double acc = 0.0;
            #pragma GCC unroll 2
            for (int j = 0; j < n; j++) {
                /* Non-trivial addressing with stride */
                double val = matrix[i * n + j] * vector[j];
                
                /* Conditional move simulation */
                acc += (val > 0.0) ? val : val * 0.5;
                
                /* Complex floating point operations */
                if ((i + j) & 1) {
                    acc = acc / (1.0 + (double)(i & 0xF) * 0.1);
                }
            }
            
            /* Dependency chain with reduction */
            result[i] = result[i] * 0.9 + acc * 0.1;
            
            /* Another inline assembly barrier */
            asm volatile("" : : : "memory");
        }
        
        /* Update vector with data-dependent pattern */
        for (int i = 0; i < n; i++) {
            vector[i] = vector[i] + result[(i + 1) % n] * 0.01;
        }
    }
    
    /* Compute checksum */
    double checksum = 0.0;
    for (int i = 0; i < n; i++) {
        checksum += result[i] + vector[i];
    }
    printf("Kernel2 checksum: %.6f\n", checksum);
}

/* Main driver with initialization and multiple kernels */
int main(void) {
    const int size = 1024;
    const int matrix_size = 64;
    
    /* Initialize data with pseudo-random values */
    float* data_f = (float*)malloc(size * sizeof(float));
    int32_t* data_i = (int32_t*)malloc(size * sizeof(int32_t));
    double* matrix = (double*)malloc(matrix_size * matrix_size * sizeof(double));
    double* vector = (double*)malloc(matrix_size * sizeof(double));
    double* result = (double*)malloc(matrix_size * sizeof(double));
    
    /* Use volatile seed to prevent compile-time computation */
    prng_state = g_volatile_seed;
    
    for (int i = 0; i < size; i++) {
        data_f[i] = (float)fast_rand() / (float)UINT32_MAX * 100.0f;
        data_i[i] = fast_rand() & 0xFFFF;
    }
    
    for (int i = 0; i < matrix_size * matrix_size; i++) {
        matrix[i] = (double)fast_rand() / (double)UINT32_MAX * 2.0 - 1.0;
    }
    
    for (int i = 0; i < matrix_size; i++) {
        vector[i] = (double)fast_rand() / (double)UINT32_MAX;
        result[i] = 0.0;
    }
    
    /* Execute kernels multiple times to increase scheduling opportunities */
    for (int repeat = 0; repeat < 3; repeat++) {
        compute_kernel_1(data_f, data_i, size);
        compute_kernel_2(matrix, vector, result, matrix_size);
        
        /* Modify volatile to change loop bounds */
        g_volatile_bound = 800 + (repeat * 100);
    }
    
    /* Final reduction to prevent dead code elimination */
    int64_t final_hash = 0;
    for (int i = 0; i < size; i++) {
        final_hash ^= (int64_t)(data_f[i] * 1000) + data_i[i];
    }
    
    printf("Final hash: 0x%016lx\n", final_hash);
    
    /* Cleanup */
    free(data_f);
    free(data_i);
    free(matrix);
    free(vector);
    free(result);
    
    return 0;
}
