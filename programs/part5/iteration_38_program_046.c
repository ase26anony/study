/* ddg_coverage.c
 * Program designed to exercise Data Dependency Graph edge creation
 * Compile with: gcc -O2 -fschedule-insns -fdump-rtl-sched1 -fdump-rtl-sched2 ddg_coverage.c -o ddg_coverage
 * Or for modulo scheduling: gcc -O3 -fmodulo-sched -fdump-rtl-sms ddg_coverage.c -o ddg_coverage
 */

#include <stdlib.h>

/* Global variables for function call dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Helper function with side effects (Requirement 5) */
static inline void update_globals(int* arr, int idx, float* farr) {
    global_counter++;
    *arr += global_counter;
    global_accumulator += *farr;
    *farr = global_accumulator * 0.5f;
}

/* Another static function for different dependency pattern */
static void transform_value(float* x, int* y) {
    volatile float tmp = *x;  /* Prevent optimization */
    *x = tmp * 1.1f + *y;
    *y = (int)(*x) % 256;
}

int main(void) {
    const int N = 1024;
    volatile int limit = N;  /* Prevent constant propagation */
    
    /* Arrays with different data types (Requirement 6) */
    int int_arr[N];
    float float_arr[N];
    double double_arr[N];
    
    /* Non-linear index array (Requirement 4) */
    int indices[N];
    for (int i = 0; i < N; i++) {
        indices[i] = (i * i + i * 3 + 7) % N;  /* Quadratic non-affine */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i;
        float_arr[i] = i * 0.5f;
        double_arr[i] = i * 0.25;
    }
    
    volatile int result = 0;
    
    /* PRIMARY LOOP with multiple dependency types */
    for (int i = 1; i < limit; i++) {
        /* 1. Loop-carried true dependency (RAW) - Requirement 1 */
        int_arr[i] = int_arr[i-1] + 2;  /* Integer chain */
        float_arr[i] = float_arr[i-1] * 1.01f;  /* Float chain */
        
        /* 2. Anti-dependency (WAR) - Requirement 2 */
        int tmp_storage = int_arr[i];  /* Read */
        int_arr[i] = float_arr[i] > 0 ? 1 : -1;  /* Write later - creates WAR */
        float tmp_float = float_arr[i];
        float_arr[i] = tmp_storage * 0.3f;  /* Another WAR */
        
        /* 3. Output dependency (WAW) - Requirement 2 */
        double local_var = double_arr[i];
        local_var = local_var * 2.0;  /* First write */
        local_var = local_var + 1.0;  /* Second write to same variable - WAW */
        double_arr[i] = local_var;
        
        /* 4. Conditional dependencies - Requirement 3 */
        if (i % 3 == 0) {
            /* One dependency pattern */
            int_arr[i] = float_arr[i] + int_arr[i-1];
            float_arr[i] = int_arr[i] * 0.5f;
        } else if (i % 3 == 1) {
            /* Different pattern creating cross-iteration dependencies */
            float_arr[i] = int_arr[i-1] * 0.7f + float_arr[i-1];
            int_arr[i] = (int)float_arr[i] ^ int_arr[i-1];
        } else {
            /* Third pattern with output dependencies */
            int tmp = int_arr[i];
            int_arr[i] = tmp * 2;  /* WAW on int_arr[i] */
            float_arr[i] = tmp * 3.0f;
        }
        
        /* 5. Function call with side effects - Requirement 5 */
        update_globals(&int_arr[i], i, &float_arr[i]);
        
        /* Mixed data type operations - Requirement 6 */
        float mixed_result = int_arr[i] * 0.25f + float_arr[i];
        double_arr[i] = mixed_result * 1.5 + double_arr[i-1];
        
        /* Another function call with different signature */
        transform_value(&float_arr[i], &int_arr[i]);
    }
    
    /* SECONDARY LOOP with non-affine array accesses - Requirement 4 */
    for (int i = 0; i < limit - 10; i++) {
        int idx = indices[i];  /* Non-linear access pattern */
        int_arr[idx] = int_arr[indices[i+1]] + int_arr[indices[i+2]];
        
        /* Create complex dependency chain */
        float_arr[idx] = float_arr[indices[i+3]] * 0.9f + 
                        float_arr[indices[i+4]] * 0.1f;
        
        /* Cross-type dependencies */
        double_arr[idx] = int_arr[idx] + float_arr[idx];
    }
    
    /* NESTED LOOP for additional DDG complexity */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int index = i * 32 + j;
            if (index < N) {
                /* Create dependencies in both dimensions */
                float_arr[index] = float_arr[index] + 
                                  (i > 0 ? float_arr[index - 32] : 0) +
                                  (j > 0 ? float_arr[index - 1] : 0);
                
                /* Interleaved dependencies */
                int_arr[index] = (int_arr[index] + int_arr[index] % 17) * 
                                (j % 2 == 0 ? 2 : 3);
            }
        }
    }
    
    /* Aggregate results to prevent dead code elimination */
    for (int i = 0; i < limit; i++) {
        result += int_arr[i];
        result += (int)float_arr[i];
        result += (int)double_arr[i];
    }
    
    /* Use globals to prevent optimization */
    result += global_counter;
    result += (int)global_accumulator;
    
    return result % 256;
}
