#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Function with complex scheduling patterns */
__attribute__((hot, optimize("O3")))
static float hot_function(float* restrict a, float* restrict b, float* restrict c, int n) {
    float sum = 0.0f;
    
    /* Mixed integer/float operations with dependencies */
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* RAW hazard: b depends on a */
        float temp = a[i] * 2.0f;
        
        /* WAR hazard: temp is written, then read */
        b[i] = temp + 1.0f;
        temp = b[i] * 3.0f;
        
        /* WAW hazard: c[i] written twice */
        c[i] = temp;
        c[i] = c[i] * 0.5f;
        
        /* Pointer chasing pattern */
        float* ptr = &c[i];
        *ptr = *ptr + sinf((float)i);
        
        /* Memory barrier forcing scheduler decisions */
        asm volatile("" ::: "memory");
        
        sum += c[i];
    }
    
    return sum;
}

/* Cold function with different scheduling characteristics */
__attribute__((cold, noinline, optimize("sched-pressure")))
static double cold_function(double* restrict arr, int* restrict indices, int n) {
    double result = 0.0;
    
    /* Complex control flow with switch */
    for (int i = 0; i < n; i++) {
        int idx = indices[i] % 8;
        
        switch (idx) {
            case 0:
                arr[i] = arr[i] * 1.1;
                break;
            case 1:
                arr[i] = arr[i] + 2.5;
                /* Fall through */
            case 2:
                arr[i] = arr[i] - 1.0;
                break;
            case 3:
                arr[i] = arr[i] / 3.0;
                /* Inline asm with register clobber */
                asm volatile("" ::: "rax", "rcx");
                break;
            case 4:
                arr[i] = sqrt(arr[i]);
                break;
            default:
                arr[i] = arr[i] * arr[i];
                break;
        }
        
        /* Conditional move vs branch */
        result += (idx > 3) ? arr[i] * 2.0 : arr[i];
        
        /* Early exit condition */
        if (result > 1000000.0) {
            break;
        }
        
        /* Continue with another condition */
        if (i % 7 == 0) {
            continue;
        }
        
        /* Additional computation */
        result += 0.1;
    }
    
    return result;
}

/* Vectorization-friendly function */
__attribute__((optimize("O3")))
static void vectorized_loop(int* restrict src1, int* restrict src2, 
                           int* restrict dst, int n) {
    /* SIMD-friendly loop with multiple dependencies */
    #pragma GCC unroll(8)
    for (int i = 0; i < n; i++) {
        /* Multiple RAW hazards */
        int t1 = src1[i] + i;
        int t2 = src2[i] * 2;
        int t3 = t1 + t2;
        
        /* Cross-iteration dependency */
        if (i > 0) {
            t3 += dst[i-1];
        }
        
        /* Complex expression with mixed operations */
        dst[i] = (t3 * 3) / 2 + (src1[i] & 0xFF) | (src2[i] << 2);
        
        /* Memory barrier splitting scheduling regions */
        if (i % 16 == 0) {
            asm volatile("" ::: "memory");
        }
    }
}

/* Function with nested loops and mixed operations */
__attribute__((noinline))
static float nested_loop_scheduler_test(float* matrix, int size) {
    float total = 0.0f;
    
    /* Nested loops create complex scheduling regions */
    for (int i = 0; i < size; i++) {
        float row_sum = 0.0f;
        
        /* Inner loop with mixed FP/int operations */
        for (int j = 0; j < size; j++) {
            float val = matrix[i * size + j];
            
            /* Floating point operation chain */
            val = val * 1.5f;
            val = val + sinf((float)j);
            val = val / (1.0f + fabsf((float)i - (float)j));
            
            /* Integer operation mixed in */
            int int_part = (int)val;
            val = val - (float)int_part;
            
            row_sum += val;
            
            /* Scheduling barrier every 8 iterations */
            if (j % 8 == 0) {
                asm volatile("" ::: "r8", "r9", "r10");
            }
        }
        
        total += row_sum;
        
        /* Multiple exit points challenge scheduler */
        if (total > 1000.0f && i > size/2) {
            break;
        }
        
        if (i % 3 == 0) {
            continue;
        }
        
        total += 0.01f;
    }
    
    return total;
}

/* Computed goto pattern for complex control flow */
__attribute__((optimize("O2")))
static int computed_goto_test(int x) {
    static void* jump_table[] = {
        &&case_0, &&case_1, &&case_2, &&case_3, 
        &&case_4, &&case_5, &&case_6, &&case_7
    };
    
    int result = 0;
    int idx = x % 8;
    
    goto *jump_table[idx];
    
case_0:
    result = x + 1;
    /* Fall through */
case_1:
    result = result * 2;
    goto end;
case_2:
    result = x << 1;
    asm volatile("" ::: "rbx");
    goto end;
case_3:
    result = x / 2;
    goto end;
case_4:
    result = x | 0xFF;
    goto end;
case_5:
    result = x & 0x0F;
    /* Fall through */
case_6:
    result = result + 10;
    goto end;
case_7:
    result = x ^ 0x55;
    goto end;
    
end:
    return result;
}

/* Main test driver */
int main(void) {
    /* Allocate aligned memory for better vectorization */
    float* farr1 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* farr2 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* farr3 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    double* darr = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    int* iarr1 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* iarr2 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* iarr3 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* indices = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    float* matrix = (float*)aligned_alloc(64, 64 * 64 * sizeof(float));
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        farr1[i] = (float)i * 0.1f;
        farr2[i] = (float)i * 0.2f;
        farr3[i] = (float)i * 0.3f;
        darr[i] = (double)i * 0.01;
        iarr1[i] = i * 2;
        iarr2[i] = i * 3;
        iarr3[i] = 0;
        indices[i] = i * 7 % 13;
    }
    
    for (int i = 0; i < 64 * 64; i++) {
        matrix[i] = (float)(i % 100) * 0.01f;
    }
    
    float total_sum = 0.0f;
    double cold_result = 0.0;
    
    /* Run multiple iterations to ensure execution */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call hot function with complex scheduling */
        total_sum += hot_function(farr1, farr2, farr3, ARRAY_SIZE);
        
        /* Call cold function */
        cold_result += cold_function(darr, indices, ARRAY_SIZE);
        
        /* Vectorized loop test */
        vectorized_loop(iarr1, iarr2, iarr3, ARRAY_SIZE);
        
        /* Use computed results to prevent dead code elimination */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            farr1[i] += farr3[i] * 0.01f;
            darr[i] += (double)iarr3[i] * 0.001;
        }
        
        /* Nested loop test */
        total_sum += nested_loop_scheduler_test(matrix, 64);
        
        /* Computed goto test */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            iarr1[i] = computed_goto_test(iarr1[i]);
        }
    }
    
    /* Final computation to use all results */
    float final_result = total_sum + (float)cold_result;
    
    /* Print result to prevent optimization */
    printf("Final result: %f\n", final_result);
    
    /* Cleanup */
    free(farr1);
    free(farr2);
    free(farr3);
    free(darr);
    free(iarr1);
    free(iarr2);
    free(iarr3);
    free(indices);
    free(matrix);
    
    return 0;
}
