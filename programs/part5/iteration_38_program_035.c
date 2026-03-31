/* ddg_test.c - Comprehensive test for DDG edge creation */
#include <stdlib.h>

/* Global variables for memory dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Helper function with side effects (Requirement 5) */
static void update_globals(int* arr, int idx, float* farr) {
    global_counter++;
    global_accumulator += farr[idx % 8];
    arr[idx % 8] = global_counter;
}

/* Another helper with output dependency potential */
static inline float transform_value(float x, int scale) {
    static float last_result;  /* Creates potential output dependencies */
    float result = x * scale + global_accumulator;
    last_result = result;      /* WAW dependency through static variable */
    return result;
}

int main(void) {
    /* Declare arrays with different data types (Requirement 6) */
    int int_arr[256] = {0};
    float float_arr[256] = {0.0f};
    double double_arr[128] = {0.0};
    volatile int N = 128;  /* Prevent optimization */
    
    /* Non-linear index array (Requirement 4) */
    int nonlinear_idx[128];
    for (int i = 0; i < 128; i++) {
        nonlinear_idx[i] = (i * i + i * 3 + 7) % 128;  /* Quadratic indexing */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < 128; i++) {
        float_arr[i] = (float)i * 0.5f;
        double_arr[i] = (double)i * 0.25;
    }
    
    /* PRIMARY LOOP with multiple dependency types */
    for (int i = 1; i < N; i++) {
        /* 1. LOOP-CARRIED TRUE DEPENDENCY (RAW) - Requirement 1 */
        int_arr[i] = int_arr[i-1] + i;  /* True dependency chain */
        
        /* Mixed data type operations - Requirement 6 */
        float temp_float = float_arr[i] * 2.0f;
        int temp_int = int_arr[i] / 3;
        
        /* 2. ANTI-DEPENDENCY (WAR) - Requirement 2 */
        int read_first = int_arr[i];      /* Read */
        float_arr[i] = temp_float + read_first;  /* Write to different array */
        int_arr[i] = temp_int * 2;        /* Overwrite original location - WAR */
        
        /* 3. CONDITIONAL DEPENDENCY PATTERNS - Requirement 3 */
        if (i % 3 == 0) {
            /* Pattern A: Longer dependency chain */
            double_arr[i % 128] = double_arr[(i-1) % 128] * 1.1;
            float_arr[i] = (float)double_arr[i % 128] + float_arr[i-1];
        } else if (i % 3 == 1) {
            /* Pattern B: Output dependencies (WAW) - Requirement 2 */
            float tmp = transform_value(float_arr[i], i);
            tmp = tmp * 0.5f;            /* WAW on tmp */
            float_arr[i] = tmp;
            
            /* Another WAW example */
            int_arr[i] = i * 2;
            int_arr[i] = int_arr[i] + 5;  /* Second write to same location */
        } else {
            /* Pattern C: Complex anti-dependencies */
            int save_val = int_arr[i];
            float_arr[i] = float_arr[i] * 2.0f;
            int_arr[(i+1) % 128] = save_val;  /* WAR through different index */
        }
        
        /* 5. FUNCTION CALL WITH SIDE EFFECTS - Requirement 5 */
        update_globals(int_arr, i, float_arr);
        
        /* Volatile operations to prevent optimization */
        volatile int volatile_dummy = int_arr[i];
        (void)volatile_dummy;
    }
    
    /* SECONDARY LOOP with non-linear array access - Requirement 4 */
    for (int i = 0; i < N; i++) {
        int idx = nonlinear_idx[i];  /* Non-affine index */
        
        /* Create dependencies through non-linear access */
        if (idx > 0) {
            float_arr[idx] = float_arr[nonlinear_idx[idx-1]] * 1.5f;
        }
        
        /* More mixed-type operations - Requirement 6 */
        double_arr[idx % 128] = (double)int_arr[idx] * 0.33;
        
        /* Output dependency in this loop too */
        double local_acc = double_arr[idx % 128];
        local_acc = local_acc + 1.0;  /* WAW on local_acc */
        double_arr[idx % 128] = local_acc;
    }
    
    /* NESTED LOOP for additional complexity */
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            /* Multi-dimensional with non-trivial index */
            int idx = i * 32 + j;
            int_arr[idx] = int_arr[idx] + i * j;
            
            /* Conditional with potential dependencies */
            if (j > i) {
                float_arr[idx] = float_arr[idx - 1] + float_arr[idx];
            } else {
                float_arr[idx] = float_arr[idx] * 0.9f;
            }
        }
    }
    
    /* Aggregate results to prevent dead code elimination */
    volatile int final_result = 0;
    for (int i = 0; i < 128; i++) {
        final_result += int_arr[i];
        final_result += (int)float_arr[i];
        final_result += (int)double_arr[i % 128];
    }
    final_result += global_counter;
    
    return final_result % 256;
}
