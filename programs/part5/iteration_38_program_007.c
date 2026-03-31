/* ddg_test.c - Test program to exercise DDG edge creation */
#include <stdlib.h>

/* Global variables for function call dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Helper function with side effects (Requirement 5) */
static void update_globals(int *arr, float *farr, int idx) {
    global_counter++;
    global_accumulator += farr[idx % 16];
    arr[idx % 16] = global_counter;
}

/* Another helper for output dependencies */
static inline float compute_value(float a, float b) {
    volatile float result; /* Prevent optimization */
    result = a * b;
    return result;
}

int main(void) {
    /* Declare arrays with different data types (Requirement 6) */
    int int_arr[256];
    float float_arr[256];
    double double_arr[128];
    volatile int N = 256; /* Volatile loop limit */
    
    /* Non-linear index array (Requirement 4) */
    int nonlin_idx[256];
    for (int i = 0; i < 256; i++) {
        nonlin_idx[i] = (i * i + 3 * i + 7) % 256;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        int_arr[i] = i;
        float_arr[i] = i * 0.5f;
    }
    for (int i = 0; i < 128; i++) {
        double_arr[i] = i * 0.25;
    }
    
    volatile int result = 0;
    float tmp_float = 0.0f;
    int tmp_int = 0;
    
    /* Primary loop with various dependencies */
    for (int i = 1; i < N; i++) {
        /* 1. Loop-carried true dependency (RAW) - Requirement 1 */
        int_arr[i] = int_arr[i-1] + 1; /* Integer chain */
        float_arr[i] = float_arr[i-1] * 1.1f; /* Float chain */
        
        /* 2. Anti-dependency (WAR) - Requirement 2 */
        tmp_int = int_arr[i];           /* Read */
        int_arr[i] = float_arr[i] > 0 ? 1 : 0; /* Write later - creates MEM_DEP */
        result += tmp_int;              /* Use read value */
        
        /* 3. Output dependency (WAW) - Requirement 2 */
        tmp_float = compute_value(float_arr[i], 2.0f); /* First write to tmp_float */
        tmp_float = compute_value(tmp_float, 0.5f);    /* Second write - creates REG_DEP */
        
        /* 4. Conditional dependencies - Requirement 3 */
        if (i % 3 == 0) {
            /* One dependency pattern */
            double_arr[i % 128] = double_arr[(i-1) % 128] + 0.1;
            tmp_int = int_arr[i] * 2;
        } else if (i % 3 == 1) {
            /* Alternative pattern with cross-iteration dependency */
            int_arr[i] = tmp_int + int_arr[i-1]; /* Uses tmp_int from previous iteration */
            tmp_int = i;
        } else {
            /* Third pattern with anti-dependency */
            float old_val = float_arr[i];
            float_arr[i] = old_val * old_val;
            tmp_float = old_val;
        }
        
        /* 5. Function call with side effects - Requirement 5 */
        update_globals(int_arr, float_arr, i);
        
        /* 6. Mixed data type operations - Requirement 6 */
        float mixed_result = int_arr[i] * 0.5f + float_arr[i];
        double_arr[i % 128] = mixed_result * 0.25;
        
        /* Create register pressure and dependencies */
        volatile int reg_var = int_arr[i] + global_counter;
        reg_var *= 2;
        result += reg_var;
    }
    
    /* Nested loop with non-linear array access - Requirement 4 */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 16; j++) {
            /* Access using non-affine index */
            int idx = nonlin_idx[i * 16 + j];
            float_arr[idx] = float_arr[idx] + int_arr[j] * 0.1f;
            
            /* Cross-iteration dependency in inner loop */
            if (j > 0) {
                double_arr[j % 128] = double_arr[(j-1) % 128] * 1.01;
            }
        }
    }
    
    /* Additional loop with complex dependencies */
    for (int i = 0; i < 128; i++) {
        /* Multiple writes to same location (WAW) */
        double tmp_dbl = double_arr[i];
        tmp_dbl = tmp_dbl * tmp_dbl;
        tmp_dbl = sqrt(tmp_dbl); /* Assume math.h included in real use */
        double_arr[i] = tmp_dbl;
        
        /* Chain of dependencies */
        int_arr[i*2] = int_arr[i*2] + (int)tmp_dbl;
        float_arr[i*2] = float_arr[i*2] + (float)int_arr[i*2];
    }
    
    /* Aggregate results to prevent dead code elimination */
    volatile int final_result = 0;
    for (int i = 0; i < 256; i++) {
        final_result += int_arr[i];
        final_result += (int)float_arr[i];
    }
    for (int i = 0; i < 128; i++) {
        final_result += (int)double_arr[i];
    }
    final_result += global_counter;
    
    return final_result % 256;
}
