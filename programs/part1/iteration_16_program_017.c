/* sel-sched-dump-trigger.c
 * 
 * This program is specifically designed to trigger GCC's selective scheduler
 * debug dump logic, particularly the uncovered lines in sel-sched-dump.cc
 * that perform RTL instruction dumping with switch_dump()/restore_dump().
 *
 * Compilation recommendations:
 *   gcc -O3 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops \
 *       -fdump-rtl-all -c sel-sched-dump-trigger.c -o test.o
 *
 *   gcc -O2 -fschedule-insns -fschedule-insns2 -fdump-rtl-sched1 \
 *       -fdump-rtl-sched2 -c sel-sched-dump-trigger.c
 *
 *   gcc -O3 -funroll-loops -ftree-vectorize -fsel-sched-pipelining \
 *       -fdump-rtl-sched -fprofile-generate sel-sched-dump-trigger.c -o test \
 *       && ./test \
 *       && gcc -O3 -funroll-loops -ftree-vectorize -fsel-sched-pipelining \
 *              -fdump-rtl-sched -fprofile-use sel-sched-dump-trigger.c -o test_opt
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* ============================================================================
 * Function 1: Mixed data dependencies and pointer chasing
 * ============================================================================
 */
__attribute__((hot, noinline))
static double pointer_chase_mixed_ops(double* restrict arr1, 
                                      double* restrict arr2,
                                      int* restrict indices,
                                      int size) {
    double sum = 0.0;
    double acc = 1.0;
    
    /* RAW hazard: arr1[i] -> temp -> sum */
    /* WAR hazard: acc is both read and written in loop */
    /* WAW hazard: Multiple writes to acc through different paths */
    for (int i = 0; i < size; i++) {
        double temp = arr1[i] * 2.5;
        
        /* Pointer chasing with data-dependent index */
        int idx = indices[i] & (size - 1);
        double val = arr2[idx];
        
        /* Mixed FP and integer operations */
        int int_part = (int)temp;
        double frac = temp - int_part;
        
        /* Complex dependency chain */
        acc = acc * 0.99 + val * frac;
        
        /* Conditional update creates control flow for scheduler */
        if (acc > 100.0) {
            acc = acc * 0.5;
            sum += acc * int_part;
        } else {
            sum += acc + int_part;
        }
        
        /* Memory barrier to force scheduling boundary */
        asm volatile("" ::: "memory");
    }
    
    return sum;
}

/* ============================================================================
 * Function 2: SIMD-friendly loops with unrolling directives
 * ============================================================================
 */
__attribute__((optimize("O3"), hot))
static float vectorized_loop(float* restrict a, 
                            float* restrict b,
                            float* restrict c,
                            int size) {
    float sum = 0.0f;
    
    /* Loop designed for auto-vectorization */
    #pragma GCC unroll 4
    for (int i = 0; i < size; i++) {
        /* SIMD-friendly operations */
        float t1 = a[i] * b[i];
        float t2 = a[i] + b[i];
        c[i] = t1 * t2 - sqrtf(fabsf(t1));
        
        /* Horizontal reduction with dependency */
        sum += c[i];
        
        /* Inline asm with register clobber to constrain scheduler */
        if (i % 8 == 0) {
            asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
        }
    }
    
    return sum;
}

/* ============================================================================
 * Function 3: Complex control flow with switch and computed gotos
 * ============================================================================
 */
__attribute__((noinline, cold))
static int control_flow_pattern(int* data, int size, int mode) {
    static void* jump_table[] = {
        &&case_0, &&case_1, &&case_2, &&case_3,
        &&case_4, &&case_5, &&case_6, &&case_7
    };
    
    int result = 0;
    
    for (int i = 0; i < size; i++) {
        int val = data[i];
        
        /* Switch with sparse cases */
        switch (val & 0x7) {
            case 0:
                result += val * 2;
                /* Fall through */
            case 1:
                result += val >> 1;
                break;
            case 2:
                result += val * val;
                /* Memory barrier between dependent ops */
                asm volatile("" ::: "memory");
                result -= val;
                break;
            case 3:
                result += val | 0xFF;
                break;
            case 4:
                /* Computed goto for complex dispatch */
                goto *jump_table[val & 0x7];
            case_4:
                result += val & 0xAAAA;
                break;
            case 5:
                result += ~val;
                break;
            case 6:
                result += val ^ 0x5555;
                break;
            case 7:
                result += val % 17;
                break;
            default:
                result += 1;
        }
        
        /* Conditional move mixed with branch */
        int pred = (val > 1000) ? val : -val;
        result = (mode == 0) ? result + pred : result - pred;
        
        /* Early exit point */
        if (result > 1000000) {
            result /= 2;
            continue;
        }
        
        /* Another early exit with different condition */
        if (result < -1000000) {
            break;
        }
    }
    
    case_0: case_1: case_2: case_3: case_5: case_6: case_7:
    return result;
}

/* ============================================================================
 * Function 4: Nested loops with heterogeneous operations
 * ============================================================================
 */
__attribute__((optimize("sched-pressure")))
static double nested_loop_scheduler_stress(int size) {
    double matrix[32][32];
    double sum = 0.0;
    
    /* Initialize */
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            matrix[i][j] = (i * j) / 100.0;
        }
    }
    
    /* Nested loops with mixed operations */
    for (int iter = 0; iter < size; iter++) {
        for (int i = 1; i < 31; i++) {
            for (int j = 1; j < 31; j++) {
                /* Stencil computation with FP and integer mix */
                double avg = (matrix[i-1][j] + matrix[i+1][j] +
                            matrix[i][j-1] + matrix[i][j+1]) / 4.0;
                
                /* Integer operation in FP loop */
                int int_avg = (int)avg;
                
                /* Trigonometric function adds latency diversity */
                matrix[i][j] = sin(avg) * cos(int_avg * 0.1) + 
                              tanh(avg * 0.01);
                
                /* Reduction with accumulating dependency */
                sum += matrix[i][j] * (i + j);
                
                /* Scheduling barrier every 8 iterations */
                if ((i * j) % 8 == 0) {
                    asm volatile("" ::: "memory");
                }
            }
        }
    }
    
    return sum;
}

/* ============================================================================
 * Function 5: Loop with multiple exit points and continue conditions
 * ============================================================================
 */
__attribute__((hot))
static int multi_exit_loop(int* data, int size, int threshold) {
    int count = 0;
    int total = 0;
    
    for (int i = 0; i < size; i++) {
        /* Multiple continue conditions */
        if (data[i] == 0) {
            continue;
        }
        
        if (data[i] < 0 && (i & 1)) {
            total -= data[i];
            continue;
        }
        
        /* Early exit point 1 */
        if (total > threshold) {
            total /= 2;
            if (++count > 100) break;
        }
        
        /* Early exit point 2 */
        if (total < -threshold) {
            total = -total;
            if (i > size / 2) break;
        }
        
        /* Main computation path */
        int val = data[i];
        val = (val * 1103515245 + 12345) & 0x7fffffff;
        
        /* Conditional update */
        total += (val > 0x40000000) ? val >> 16 : val >> 8;
        
        /* Another continue condition */
        if ((total & 0xFF) == 0) {
            continue;
        }
        
        /* Final update with memory barrier */
        count++;
        asm volatile("" ::: "memory");
    }
    
    return total + count;
}

/* ============================================================================
 * Main function: Calls all test patterns sequentially
 * ============================================================================
 */
int main(void) {
    /* Allocate and initialize data */
    double* arr1 = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double* arr2 = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    float* farr1 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* farr2 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* farr3 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    int* indices = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* idata = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = sin(i * 0.1) * 100.0;
        arr2[i] = cos(i * 0.07) * 50.0;
        farr1[i] = (i % 37) * 0.7f;
        farr2[i] = (i % 41) * 0.3f;
        indices[i] = (i * 1103515245 + 12345) & (ARRAY_SIZE - 1);
        idata[i] = (i * 1664525 + 1013904223) & 0x7FFFFFFF;
    }
    
    double total_result = 0.0;
    
    /* Execute each test function multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Modify inputs slightly each iteration to avoid constant propagation */
        arr1[iter % ARRAY_SIZE] += 0.1;
        idata[iter % ARRAY_SIZE] ^= iter;
        
        /* Test 1: Pointer chasing with mixed dependencies */
        total_result += pointer_chase_mixed_ops(arr1, arr2, indices, ARRAY_SIZE);
        
        /* Test 2: Vectorized loops */
        total_result += vectorized_loop(farr1, farr2, farr3, ARRAY_SIZE);
        
        /* Test 3: Complex control flow */
        total_result += control_flow_pattern(idata, ARRAY_SIZE / 4, iter & 1);
        
        /* Test 4: Nested loop stress */
        total_result += nested_loop_scheduler_stress(10);
        
        /* Test 5: Multi-exit loops */
        total_result += multi_exit_loop(idata, ARRAY_SIZE / 2, 1000000);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Total result: %f\n", total_result);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    free(farr3);
    free(indices);
    free(idata);
    
    return (total_result > 0) ? 0 : 1;
}
