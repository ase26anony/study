/* ddg_coverage.c - Complex dependency patterns to exercise DDG edge creation */

#include <stdlib.h>
#include <stdio.h>

/* Global variables for function call dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Static helper function with side effects (Requirement 5) */
static void update_global(int* arr, int idx) {
    global_counter++;
    global_accumulator += arr[idx] * 0.5f;
    arr[idx] = global_counter % 100;
}

/* Another static function for output dependencies */
static inline float transform_value(float x, int scale) {
    static float last_result = 0.0f;
    float result = x * scale + last_result;
    last_result = result;  /* Creates output dependency through static variable */
    return result;
}

/* Non-linear index computation */
static int nonlinear_index(int i, int j) {
    return (i * i + j * j) % 256;  /* Non-affine access pattern */
}

int main(void) {
    /* Declare arrays with different data types (Requirement 6) */
    int int_arr[256];
    float float_arr[256];
    double double_arr[256];
    volatile int N = 256;  /* Prevent optimization of loop bounds */
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i;
        float_arr[i] = i * 0.5f;
        double_arr[i] = i * 0.25;
    }
    
    /* Non-linear index array (Requirement 4) */
    int index_map[256];
    for (int i = 0; i < N; i++) {
        index_map[i] = (i * 13 + 7) % N;  /* Pseudo-random but deterministic */
    }
    
    volatile int final_result = 0;
    float temp_float = 0.0f;
    int temp_int = 0;
    
    /* PRIMARY LOOP with complex dependencies */
    for (int i = 1; i < N; i++) {
        /* 1. LOOP-CARRIED TRUE DEPENDENCY (RAW) - Requirement 1 */
        int_arr[i] = int_arr[i-1] + i;  /* True dependency across iterations */
        
        /* Mixed data type dependency chain - Requirement 6 */
        float_arr[i] = float_arr[i-1] + int_arr[i] * 0.3f;
        double_arr[i] = double_arr[i-1] + float_arr[i] * 0.1;
        
        /* 2. ANTI-DEPENDENCY (WAR) - Requirement 2 */
        temp_int = int_arr[i];           /* Read */
        int_arr[i] = float_arr[i] > 0 ? temp_int * 2 : temp_int / 2;  /* Write later */
        
        /* 3. CONDITIONAL DEPENDENCY PATTERNS - Requirement 3 */
        if (i % 3 == 0) {
            /* Pattern A: More dependencies when i divisible by 3 */
            float old_val = float_arr[i];
            float_arr[i] = transform_value(old_val, i);
            temp_float = old_val + float_arr[i];  /* Use both old and new */
        } else if (i % 3 == 1) {
            /* Pattern B: Different dependency pattern */
            temp_float = float_arr[i];
            float_arr[i] = temp_float * temp_float - float_arr[i-1];
            /* Output dependency on temp_float */
            temp_float = float_arr[i] * 0.5f;  /* Overwrites temp_float */
        } else {
            /* Pattern C: Complex chain */
            double tmp_dbl = double_arr[i];
            double_arr[i] = tmp_dbl + double_arr[i-1] * 0.7;
            tmp_dbl = double_arr[i] * 2.0;  /* Output dependency */
            float_arr[i] = (float)(tmp_dbl + double_arr[i]);
        }
        
        /* 5. FUNCTION CALL WITH SIDE EFFECTS - Requirement 5 */
        update_global(int_arr, i % 128);
        
        /* 2. OUTPUT DEPENDENCY (WAW) - Requirement 2 */
        volatile int waw_var = i * 2;    /* First write */
        waw_var = global_counter + i;    /* Second write to same variable */
        
        /* Mixed operations maintaining dependency chain */
        final_result += int_arr[i] + (int)float_arr[i];
    }
    
    /* SECONDARY NESTED LOOP with non-linear accesses - Requirement 4 */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            int idx = nonlinear_index(i, j);
            
            /* Complex access pattern with dependencies */
            int_arr[idx] = int_arr[idx] + i - j;
            
            /* Anti-dependency in nested loop */
            float temp = float_arr[idx];
            float_arr[idx] = (float)(int_arr[idx]) * 0.25f;
            double_arr[idx] = temp + float_arr[idx];
            
            /* Cross-iteration dependency in inner loop */
            if (j > 0) {
                float_arr[idx] += float_arr[nonlinear_index(i, j-1)] * 0.1f;
            }
        }
    }
    
    /* Additional loop with pointer aliasing possibilities */
    int* ptr1 = &int_arr[0];
    int* ptr2 = &int_arr[128];
    for (int i = 0; i < 64; i++) {
        /* Potential pointer aliasing creates conservative dependencies */
        *ptr1 = *ptr1 + *ptr2;
        ptr1++;
        ptr2--;
        
        /* Volatile prevents certain optimizations */
        volatile int barrier = *ptr1;
        (void)barrier;  /* Use barrier to prevent dead code elimination */
    }
    
    /* Aggregate results to prevent optimization */
    float total_float = 0.0f;
    for (int i = 0; i < N; i++) {
        total_float += float_arr[index_map[i]];  /* Non-linear access */
        final_result += int_arr[i];
    }
    
    /* Use all computed values */
    final_result += (int)total_float + (int)global_accumulator + global_counter;
    
    printf("Result: %d\n", final_result);
    return final_result % 256;
}
