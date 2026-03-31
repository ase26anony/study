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
    global_accumulator += arr[idx] * 0.5f;
}

/* Another static function for output dependencies */
static inline float transform_value(float x, int scale) {
    static float last_result = 0.0f;
    float result = x * scale + last_result;
    last_result = result;  /* Creates output dependency between calls */
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
        nonlin_idx[i] = (i * i + i * 3 + 7) % N;  /* Quadratic index */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i;
        float_arr[i] = i * 0.5f;
        double_arr[i] = i * 0.25;
    }
    
    volatile int final_result = 0;
    
    /* PRIMARY LOOP with multiple dependency types */
    for (int i = 1; i < limit; i++) {
        /* 1. LOOP-CARRIED TRUE DEPENDENCY (RAW) - Requirement 1 */
        int_arr[i] = int_arr[i-1] + 1;  /* True dependency chain */
        
        /* Mixed data type operations - Requirement 6 */
        float temp_float = float_arr[i-1] * 2.0f;
        
        /* 2. ANTI-DEPENDENCY (WAR) - Requirement 2 */
        int tmp = int_arr[i];           /* Read */
        int_arr[i] = float_arr[i] > 0 ? tmp + 1 : tmp - 1;  /* Write later - anti-dependency */
        
        /* 3. CONDITIONAL DEPENDENCIES - Requirement 3 */
        if (i % 3 == 0) {
            /* Path A: Different dependency pattern */
            float_arr[i] = float_arr[i-1] + temp_float;
            double_arr[i] = double_arr[i-1] * 1.1;
        } else if (i % 3 == 1) {
            /* Path B: Alternative pattern with output dependency */
            float x = transform_value(float_arr[i], i);  /* First write to x */
            x = x * 0.8f + global_accumulator;           /* Second write to x - output dependency */
            float_arr[i] = x;
        } else {
            /* Path C: Anti-dependency pattern */
            float old_val = float_arr[i];                /* Read */
            float_arr[i] = int_arr[i] * 0.7f;            /* Write - anti-dependency */
            double_arr[i] = old_val + double_arr[i-1];
        }
        
        /* 5. FUNCTION CALL WITH SIDE EFFECTS - Requirement 5 */
        update_global(int_arr, i % 100);
        
        /* 2. OUTPUT DEPENDENCY (WAW) - Requirement 2 */
        double local_var = double_arr[i];
        local_var = local_var * local_var;  /* Output dependency on local_var */
        if (i % 5 == 0) {
            local_var = sqrt(local_var);    /* Another write - output dependency */
        }
        double_arr[i] = local_var;
    }
    
    /* SECONDARY LOOP with non-linear array access - Requirement 4 */
    for (int i = 0; i < limit / 2; i++) {
        int idx = nonlin_idx[i];  /* Non-affine index access */
        
        /* Complex dependency chain with non-linear access */
        float_arr[idx] = float_arr[nonlin_idx[(i + 1) % N]] * 1.5f;
        
        /* Loop-carried dependency through non-linear indices */
        if (i > 0) {
            int_arr[nonlin_idx[i]] = int_arr[nonlin_idx[i-1]] + 
                                     float_arr[nonlin_idx[i]] * 2;
        }
        
        /* Call side-effect function */
        update_global(int_arr, idx % 50);
    }
    
    /* NESTED LOOP for additional DDG complexity */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* Multi-dimensional like access pattern */
            int index = i * 32 + j * 3;
            if (index < N) {
                /* Cross-iteration dependency in inner loop */
                float_arr[index] = float_arr[(index + N - 1) % N] * 
                                   (i + j) * 0.1f;
                
                /* Output dependency in inner loop */
                double temp = double_arr[index];
                temp = temp + int_arr[index];
                temp = temp * 0.9;  /* Multiple writes - output dependency */
                double_arr[index] = temp;
            }
        }
    }
    
    /* Aggregate results to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        final_result += int_arr[i];
        final_result += (int)float_arr[i];
        final_result += (int)double_arr[i];
    }
    
    final_result += global_counter;
    
    printf("Result: %d\n", final_result);
    return final_result % 256;
}
