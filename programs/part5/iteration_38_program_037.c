/* ddg_coverage.c
 * Program designed to exercise DDG edge creation in GCC's instruction scheduler
 * Compile with: gcc -O2 -fschedule-insns -fdump-rtl-sched1 -fdump-rtl-sched2 ddg_coverage.c -o ddg_coverage
 * Or for modulo scheduling: gcc -O3 -fmodulo-sched -fdump-rtl-sms ddg_coverage.c -o ddg_coverage
 */

#include <stdlib.h>
#include <stdio.h>

/* Global variables for function call dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Static helper function with side effects (Requirement 5) */
static inline void update_globals(int* ptr, float* fptr) {
    *ptr += global_counter++;
    *fptr += (float)global_counter * 0.5f;
    global_accumulator += *fptr;
}

/* Another static function for output dependencies */
static inline double compute_value(int i, double base) {
    static double last_result = 0.0;
    double result = base * i + last_result * 0.1;
    last_result = result;  /* Creates output dependency through static variable */
    return result;
}

int main(void) {
    /* Declare arrays with different data types (Requirement 6) */
    const int N = 1024;
    volatile int limit = N;  /* Volatile to prevent optimization */
    int int_arr[N];
    float float_arr[N];
    double double_arr[N];
    int index_map[N];  /* For non-affine accesses */
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i;
        float_arr[i] = i * 0.5f;
        double_arr[i] = i * 0.25;
        /* Create non-linear index mapping (Requirement 4) */
        index_map[i] = (i * i) % N;  /* Non-affine access pattern */
    }
    
    /* Primary loop with loop-carried dependency (Requirement 1) */
    for (int i = 1; i < limit; i++) {
        /* TRUE DEPENDENCY (RAW): Loop-carried through int_arr */
        int_arr[i] = int_arr[i-1] + i;  /* Line 1: Read i-1, write i */
        
        /* Conditional creating different dependency patterns (Requirement 3) */
        if (i % 3 == 0) {
            /* Path A: Anti-dependency pattern (WAR) */
            int temp = int_arr[i];           /* Read int_arr[i] */
            int_arr[i] = float_arr[i] * 2;   /* Write int_arr[i] - anti-dep on temp */
            float_arr[i] = temp * 0.5f;      /* Write float_arr[i] */
            
            /* Output dependency (WAW) on local variable */
            double local_var = compute_value(i, 1.0);
            local_var = local_var * 2.0;     /* WAW on local_var */
            double_arr[i] = local_var;
        } else if (i % 3 == 1) {
            /* Path B: Different true dependency chain */
            float_arr[i] = float_arr[i-1] + int_arr[i] * 0.3f;
            
            /* Anti-dependency through array aliasing */
            int tmp = int_arr[index_map[i]];      /* Read via non-affine index */
            int_arr[index_map[i]] = i * 2;        /* Write - anti-dep on tmp */
            double_arr[i] = tmp * 0.25;
        } else {
            /* Path C: Complex mixed dependencies */
            /* Function call creating memory dependencies (Requirement 5) */
            update_globals(&int_arr[i], &float_arr[i]);
            
            /* Output dependency in same iteration */
            double_arr[i] = compute_value(i, 2.0);
            double_arr[i] = compute_value(i, 3.0);  /* WAW on double_arr[i] */
        }
        
        /* Mixed data type operations (Requirement 6) */
        float mixed_result = int_arr[i] * 0.7f + float_arr[i];
        double_arr[i] += (double)mixed_result * 1.5;
        
        /* Anti-dependency with register reuse (Requirement 2) */
        {
            volatile int reuse_var = int_arr[i];  /* Read */
            /* Various operations... */
            reuse_var = float_arr[i] > 0 ? 1 : 0;  /* Write - anti-dep */
            int_arr[i] = reuse_var * 10;
        }
    }
    
    /* Nested loop with non-affine array accesses (Requirement 4) */
    for (int i = 0; i < limit/2; i++) {
        for (int j = 0; j < 4; j++) {
            /* Access using non-linear function of indices */
            int idx = index_map[(i * 7 + j * 3) % N];
            float_arr[idx] = float_arr[idx] * 1.1f + i * 0.01f;
            
            /* Loop-carried dependency in inner loop */
            if (j > 0) {
                double_arr[idx] = double_arr[idx] + double_arr[index_map[(i * 7 + (j-1) * 3) % N]] * 0.5;
            }
        }
    }
    
    /* Another loop with output dependencies */
    for (int i = 0; i < limit; i += 2) {
        /* Multiple writes to same location */
        int output_var = i * 2;
        output_var = output_var + 1;      /* WAW */
        output_var = output_var * 3;      /* WAW */
        int_arr[i] = output_var;
        
        /* Function call creating memory deps */
        update_globals(&int_arr[i], &float_arr[i]);
    }
    
    /* Aggregate results to prevent dead code elimination */
    volatile int final_result = 0;
    volatile float final_float = 0.0f;
    volatile double final_double = 0.0;
    
    for (int i = 0; i < limit; i++) {
        final_result += int_arr[i];
        final_float += float_arr[i];
        final_double += double_arr[i];
    }
    
    /* Use results to prevent optimization */
    printf("Results: %d %f %lf\n", final_result, final_float, final_double);
    
    return final_result > 0 ? 0 : 1;
}
