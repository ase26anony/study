/* ddg_coverage.c - Program to exercise DDG edge creation in GCC scheduler */
#include <stdlib.h>

/* Global variables for function call dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Static function with side effects (Requirement 5) */
static void update_global(int *arr, int idx, float *farr) {
    global_counter++;
    *arr += global_counter;
    global_accumulator += *farr;
    *farr = global_accumulator * 0.5f;
}

/* Another static function for output dependencies */
static inline float compute_value(float a, float b) {
    volatile float result; /* Prevent optimization */
    result = a * b + (float)global_counter;
    return result;
}

int main(void) {
    const int N = 100;
    volatile int limit = N; /* Prevent constant propagation */
    
    /* Arrays with different data types (Requirement 6) */
    int int_arr[N];
    float float_arr[N];
    double double_arr[N];
    int results[N];
    
    /* Non-linear index array (Requirement 4) */
    int nonlin_idx[N];
    for (int i = 0; i < N; i++) {
        nonlin_idx[i] = (i * i + i * 3) % N; /* Non-affine index calculation */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i;
        float_arr[i] = i * 1.5f;
        double_arr[i] = i * 2.5;
        results[i] = 0;
    }
    
    /* Primary loop with multiple dependency types */
    for (int i = 1; i < limit; i++) {
        /* 1. Loop-carried true dependency (RAW) - Requirement 1 */
        int_arr[i] = int_arr[i-1] + 1; /* True dependency chain */
        
        /* 2. Anti-dependency (WAR) - Requirement 2 */
        int temp = int_arr[i];        /* Read */
        int_arr[i] = float_arr[i] > 0 ? 1 : 0; /* Write later - creates WAR */
        results[i] = temp;            /* Use the read value */
        
        /* 3. Output dependency (WAW) - Requirement 2 */
        float fval = compute_value(float_arr[i], 2.0f);
        fval = compute_value(fval, 3.0f); /* Second write to fval - WAW */
        float_arr[i] = fval;
        
        /* 4. Conditional with different dependency patterns - Requirement 3 */
        if (i % 3 == 0) {
            /* Pattern A: More complex dependency chain */
            double_arr[i] = double_arr[i-1] * 1.1;
            int_arr[i] = (int)(double_arr[i]) + int_arr[i];
        } else if (i % 3 == 1) {
            /* Pattern B: Different dependency pattern */
            int_arr[i] = int_arr[i] * 2 - int_arr[i-1];
            double_arr[i] = (double)int_arr[i] / 3.0;
        } else {
            /* Pattern C: Cross-type dependencies */
            float_arr[i] = (float)int_arr[i] * 0.7f + float_arr[i-1];
            double_arr[i] = (double)float_arr[i] * 1.3;
        }
        
        /* 5. Function call with side effects - Requirement 5 */
        update_global(&int_arr[i], i, &float_arr[i]);
        
        /* 6. Mixed data type operations - Requirement 6 */
        float mixed_result = (float)int_arr[i] * 1.7f;
        double_arr[i] = (double)mixed_result + double_arr[i-1];
        int_arr[i] = (int)(mixed_result) | int_arr[i];
    }
    
    /* Nested loop with non-linear array access - Requirement 4 */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 5; j++) {
            int idx = nonlin_idx[(i + j) % N]; /* Non-affine access */
            float_arr[idx] = float_arr[idx] * 1.1f + (float)j;
            
            /* Create output dependency in inner loop */
            double tmp = double_arr[idx];
            tmp = tmp * 2.0; /* WAW on tmp */
            double_arr[idx] = tmp;
        }
    }
    
    /* Additional anti-dependency example with array reuse */
    for (int i = 0; i < N-1; i++) {
        /* Read from arr[i+1], write to arr[i] - creates WAR */
        int read_val = int_arr[i+1];
        int_arr[i] = read_val * 2; /* Anti-dependency if same memory? Actually different indices */
        
        /* Real anti-dependency on same location using temp array */
        float temp = float_arr[i];
        float_arr[i] = compute_value(temp, 2.0f); /* WAR: temp read, then float_arr[i] written */
        results[i] = (int)temp;
    }
    
    /* Final aggregation to prevent dead code elimination */
    volatile int final_result = 0;
    volatile float final_float = 0.0f;
    volatile double final_double = 0.0;
    
    for (int i = 0; i < N; i++) {
        final_result += int_arr[i] + results[i];
        final_float += float_arr[i];
        final_double += double_arr[i];
    }
    
    /* Use all results to prevent optimization */
    return (int)(final_result + final_float + final_double) % 256;
}
