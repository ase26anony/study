/* ddg_coverage.c
 * Program designed to exercise DDG edge creation in GCC's instruction scheduler
 * Compile with: gcc -O2 -fschedule-insns -fdump-rtl-sched1 -fdump-rtl-sched2 ddg_coverage.c -o ddg_coverage
 * Or for modulo scheduling: gcc -O3 -fmodulo-sched -fdump-rtl-sms ddg_coverage.c -o ddg_coverage
 */

#include <stdlib.h>
#include <stdio.h>

/* Global variables for creating memory dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Static function with side effects (Requirement 5) */
static void update_global(int *arr, int idx) {
    global_counter++;
    arr[idx] += global_counter;
    global_accumulator += (float)arr[idx] * 0.5f;
}

/* Another static function for anti-dependency patterns */
static float process_value(float x, float y) {
    volatile float tmp = x;  /* Prevent optimization */
    x = y * 2.0f;           /* WAR: x is written after tmp reads it */
    y = tmp + x;            /* RAW: y reads x after it was written */
    return y;
}

int main(void) {
    const int N = 256;
    volatile int limit = N;  /* Volatile to prevent optimization */
    
    /* Arrays with different data types (Requirement 6) */
    int int_arr[N];
    float float_arr[N];
    double double_arr[N];
    
    /* Non-linear index array (Requirement 4) */
    int nonlin_idx[N];
    for (int i = 0; i < N; i++) {
        nonlin_idx[i] = (i * i + i * 3 + 7) % N;  /* Quadratic mapping */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i;
        float_arr[i] = (float)i * 0.5f;
        double_arr[i] = (double)i * 0.25;
    }
    
    int prev_int = 0;
    float prev_float = 0.0f;
    
    /* Primary loop with complex dependencies */
    for (int i = 0; i < limit; i++) {
        /* Requirement 1: Loop-carried true dependencies (RAW) */
        int_arr[i] = prev_int + int_arr[i];  /* RAW on prev_int */
        prev_int = int_arr[i];               /* RAW on int_arr[i] */
        
        float_arr[i] = prev_float * 1.1f;    /* RAW on prev_float */
        prev_float = float_arr[i];           /* RAW on float_arr[i] */
        
        /* Requirement 2: Anti-dependencies (WAR) and Output dependencies (WAW) */
        float tmp = float_arr[i];            /* Read float_arr[i] */
        float_arr[i] = (float)int_arr[i];    /* WAR: Write to float_arr[i] after reading */
        double tmp2 = double_arr[i];         /* Read double_arr[i] */
        
        /* Output dependency (WAW) on local variable */
        tmp = process_value(tmp, float_arr[i]);  /* WAW: tmp written twice */
        tmp = tmp * 2.0f;                        /* Another WAW on tmp */
        
        /* Requirement 3: Control flow with different dependency patterns */
        if (i % 3 == 0) {
            /* Pattern A: Chain of dependencies */
            int x = int_arr[i];
            x = x * 2 + 1;          /* WAW on x */
            int_arr[i] = x;         /* WAR: int_arr[i] written after being read earlier */
            
            /* Call function with side effects */
            update_global(int_arr, i);
        } else if (i % 3 == 1) {
            /* Pattern B: Different dependency structure */
            float local = float_arr[i];
            float_arr[i] = local + (float)int_arr[i];  /* RAW on int_arr[i], WAR on float_arr[i] */
            local = float_arr[i] * 0.5f;               /* RAW on float_arr[i], WAW on local */
        } else {
            /* Pattern C: More complex with mixed types */
            double d = (double)int_arr[i] + double_arr[i];
            int_arr[i] = (int)d;                       /* WAW on int_arr[i] */
            double_arr[i] = d * 0.75;                  /* WAW on double_arr[i] */
        }
        
        /* Requirement 5: Function call with side effects */
        update_global(int_arr, i % N);
        
        /* Requirement 6: Mixed data type operations */
        double_arr[i] = (double)int_arr[i] * 0.33 + (double)float_arr[i];
        
        /* Create register pressure to force spills and reloads */
        int r1 = int_arr[i];
        int r2 = r1 * 2;
        int r3 = r2 + r1;
        int r4 = r3 - i;
        int r5 = r4 * 3;
        int_arr[i] = r5;  /* Final WAW */
    }
    
    /* Nested loop with non-linear array access (Requirement 4) */
    for (int i = 0; i < N; i++) {
        int idx = nonlin_idx[i];
        /* Complex addressing creates memory dependencies */
        float_arr[idx] = float_arr[idx] + (float)int_arr[i];
        
        /* Chain of dependencies with non-linear access */
        if (idx > 0) {
            float_arr[idx] = float_arr[idx-1] * 1.5f;  /* RAW with non-adjacent index */
        }
    }
    
    /* Another loop with stride to create distance > 1 dependencies */
    for (int i = 2; i < N; i++) {
        /* Distance-2 dependency */
        int_arr[i] = int_arr[i-2] * 3;  /* RAW with distance 2 */
    }
    
    /* Final aggregation to prevent dead code elimination */
    volatile int final_result = 0;
    volatile float final_float = 0.0f;
    volatile double final_double = 0.0;
    
    for (int i = 0; i < N; i++) {
        final_result += int_arr[i];
        final_float += float_arr[i];
        final_double += double_arr[i];
    }
    
    /* Use results to prevent optimization */
    printf("Results: %d %f %f\n", final_result, final_float, final_double);
    
    return final_result > 0 ? 0 : 1;
}
