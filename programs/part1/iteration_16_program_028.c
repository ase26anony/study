/* Selective scheduling stress test designed to trigger GCC's internal
   RTL dump logic in sel-sched-dump.cc, specifically the uncovered
   switch_dump/dump_insn_rtx_1 block. */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#define SIZE 1024
#define ITERATIONS 100

/* ==================== Function Attributes for Scheduling Control ==================== */

__attribute__((hot, optimize("O3"), noinline))
static float hot_loop_with_hazards(float* restrict a, float* restrict b, 
                                   float* restrict c, int n) {
    /* RAW, WAR, and WAW hazards mixed with pointer chasing */
    float sum = 0.0f;
    float* ptr_a = a;
    float* ptr_b = b;
    float* ptr_c = c;
    
    for (int i = 0; i < n; ++i) {
        /* RAW: b depends on a */
        float t1 = *ptr_a * 2.0f;
        float t2 = t1 + *ptr_b;  /* RAW hazard */
        
        /* WAR: t3 overwrites t2's register before t2 is used? No - but compiler thinks maybe */
        float t3 = *ptr_a + *ptr_b;
        *ptr_c = t2 + t3;  /* WAW hazard potential with later writes */
        
        /* Pointer chasing with assembly barrier */
        asm volatile("" ::: "memory");
        
        /* More dependencies */
        sum += *ptr_c;
        *ptr_a = t3 * 0.5f;  /* WAR: a written after being read earlier */
        
        ptr_a++;
        ptr_b++;
        ptr_c++;
    }
    return sum;
}

__attribute__((cold, optimize("sched-pressure"), noinline))
static int cold_control_flow(int* data, int n) {
    /* Complex control flow with switch and computed gotos */
    int result = 0;
    for (int i = 0; i < n; ++i) {
        switch (data[i] % 7) {
            case 0: result += data[i] * 2; break;
            case 1: result += data[i] >> 1; break;
            case 2: result += data[i] & 0xFF; break;
            case 3: result ^= data[i]; break;
            case 4: result |= data[i]; break;
            case 5: result = (result > data[i]) ? result : data[i]; /* conditional move */
            default: result -= data[i]; break;
        }
        
        /* Multiple early exit points */
        if (result > 1000000) return result;
        if (result < -1000000) break;
        if (i % 13 == 0) continue;
        
        /* Mixed integer/float operations */
        float f = (float)result;
        f = sqrtf(fabsf(f) + 1.0f);
        result = (int)f;
    }
    return result;
}

__attribute__((optimize("O3"), noinline))
static void vectorized_unrolled_loop(double* restrict arr1, double* restrict arr2, 
                                     double* restrict out, int n) {
    /* SIMD-friendly loop with unrolling pragma */
    #pragma GCC unroll 4
    for (int i = 0; i < n; ++i) {
        /* Mixed operations that should vectorize */
        double a = arr1[i];
        double b = arr2[i];
        
        /* Create scheduling pressure with FP operations */
        double t1 = a * b;
        double t2 = sin(a) + cos(b);
        double t3 = t1 / (fabs(t2) + 1.0);
        double t4 = sqrt(t1 * t1 + t2 * t2);
        
        /* Assembly with register clobbers */
        asm volatile("" : "+r"(t3) : : "r8", "r9", "r10");
        
        out[i] = t3 + t4;
        
        /* Memory barrier between dependent operations */
        if (i % 8 == 0) {
            asm volatile("" ::: "memory");
        }
    }
}

__attribute__((optimize("O3")))
static void nested_loop_with_barriers(int* mat, int size) {
    /* Nested loops with data dependencies and inline asm barriers */
    for (int i = 1; i < size - 1; ++i) {
        for (int j = 1; j < size - 1; ++j) {
            /* Stencil computation with RAW hazards */
            int idx = i * size + j;
            int up = mat[idx - size];
            int down = mat[idx + size];
            int left = mat[idx - 1];
            int right = mat[idx + 1];
            
            /* Scheduling barrier */
            asm volatile("" ::: "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
            
            /* Computation with dependencies */
            int sum = up + down + left + right;
            int avg = sum / 4;
            
            /* Conditional update */
            mat[idx] = (avg > mat[idx]) ? avg : mat[idx];
            
            /* Another barrier every few iterations */
            if ((i * j) % 16 == 0) {
                asm volatile("" ::: "memory");
            }
        }
    }
}

/* ==================== Main Execution Flow ==================== */

int main(void) {
    /* Allocate aligned memory for vectorization */
    float* farr1 = (float*)aligned_alloc(32, SIZE * sizeof(float));
    float* farr2 = (float*)aligned_alloc(32, SIZE * sizeof(float));
    float* farr3 = (float*)aligned_alloc(32, SIZE * sizeof(float));
    
    double* darr1 = (double*)aligned_alloc(32, SIZE * sizeof(double));
    double* darr2 = (double*)aligned_alloc(32, SIZE * sizeof(double));
    double* darr3 = (double*)aligned_alloc(32, SIZE * sizeof(double));
    
    int* iarr1 = (int*)aligned_alloc(32, SIZE * SIZE * sizeof(int));
    int* iarr2 = (int*)aligned_alloc(32, SIZE * sizeof(int));
    
    if (!farr1 || !farr2 || !farr3 || !darr1 || !darr2 || !darr3 || !iarr1 || !iarr2) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < SIZE; ++i) {
        farr1[i] = (float)i * 0.1f;
        farr2[i] = (float)(SIZE - i) * 0.2f;
        farr3[i] = 0.0f;
        
        darr1[i] = (double)i * 0.01;
        darr2[i] = (double)(SIZE - i) * 0.02;
        darr3[i] = 0.0;
        
        iarr2[i] = (i * 17) % 7919;
    }
    
    for (int i = 0; i < SIZE * SIZE; ++i) {
        iarr1[i] = (i * 13) % 9973;
    }
    
    float total_sum = 0.0f;
    int int_result = 0;
    
    /* Execute test functions multiple times to ensure coverage */
    for (int iter = 0; iter < ITERATIONS; ++iter) {
        /* Modify inputs slightly each iteration */
        farr1[iter % SIZE] += 0.5f;
        iarr2[iter % SIZE] ^= iter;
        
        /* Call functions with scheduling challenges */
        total_sum += hot_loop_with_hazards(farr1, farr2, farr3, SIZE);
        
        int_result += cold_control_flow(iarr2, SIZE);
        
        vectorized_unrolled_loop(darr1, darr2, darr3, SIZE);
        
        if (iter % 10 == 0) {
            nested_loop_with_barriers(iarr1, SIZE);
        }
        
        /* Prevent dead code elimination */
        asm volatile("" : "+r"(total_sum), "+r"(int_result) : : "memory");
    }
    
    /* Use results to prevent optimization */
    printf("Result: sum=%f, int=%d, darr3[0]=%f, iarr1[0]=%d\n", 
           total_sum, int_result, darr3[0], iarr1[0]);
    
    /* Cleanup */
    free(farr1);
    free(farr2);
    free(farr3);
    free(darr1);
    free(darr2);
    free(darr3);
    free(iarr1);
    free(iarr2);
    
    return 0;
}
