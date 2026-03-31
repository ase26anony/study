/* test_ddg_edges.c
 * Program designed to trigger DDG edge creation in GCC's instruction scheduler
 * Specifically targets lines 749-757 in ddg.cc
 */

#include <stdlib.h>
#include <stdio.h>

/* Global variables for function call dependencies (Requirement 5) */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Static helper function with side effects (Requirement 5) */
static void update_global(int idx, float* arr) {
    global_counter += idx;
    global_accumulator += arr[idx % 8] * 0.5f;
    /* Memory barrier effect */
    asm volatile("" ::: "memory");
}

/* Another static function for output dependencies */
static inline float transform_value(float x, int scale) {
    float result = x * scale;
    /* Create register pressure */
    result = result / (scale + 1);
    return result;
}

int main(void) {
    const int N = 1024;
    volatile int limit = N;  /* Prevent optimization (Requirement 6) */
    
    /* Arrays with different data types */
    int int_arr[N];
    float float_arr[N];
    double double_arr[N];
    
    /* Non-linear index array (Requirement 4) */
    int nonlin_idx[16];
    for (int i = 0; i < 16; i++) {
        nonlin_idx[i] = (i * i + i * 3) % 16;  /* Non-affine pattern */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i;
        float_arr[i] = i * 0.5f;
        double_arr[i] = i * 0.25;
    }
    
    /* Primary loop with multiple dependency types */
    for (int i = 1; i < limit; i++) {
        /* 1. LOOP-CARRIED TRUE DEPENDENCY (RAW) - Requirement 1 */
        int_arr[i] = int_arr[i-1] + i;  /* Integer chain */
        float_arr[i] = float_arr[i-1] * 1.01f;  /* Float chain */
        
        /* 2. ANTI-DEPENDENCY (WAR) - Requirement 2 */
        int tmp_int = int_arr[i];      /* Read */
        float tmp_float = float_arr[i]; /* Read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Requirement 2 */
        double tmp_double = double_arr[i] * 2.0;
        tmp_double = transform_value(tmp_double, i);  /* Overwrite - WAW */
        
        /* 4. CONDITIONAL DEPENDENCIES - Requirement 3 */
        if (i % 3 == 0) {
            /* Path A: Different dependency pattern */
            int_arr[i] = tmp_int * 2;  /* WAW on int_arr[i] */
            float_arr[i] = tmp_float + global_accumulator;
        } else if (i % 3 == 1) {
            /* Path B: Alternative pattern with anti-dependency */
            tmp_int = float_arr[i];  /* WAR - reusing tmp_int */
            int_arr[i] = tmp_int + global_counter;
        } else {
            /* Path C: Complex chain */
            tmp_float = int_arr[i] * 0.7f;
            float_arr[i] = tmp_float * tmp_float;
        }
        
        /* 5. FUNCTION CALL WITH SIDE EFFECTS - Requirement 5 */
        update_global(i, float_arr);
        
        /* 6. MIXED DATA TYPE OPERATIONS - Requirement 6 */
        volatile int volatile_var = i % 64;  /* Prevent optimization */
        
        /* Mixed-type computation chain */
        float mixed_result = int_arr[i] * 0.3f + float_arr[i];
        double_arr[i] = mixed_result * 1.5 + volatile_var;
        
        /* Create register pressure with different types */
        int int_temp = int_arr[i] * 3;
        float float_temp = float_arr[i] / 2.0f;
        double double_temp = double_arr[i] + int_temp - float_temp;
        
        /* Final WAW on double_arr[i] */
        double_arr[i] = double_temp * (i % 10 + 1);
    }
    
    /* Nested loop with non-linear array access - Requirement 4 */
    int sum_nonlin = 0;
    for (int i = 0; i < limit - 16; i += 8) {
        for (int j = 0; j < 16; j++) {
            /* Non-affine access pattern */
            int idx = nonlin_idx[j] + i;
            if (idx < N) {
                /* Complex dependency chain */
                int_arr[idx] += int_arr[nonlin_idx[(j + 1) % 16] + i];
                sum_nonlin += int_arr[idx];
            }
        }
        
        /* Additional mixed operations in nested loop */
        float nested_float = 0.0f;
        for (int k = 0; k < 4; k++) {
            nested_float += float_arr[i + k] * (k + 1);
        }
        float_arr[i] = nested_float;  /* WAW */
    }
    
    /* Aggregate results to prevent dead code elimination */
    volatile int final_result = 0;
    volatile float final_float = 0.0f;
    
    for (int i = 0; i < limit; i++) {
        final_result += int_arr[i];
        final_float += float_arr[i];
        final_result += (int)double_arr[i];
    }
    
    final_result += global_counter;
    final_result += (int)global_accumulator;
    final_result += sum_nonlin;
    
    printf("Result: %d (float: %.2f)\n", final_result, final_float);
    
    return final_result > 0 ? 0 : 1;
}
