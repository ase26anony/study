/* ddg_edge_coverage.c
 * Program designed to exercise DDG edge creation in GCC's instruction scheduler
 * Compile with: gcc -O2 -fschedule-insns -fdump-rtl-sched1 -fdump-rtl-sched2 ddg_edge_coverage.c -o ddg_test
 * Or for modulo scheduling: gcc -O3 -fmodulo-sched -fdump-rtl-sms ddg_edge_coverage.c -o ddg_test
 */

#include <stdlib.h>
#include <stdio.h>

/* Global variables for memory dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Static function with side effects (Requirement 5) */
static void update_global(int* arr, int idx) {
    global_counter++;
    arr[idx] += global_counter;
    global_accumulator += (float)arr[idx];
}

/* Another static function for different data type operations */
static double process_value(double x, int y) {
    volatile double result = x * (double)y + 0.5;
    return result;
}

int main(void) {
    const int N = 1024;
    volatile int limit = N;  /* Prevent optimization */
    
    /* Arrays with different data types (Requirement 6) */
    int int_arr[N];
    float float_arr[N];
    double double_arr[N];
    
    /* Non-linear index array (Requirement 4) */
    int nonlin_idx[N];
    for (int i = 0; i < N; i++) {
        nonlin_idx[i] = (i * i + i * 3) % N;  /* Non-affine index calculation */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i;
        float_arr[i] = (float)i * 0.5f;
        double_arr[i] = (double)i * 0.25;
    }
    
    /* Primary loop with multiple dependency types */
    int tmp_storage = 0;
    float float_tmp = 0.0f;
    volatile int final_result = 0;
    
    for (int i = 1; i < limit; i++) {
        /* 1. LOOP-CARRIED TRUE DEPENDENCY (RAW) - Requirement 1 */
        int_arr[i] = int_arr[i-1] + 1;  /* True dependency across iterations */
        
        /* Mixed with floating point chain */
        float_arr[i] = float_arr[i-1] * 1.1f + (float)int_arr[i];
        
        /* 2. ANTI-DEPENDENCY (WAR) - Requirement 2 */
        int tmp_read = int_arr[i];      /* Read */
        int_arr[i] = float_arr[i] > 10.0f ? tmp_read * 2 : tmp_read / 2; /* Write later */
        
        /* 3. CONDITIONAL DEPENDENCIES - Requirement 3 */
        if (i % 3 == 0) {
            /* One dependency pattern */
            float_tmp = float_arr[i];
            float_arr[i] = float_tmp * 2.0f;
            double_arr[i] = process_value(double_arr[i-1], i);
        } else if (i % 3 == 1) {
            /* Different pattern with output dependency (WAW) */
            double local_var = (double)int_arr[i];
            local_var = local_var * 3.14;  /* WAW on local_var */
            double_arr[i] = local_var;
            
            /* Anti-dependency on array */
            float old_val = float_arr[i];
            float_arr[i] = (float)local_var;
            float_tmp = old_val;  /* Use old value */
        } else {
            /* Third pattern with both RAW and WAR */
            tmp_storage = int_arr[i] + int_arr[i-1];
            int_arr[i] = tmp_storage - 1;  /* WAR on int_arr[i] */
        }
        
        /* 5. FUNCTION CALL WITH SIDE EFFECTS - Requirement 5 */
        update_global(int_arr, i % 100);
        
        /* 6. MIXED DATA TYPE OPERATIONS - Requirement 6 */
        double complex_val = (double)int_arr[i] * 0.5 + 
                            (double)float_arr[i] * 0.3 + 
                            double_arr[i];
        
        /* Output dependency on local variable */
        complex_val = complex_val + (double)global_counter;  /* WAW */
        
        /* Store result back with non-linear index */
        if (i < N-1) {
            double_arr[nonlin_idx[i]] = complex_val;
        }
    }
    
    /* Nested loop with non-linear array access (Requirement 4) */
    double nested_accum = 0.0;
    for (int i = 0; i < limit/2; i++) {
        for (int j = 0; j < 4; j++) {
            /* Non-affine index access */
            int idx = nonlin_idx[i] + j * j;
            if (idx < N) {
                nested_accum += double_arr[idx];
                
                /* Create anti-dependency in nested loop */
                float old_float = float_arr[idx];
                float_arr[idx] = (float)nested_accum;
                float_tmp = old_float;  /* Use old value */
            }
        }
    }
    
    /* Additional loop with output dependencies */
    for (int i = 0; i < limit; i += 2) {
        /* Multiple writes to same location (WAW) */
        int output_var = int_arr[i] * 2;
        output_var = output_var + 1;  /* Second write to output_var */
        output_var = output_var * 3;   /* Third write to output_var */
        
        int_arr[i] = output_var;
        
        /* Function call creating memory dependencies */
        update_global(int_arr, i % 50);
    }
    
    /* Aggregate results to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        final_result += int_arr[i];
        final_result += (int)float_arr[i];
        if (i % 8 == 0) {
            final_result += (int)double_arr[i];
        }
    }
    
    final_result += (int)nested_accum;
    final_result += global_counter;
    final_result += (int)global_accumulator;
    
    printf("Result: %d\n", final_result);
    return final_result > 0 ? 0 : 1;
}
