/* test_ddg_coverage.c
 * 
 * This program is designed to exercise the DDG edge creation logic
 * in GCC's instruction scheduler by creating various dependency patterns.
 * Compile with: gcc -O2 -fschedule-insns -fdump-rtl-sched1 -fdump-rtl-sched2 test_ddg_coverage.c -o test_ddg
 */

#include <stdlib.h>

/* Global variables for function call dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Helper function with side effects (Requirement 5) */
static void update_globals(int *arr, int idx, float *farr) {
    global_counter++;
    global_accumulator += farr[idx % 16];
    arr[idx % 16] = global_counter;
}

/* Another helper that creates register pressure */
static inline float float_transform(float x, int scale) {
    volatile float v = x; /* Prevent optimization */
    return (v * scale) / (scale + 1);
}

int main(void) {
    /* Declare arrays with different data types (Requirement 6) */
    int arr_int[256];
    float arr_float[256];
    double arr_double[256];
    int index_map[256]; /* For non-affine accesses */
    
    /* Volatile loop limit to prevent unrolling (Requirement 6) */
    volatile int N = 128;
    int i, j;
    volatile int result = 0;
    
    /* Initialize arrays */
    for (i = 0; i < 256; i++) {
        arr_int[i] = i;
        arr_float[i] = i * 0.5f;
        arr_double[i] = i * 0.25;
        /* Create non-linear index mapping (Requirement 4) */
        index_map[i] = (i * i + 7) % 256;
    }
    
    /* PRIMARY LOOP with multiple dependency patterns */
    for (i = 1; i < N; i++) {
        /* 1. LOOP-CARRIED TRUE DEPENDENCY (RAW) - Requirement 1 */
        arr_int[i] = arr_int[i-1] + 1;                /* Integer chain */
        arr_float[i] = arr_float[i-1] * 1.1f;         /* Float chain */
        
        /* 2. ANTI-DEPENDENCY (WAR) - Requirement 2 */
        int temp = arr_int[i];                        /* Read arr_int[i] */
        arr_int[i] = arr_int[index_map[i]] * 2;       /* Overwrite arr_int[i] */
        arr_double[i] = temp * 0.5;                   /* Use temp (anti-dep through temp) */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Requirement 2 */
        float f_temp = arr_float[i] + global_accumulator;
        f_temp = float_transform(f_temp, i);          /* Second write to f_temp */
        arr_float[i] = f_temp;
        
        /* 4. CONDITIONAL DEPENDENCIES - Requirement 3 */
        if (i % 3 == 0) {
            /* Path A: Creates dependency chain */
            arr_int[i] = arr_int[i] * arr_float[i];
            global_accumulator += arr_int[i];
        } else if (i % 3 == 1) {
            /* Path B: Different dependency pattern */
            arr_float[i] = arr_float[i] + arr_int[i];
            arr_int[i] = global_counter;
        } else {
            /* Path C: Mix dependencies */
            arr_double[i] = arr_double[i-1] + arr_float[i];
        }
        
        /* 5. FUNCTION CALL WITH SIDE EFFECTS - Requirement 5 */
        update_globals(arr_int, i, arr_float);
        
        /* 6. MIXED DATA TYPE OPERATIONS - Requirement 6 */
        double mixed_calc = arr_int[i] * 0.25 + arr_float[i];
        arr_double[i] = mixed_calc + arr_double[i-1];
    }
    
    /* SECONDARY LOOP: Non-affine array accesses - Requirement 4 */
    for (j = 0; j < N/2; j++) {
        int complex_idx = index_map[j];
        /* Multiple dependencies through non-affine index */
        arr_int[complex_idx] = arr_int[index_map[complex_idx]] + 
                               arr_float[j] * arr_double[complex_idx];
        
        /* Function call inside nested loop */
        if (j % 4 == 0) {
            update_globals(arr_int, complex_idx, arr_float);
        }
    }
    
    /* Aggregate results to prevent dead code elimination */
    for (i = 0; i < N; i++) {
        result += arr_int[i] + (int)arr_float[i] + (int)arr_double[i];
    }
    result += global_counter;
    
    return result;
}
