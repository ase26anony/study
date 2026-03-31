/* ddg_test.c - Test program to trigger DDG edge creation in GCC scheduler */
#include <stdlib.h>

/* Global variables for memory dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Static function with side effects (Requirement 5) */
static void modify_global(int *arr, int idx) {
    global_counter++;
    arr[idx] += global_counter;
    global_accumulator += (float)arr[idx];
}

/* Another static function for output dependencies */
static inline float compute_value(float a, float b, int scale) {
    static float last_result = 0.0f;  /* Static local creates memory dependencies */
    float result = a * b + (float)scale;
    last_result = result;  /* Output dependency on static variable */
    return result;
}

/* Non-linear index computation */
static int nonlinear_index(int i, int j) {
    return (i * i + j * j + i * j) % 256;  /* Non-affine expression */
}

int main(void) {
    /* Declare arrays with different data types (Requirement 6) */
    int int_arr[256];
    float float_arr[256];
    double double_arr[256];
    int index_map[256];
    
    /* Volatile to prevent optimization (Requirement 6) */
    volatile int N = 128;
    volatile int result = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        int_arr[i] = i;
        float_arr[i] = (float)i * 0.5f;
        double_arr[i] = (double)i * 0.25;
        index_map[i] = (i * 13 + 7) % 256;  /* Non-linear mapping */
    }
    
    /* PRIMARY LOOP with various dependencies */
    for (int i = 1; i < N; i++) {
        /* 1. Loop-carried true dependency (RAW) - Requirement 1 */
        int_arr[i] = int_arr[i-1] + i;  /* True dependency chain */
        
        /* 2. Anti-dependency (WAR) - Requirement 2 */
        int tmp = int_arr[i];           /* Read */
        int_arr[i] = float_arr[i] > 0 ? tmp * 2 : tmp / 2;  /* Later write - WAR */
        
        /* 3. Output dependency (WAW) - Requirement 2 */
        float f1 = compute_value(float_arr[i], 2.0f, i);
        float f1_alt = compute_value(float_arr[i-1], 3.0f, i+1);
        f1 = f1_alt;  /* WAW on f1 */
        
        /* 4. Conditional dependency patterns - Requirement 3 */
        if (i % 3 == 0) {
            /* Pattern A: Cross iteration dependency */
            float_arr[i] = float_arr[i-1] * 1.1f;
            double_arr[i] = double_arr[i] + (double)int_arr[i];
        } else if (i % 3 == 1) {
            /* Pattern B: Different dependency chain */
            float_arr[i] = (float)int_arr[i] / 3.0f;
            double_arr[i] = double_arr[i-1] * 0.9;
        } else {
            /* Pattern C: Complex mixed dependencies */
            float tmp_f = float_arr[i];
            float_arr[i] = (float)(int_arr[i] + int_arr[i-1]) * 0.5f;
            int_arr[i] = (int)tmp_f;  /* Anti-dependency through type conversion */
        }
        
        /* 5. Function call with side effects - Requirement 5 */
        modify_global(int_arr, i % 128);
        
        /* 6. Mixed data type operations - Requirement 6 */
        volatile int mixed_calc = int_arr[i] + (int)float_arr[i];
        float_arr[i] = float_arr[i] + (float)mixed_calc * 0.01f;
    }
    
    /* SECONDARY LOOP with non-linear array accesses - Requirement 4 */
    for (int i = 0; i < N/2; i++) {
        for (int j = 0; j < 8; j++) {
            /* Non-linear index calculation */
            int idx = nonlinear_index(i, j);
            
            /* Access arrays with non-affine indices */
            int_arr[idx] += index_map[i] + j;
            float_arr[idx] += (float)int_arr[index_map[j]] * 0.5f;
            
            /* Additional dependency chain */
            if (j > 0) {
                double_arr[idx] = double_arr[nonlinear_index(i, j-1)] * 1.01;
            }
        }
    }
    
    /* Aggregate results to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        result += int_arr[i] + (int)float_arr[i] + (int)double_arr[i];
    }
    
    /* Add global state to result */
    result += global_counter + (int)global_accumulator;
    
    return result;
}
