/* sel-sched-dump-trigger.c
 * 
 * This program is specifically designed to trigger GCC's selective scheduler
 * debug dump logic (sel-sched-dump.cc lines 159-163) by creating complex
 * scheduling scenarios that require detailed RTL instruction dumping.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

/* ========== Function Attributes for Scheduling Control ========== */

/* Hot function with selective scheduling pressure */
__attribute__((hot, optimize("O3"), optimize("sched-pressure")))
static float hot_loop_scheduler(float *data, int size) {
    volatile float sum = 0.0f;
    
    /* Mixed integer and floating point operations */
    for (int i = 0; i < size; i++) {
        /* RAW hazard: data[i] read after potential write */
        float val = data[i];
        
        /* Floating point operation chain */
        val = val * 1.5f + 2.0f;
        val = sinf(val) * cosf(val);
        
        /* Integer operation mixed in */
        int idx = (int)val % size;
        idx = idx < 0 ? size + idx : idx;
        
        /* WAR hazard: reusing val */
        val = data[idx] * 0.7f + val * 0.3f;
        
        /* WAW hazard: multiple writes to sum */
        sum += val;
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
        
        /* Another dependent operation */
        sum = sum * 0.99f + 0.01f * val;
    }
    
    return sum;
}

/* Cold function with noinline to create scheduling boundaries */
__attribute__((cold, noinline))
static double cold_path_scheduler(int *ints, double *doubles, int n) {
    double result = 0.0;
    
    /* Complex control flow with switch */
    for (int i = 0; i < n; i++) {
        switch (ints[i] % 7) {
            case 0:
                result += doubles[i] * 1.1;
                /* Fall through */
            case 1:
                result += sqrt(doubles[i]);
                break;
            case 2:
                /* Conditional move pattern */
                result = (doubles[i] > 0.5) ? result * 1.5 : result / 1.5;
                break;
            case 3:
                /* Pointer chasing simulation */
                double *ptr = &doubles[i];
                for (int j = 0; j < 3; j++) {
                    result += *ptr;
                    ptr = &doubles[(i + j) % n];
                }
                break;
            case 4:
                /* Early exit point */
                if (result > 1000.0) return result;
                continue;
            default:
                result += (double)ints[i] * doubles[i];
        }
        
        /* Another memory barrier */
        asm volatile("" ::: "memory", "eax", "ebx", "ecx", "edx");
    }
    
    return result;
}

/* Vectorization-friendly function with unrolling */
__attribute__((optimize("O3"), always_inline))
static inline void vectorized_scheduler(float *a, float *b, float *c, int size) {
    int i;
    
    /* Manual unrolling hint */
#pragma GCC unroll 4
    for (i = 0; i < size; i++) {
        /* SIMD-friendly operations */
        a[i] = b[i] * c[i] + a[i];
        b[i] = c[i] * 2.0f - b[i];
        c[i] = a[i] + b[i] * 0.5f;
        
        /* Data-dependent branching */
        if (i > 0) {
            a[i] += c[i-1] * 0.3f;
        }
    }
    
    /* Additional loop with different stride */
#pragma GCC unroll 2
    for (i = size - 1; i >= 0; i -= 2) {
        /* Reverse processing creates scheduling challenges */
        c[i] = a[i] * b[i] - c[i];
        if (i > 0) {
            c[i-1] = a[i-1] * b[i-1] + c[i-1];
        }
    }
}

/* Function with mixed hazards and assembly constraints */
__attribute__((optimize("O2"), noinline))
static int mixed_hazard_scheduler(int *arr, int n) {
    int sum = 0;
    int prod = 1;
    
    /* Multiple interleaved dependencies */
    for (int i = 0; i < n; i++) {
        int val = arr[i];
        
        /* RAW: val used before potential modification */
        sum += val;
        
        /* Inline assembly with register clobber */
        asm volatile("movl %0, %%eax\n\t"
                     "addl $1, %%eax\n\t"
                     : : "r"(val) : "eax", "memory");
        
        /* WAR: val reused */
        val = sum % 256;
        
        /* WAW: multiple writes to prod */
        prod *= val;
        prod = prod & 0xFFF;
        
        /* Complex expression with side effects */
        arr[i] = (val * prod + sum) % 1024;
        
        /* Another barrier */
        asm volatile("" ::: "memory");
    }
    
    return sum + prod;
}

/* Function with computed goto for complex control flow */
__attribute__((optimize("O1")))
static double computed_goto_scheduler(double *data, int size, int mode) {
    double result = 0.0;
    int i = 0;
    
    /* Label array for computed goto */
    static void *labels[] = { &&loop_start, &&case_a, &&case_b, &&case_c, &&default_case };
    
    goto *labels[mode % 5];
    
loop_start:
    while (i < size) {
        /* Multiple early exit conditions */
        if (data[i] < 0) goto case_a;
        if (data[i] > 1000) goto case_b;
        if (i % 13 == 0) goto case_c;
        
        result += data[i] * 0.5;
        i++;
        continue;
        
case_a:
        result -= data[i] * 0.3;
        i += 2;
        if (i >= size) break;
        goto loop_start;
        
case_b:
        result *= 1.1;
        i++;
        if (result > 1e6) return result;
        goto loop_start;
        
case_c:
        result = sqrt(fabs(result));
        i += 3;
        goto loop_start;
        
default_case:
        result += 1.0;
        i++;
        goto loop_start;
    }
    
    return result;
}

/* ========== Main Test Driver ========== */

int main(void) {
    const int SIZE = 1024;
    float *float_data = (float*)aligned_alloc(16, SIZE * sizeof(float));
    int *int_data = (int*)malloc(SIZE * sizeof(int));
    double *double_data = (double*)malloc(SIZE * sizeof(double));
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        float_data[i] = (float)(i % 100) * 0.1f;
        int_data[i] = i * 3 - SIZE/2;
        double_data[i] = sin((double)i * 0.01) * 100.0;
    }
    
    double total_result = 0.0;
    
    /* Test 1: Hot loop with mixed operations */
    printf("Test 1: Hot loop scheduler...\n");
    float result1 = hot_loop_scheduler(float_data, SIZE);
    total_result += result1;
    printf("  Result: %f\n", result1);
    
    /* Test 2: Cold path with complex control flow */
    printf("Test 2: Cold path scheduler...\n");
    double result2 = cold_path_scheduler(int_data, double_data, SIZE);
    total_result += result2;
    printf("  Result: %f\n", result2);
    
    /* Test 3: Vectorized operations */
    printf("Test 3: Vectorized scheduler...\n");
    float *a = (float*)aligned_alloc(16, SIZE * sizeof(float));
    float *b = (float*)aligned_alloc(16, SIZE * sizeof(float));
    float *c = (float*)aligned_alloc(16, SIZE * sizeof(float));
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = (float)i * 0.01f;
        b[i] = (float)(SIZE - i) * 0.02f;
        c[i] = (float)(i % 50) * 0.05f;
    }
    
    vectorized_scheduler(a, b, c, SIZE);
    
    /* Use results to prevent dead code elimination */
    float sum3 = 0.0f;
    for (int i = 0; i < SIZE; i++) {
        sum3 += a[i] + b[i] + c[i];
    }
    total_result += sum3;
    printf("  Result: %f\n", sum3);
    
    /* Test 4: Mixed hazards */
    printf("Test 4: Mixed hazard scheduler...\n");
    int result4 = mixed_hazard_scheduler(int_data, SIZE);
    total_result += result4;
    printf("  Result: %d\n", result4);
    
    /* Test 5: Computed goto scheduler */
    printf("Test 5: Computed goto scheduler...\n");
    double result5 = computed_goto_scheduler(double_data, SIZE, 2);
    total_result += result5;
    printf("  Result: %f\n", result5);
    
    /* Final accumulation and output */
    printf("\nTotal accumulated result: %f\n", total_result);
    
    /* Cleanup */
    free(float_data);
    free(int_data);
    free(double_data);
    free(a);
    free(b);
    free(c);
    
    return (total_result > 0) ? 0 : 1;
}
