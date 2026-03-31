#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* ========== FUNCTION ATTRIBUTES FOR SCHEDULING CONTROL ========== */

/* Hot function with selective scheduling optimization */
__attribute__((hot, optimize("O3", "sched-pressure"), noinline))
static float hot_loop_scheduler_test(float* restrict a, float* restrict b, 
                                     float* restrict c, int size) {
    float sum = 0.0f;
    
    /* Mixed integer and floating-point operations */
    for (int i = 0; i < size; i++) {
        /* RAW hazard: b depends on a */
        float temp = a[i] * 2.5f;
        
        /* WAR hazard: reusing temp */
        b[i] = temp + 1.0f;
        
        /* WAW hazard: multiple writes to c[i] */
        c[i] = a[i] + b[i];
        c[i] = c[i] * 0.5f;  /* Second write to c[i] */
        
        /* Pointer chasing pattern */
        float* ptr = &c[i];
        *ptr = *ptr + sinf((float)i * 0.01f);
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
        
        /* Conditional move vs branching */
        sum += (temp > 0.0f) ? (*ptr) : (-*ptr);
    }
    
    return sum;
}

/* Cold function with different optimization */
__attribute__((cold, optimize("O2"), noinline))
static int cold_control_flow_test(int* data, int size) {
    int result = 0;
    
    /* Complex control flow with switch */
    for (int i = 0; i < size; i++) {
        switch (data[i] % 7) {
            case 0:
                result += data[i] * 2;
                /* Fall through */
            case 1:
                result += data[i] >> 1;
                break;
            case 2:
                /* Multiple early exits */
                if (data[i] < 0) continue;
                if (data[i] > 1000) break;
                result += data[i] & 0xFF;
                break;
            case 3:
                result ^= data[i];
                break;
            case 4:
                /* Nested if-else */
                if (data[i] % 2 == 0) {
                    result += data[i] / 2;
                } else {
                    result -= data[i] * 3;
                }
                break;
            default:
                /* Computed operation */
                result += (data[i] * data[i]) % 256;
                break;
        }
        
        /* Inline assembly with register clobber */
        asm volatile("" : : : "eax", "ebx", "ecx", "edx");
    }
    
    return result;
}

/* Vectorization-friendly function */
__attribute__((optimize("O3", "tree-vectorize"), always_inline))
static inline void vectorized_loop_test(float* restrict src1, 
                                        float* restrict src2,
                                        float* restrict dst,
                                        int size) {
    /* SIMD-friendly loop with unrolling hint */
    #pragma GCC unroll 4
    for (int i = 0; i < size; i++) {
        /* Mixed operations that can be vectorized */
        float a = src1[i];
        float b = src2[i];
        
        /* Create various dependencies */
        dst[i] = a * b + sinf(a) - cosf(b);
        
        /* Cross-iteration dependency */
        if (i > 0) {
            dst[i] += dst[i-1] * 0.1f;
        }
    }
}

/* Function with outer loop pipelining opportunities */
__attribute__((optimize("O3", "sel-sched-pipelining"), noinline))
static double nested_loop_scheduler_test(double* matrix, int rows, int cols) {
    double total = 0.0;
    
    /* Nested loops with mixed dependencies */
    for (int i = 0; i < rows; i++) {
        double row_sum = 0.0;
        
        /* Inner loop with unrolling */
        #pragma GCC unroll 2
        for (int j = 0; j < cols; j++) {
            /* Pointer chasing with offset */
            double* elem = matrix + (i * cols + j);
            
            /* RAW hazard chain */
            double val = *elem;
            val = val * 1.5 + (double)j;
            *elem = val;
            
            /* Memory barrier between dependent operations */
            asm volatile("" ::: "memory");
            
            row_sum += val;
            
            /* Conditional operation */
            if (j % 3 == 0) {
                row_sum *= 0.99;
            }
        }
        
        total += row_sum;
        
        /* Another scheduling barrier */
        asm volatile("" : : : "xmm0", "xmm1", "xmm2", "xmm3");
    }
    
    return total;
}

/* Function with varying latency operations */
__attribute__((optimize("O3", "schedule-insns"), noinline))
static int mixed_latency_test(int* arr, float* farr, int size) {
    int int_result = 0;
    float float_result = 0.0f;
    
    for (int i = 0; i < size; i++) {
        /* Integer operations (typically lower latency) */
        int x = arr[i];
        x = (x << 3) | (x >> 5);  /* Rotation */
        x = x ^ (x * 13);
        
        /* Floating point operations (higher latency) */
        float y = farr[i];
        y = y * y + sqrtf(fabsf(y));
        
        /* Memory synchronization point */
        asm volatile("" ::: "memory");
        
        /* Interdependency between int and float */
        int_result += (int)(y * 100.0f) + x;
        float_result += y * (float)x;
        
        /* Branch with unpredictable pattern */
        if ((x ^ i) & 1) {
            float_result *= 0.5f;
        } else {
            int_result >>= 1;
        }
    }
    
    return int_result + (int)float_result;
}

/* ========== MAIN EXECUTION FLOW ========== */

int main(void) {
    /* Allocate arrays with different alignments */
    float* farr1 = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* farr2 = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* farr3 = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    double* dmatrix = (double*)aligned_alloc(64, ARRAY_SIZE * ARRAY_SIZE/16 * sizeof(double));
    int* iarr = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        farr1[i] = (float)i * 0.1f;
        farr2[i] = (float)(i % 100) * 0.01f;
        farr3[i] = 0.0f;
        iarr[i] = i * 3 - ARRAY_SIZE/2;
    }
    
    for (int i = 0; i < ARRAY_SIZE * ARRAY_SIZE/16; i++) {
        dmatrix[i] = (double)i * 0.01;
    }
    
    float total_sum = 0.0f;
    int int_result = 0;
    double matrix_result = 0.0;
    
    /* Execute test functions multiple times to ensure coverage */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Modify inputs slightly each iteration */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            farr1[i] += 0.001f;
            iarr[i] ^= iter;
        }
        
        /* Call all test functions in sequence */
        total_sum += hot_loop_scheduler_test(farr1, farr2, farr3, ARRAY_SIZE);
        
        vectorized_loop_test(farr1, farr2, farr3, ARRAY_SIZE);
        total_sum += farr3[ARRAY_SIZE/2];  /* Use result */
        
        int_result += cold_control_flow_test(iarr, ARRAY_SIZE);
        
        matrix_result += nested_loop_scheduler_test(dmatrix, 64, ARRAY_SIZE/16);
        
        int_result += mixed_latency_test(iarr, farr1, ARRAY_SIZE);
        
        /* Memory barrier between iterations */
        asm volatile("" ::: "memory");
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results:\n");
    printf("  Float sum: %f\n", total_sum);
    printf("  Int result: %d\n", int_result);
    printf("  Matrix result: %f\n", matrix_result);
    printf("  Sample values: %f, %f, %d\n", 
           farr3[0], farr3[ARRAY_SIZE-1], iarr[ARRAY_SIZE/2]);
    
    /* Cleanup */
    free(farr1);
    free(farr2);
    free(farr3);
    free(dmatrix);
    free(iarr);
    
    return 0;
}
