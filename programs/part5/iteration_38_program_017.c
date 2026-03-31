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
static float compute_value(float a, float b) {
    static float last_result = 0.0f;
    float result = a * b + last_result;
    last_result = result * 0.9f;
    return result;
}

int main(void) {
    const int N = 1024;
    volatile int limit = N;  /* Prevent optimization */
    
    /* Arrays with different data types (Requirement 6) */
    int int_arr[N];
    float float_arr[N];
    double double_arr[N];
    
    /* Non-linear index array (Requirement 4) */
    int nonlin_idx[N];
    for (int i = 0; i < N; i++) {
        nonlin_idx[i] = (i * i + i * 3 + 7) % N;  /* Quadratic non-affine */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i;
        float_arr[i] = i * 0.5f;
        double_arr[i] = i * 0.25;
    }
    
    volatile int result = 0;
    
    /* PRIMARY LOOP with complex dependencies */
    for (int i = 1; i < limit; i++) {
        /* 1. LOOP-CARRIED TRUE DEPENDENCY (RAW) - Requirement 1 */
        int_arr[i] = int_arr[i-1] + i;  /* True dependency across iterations */
        
        /* 2. ANTI-DEPENDENCY (WAR) - Requirement 2 */
        float temp = float_arr[i];      /* Read */
        float_arr[i] = int_arr[i] * 0.7f; /* Write to same location */
        double_arr[i] = temp * 2.0;     /* Use read value */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Requirement 2 */
        float tmp_var = compute_value(float_arr[i], i * 0.1f);
        tmp_var = tmp_var * 1.1f;       /* Second write to same variable */
        
        /* 4. CONDITIONAL DEPENDENCIES - Requirement 3 */
        if (i % 3 == 0) {
            /* One dependency pattern */
            int_arr[i] = float_arr[i] * 2;
            float_arr[i] = int_arr[i-1] * 0.3f;
        } else if (i % 3 == 1) {
            /* Different pattern with reversed flow */
            float_arr[i-1] = int_arr[i] * 0.7f;
            int_arr[i] = float_arr[i] + 5;
        } else {
            /* Third pattern with both dependencies */
            int old_val = int_arr[i];
            int_arr[i] = float_arr[i] * 4;
            float_arr[i] = old_val * 0.2f;
        }
        
        /* 5. FUNCTION CALL WITH SIDE EFFECTS - Requirement 5 */
        update_global(int_arr, i % 100);
        
        /* 6. MIXED DATA TYPE OPERATIONS - Requirement 6 */
        double mixed_result = int_arr[i] * 0.5 + float_arr[i] * 1.5;
        float_arr[i] = mixed_result * 0.8f;
        
        /* Anti-dependency on local variable */
        int local_tmp = int_arr[i] * 2;
        int_arr[i] = local_tmp + 1;     /* WAR on int_arr[i] */
        local_tmp = float_arr[i] * 3;   /* WAW on local_tmp */
    }
    
    /* SECONDARY LOOP with non-linear array access - Requirement 4 */
    for (int i = 0; i < limit - 10; i++) {
        /* Access using non-affine indices */
        int idx1 = nonlin_idx[i];
        int idx2 = nonlin_idx[(i * 7 + 3) % N];
        
        /* Create dependencies through non-linear access */
        float_arr[idx1] = float_arr[idx2] * 1.1f;
        int_arr[idx2] = int_arr[idx1] + i;
        
        /* More complex non-linear pattern */
        for (int j = 0; j < 5; j++) {
            int complex_idx = (i * j + j * j) % N;
            double_arr[complex_idx] = double_arr[nonlin_idx[j]] * 0.9;
        }
    }
    
    /* NESTED LOOP with output dependencies */
    for (int i = 0; i < limit / 2; i++) {
        float output_var = 0.0f;
        
        /* Multiple writes to same variable - WAW dependencies */
        for (int j = 0; j < 4; j++) {
            output_var = int_arr[i + j] * (j + 1);
            output_var = output_var + float_arr[i + j];  /* Another WAW */
        }
        
        /* Store result to prevent elimination */
        float_arr[i] = output_var;
    }
    
    /* Aggregate results to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        result += int_arr[i];
        result += (int)float_arr[i];
        result += (int)double_arr[i];
    }
    
    result += global_counter;
    
    printf("Result: %d\n", result);
    return result;
}
