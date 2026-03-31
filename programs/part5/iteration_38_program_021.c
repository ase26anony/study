/* test_ddg_edges.c
 * Compile with: gcc -O2 -fschedule-insns -fmodulo-sched test_ddg_edges.c -o test_ddg
 */

#include <stdint.h>
#include <stdlib.h>

/* Global variables for function call dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Helper function with side effects (Requirement 5) */
static inline void update_globals(int* arr, int idx) {
    global_counter++;
    arr[idx] += global_counter;
    global_accumulator += (float)arr[idx];
}

/* Another helper with output dependency potential */
static inline double transform_value(double x, int i) {
    static double last_result = 0.0;
    double result;
    
    /* Create output dependency on last_result */
    if (i % 3 == 0) {
        result = x * 2.0;
    } else if (i % 3 == 1) {
        result = x + last_result;
    } else {
        result = x / 2.0;
    }
    
    last_result = result;  /* WAW dependency through static variable */
    return result;
}

int main(void) {
    const int N = 1000;
    volatile int limit = N;  /* Prevent optimization */
    
    /* Arrays with different data types (Requirement 6) */
    int* int_arr = (int*)malloc(N * sizeof(int));
    float* float_arr = (float*)malloc(N * sizeof(float));
    double* double_arr = (double*)malloc(N * sizeof(double));
    
    /* Non-linear index array (Requirement 4) */
    int* idx_map = (int*)malloc(N * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i;
        float_arr[i] = (float)i * 0.5f;
        double_arr[i] = (double)i * 0.25;
        
        /* Create non-affine index pattern: quadratic modulo N */
        idx_map[i] = (i * i + i * 3 + 7) % N;
    }
    
    /* Primary loop with multiple dependency types */
    int tmp_storage = 0;
    float float_tmp = 0.0f;
    volatile int result = 0;  /* Prevent dead code elimination */
    
    /* Loop-carried true dependency (Requirement 1) */
    for (int i = 1; i < limit; i++) {
        /* TRUE dependency (RAW): arr[i] depends on arr[i-1] */
        int_arr[i] = int_arr[i-1] + i;
        
        /* Anti-dependency (WAR) on local variable (Requirement 2) */
        tmp_storage = int_arr[i];          /* Read int_arr[i] */
        int_arr[i] = float_arr[i] * 2.0f;  /* Write int_arr[i] - WAR with previous read */
        
        /* Output dependency (WAW) on float_tmp */
        float_tmp = float_arr[i] * 3.14f;
        float_tmp = sinf(float_arr[i]);    /* WAW on float_tmp */
        
        /* Conditional dependency patterns (Requirement 3) */
        if (i % 2 == 0) {
            /* Pattern A: Chain of dependencies */
            double_arr[i] = double_arr[i-1] * 1.1;
            float_arr[i] = (float)double_arr[i] + float_arr[i-1];
        } else {
            /* Pattern B: Different dependency structure */
            float_arr[i] = float_arr[i-1] * 0.9f;
            double_arr[i] = (double)float_arr[i] / 2.0;
            
            /* Anti-dependency within else branch */
            int old_val = int_arr[i];
            int_arr[i] = i * 2;
            tmp_storage = old_val + int_arr[i];  /* Use after redefinition */
        }
        
        /* Function call with side effects (Requirement 5) */
        update_globals(int_arr, i % 100);
        
        /* Mixed data type operations (Requirement 6) */
        double transformed = transform_value(double_arr[i], i);
        float_arr[i] = (float)transformed + global_accumulator;
        
        /* Complex expression with multiple dependencies */
        int_arr[i] = (int)(float_arr[i] * 2.0f) + int_arr[i-1] + tmp_storage;
    }
    
    /* Nested loop with non-affine array access (Requirement 4) */
    for (int i = 0; i < limit/2; i++) {
        for (int j = 0; j < 4; j++) {
            /* Access using non-linear indices */
            int idx = idx_map[i] + j * 7;
            if (idx < N) {
                /* Create dependencies through non-affine access */
                float_arr[idx] = float_arr[idx] * 2.0f + (float)int_arr[i];
                
                /* Potential loop-carried dependency through idx_map */
                if (i > 0) {
                    int prev_idx = idx_map[i-1] + j * 7;
                    if (prev_idx < N) {
                        double_arr[idx] = double_arr[prev_idx] * 0.95;
                    }
                }
            }
        }
    }
    
    /* Additional loop with output dependencies */
    int output_var = 0;
    for (int i = 0; i < limit; i++) {
        /* Multiple writes to same variable - WAW dependencies */
        output_var = int_arr[i] * 2;
        output_var = output_var + float_arr[i];  /* WAW on output_var */
        output_var = (int)(double_arr[i] * 3.0); /* Another WAW */
        
        /* Use result to prevent elimination */
        result += output_var;
    }
    
    /* Cleanup and return */
    free(int_arr);
    free(float_arr);
    free(double_arr);
    free(idx_map);
    
    return result % 256;  /* Prevent tail call optimization */
}
