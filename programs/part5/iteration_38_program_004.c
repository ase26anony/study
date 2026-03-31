/* ddg_test.c - Test program to exercise DDG edge creation in GCC scheduler */
#include <stdlib.h>

/* Global variables for function call dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Helper function with side effects (Requirement 5) */
static void update_global(int *arr, int idx) {
    global_counter++;
    global_accumulator += arr[idx] * 0.5f;
    arr[idx] = global_counter % 100;
}

/* Another helper with output dependency potential */
static inline float transform_value(float x, int scale) {
    static float last_result = 0.0f;  /* Creates potential output dep */
    float result = x * scale + last_result;
    last_result = result * 0.9f;  /* WAR with result */
    return result;
}

int main(void) {
    /* Declare arrays with different data types (Requirement 6) */
    const int N = 1024;
    volatile int limit = N;  /* Prevent optimization */
    int int_arr[N];
    float float_arr[N];
    double double_arr[N];
    int indices[N];  /* For non-affine access */
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i % 100;
        float_arr[i] = i * 0.5f;
        double_arr[i] = i * 0.25;
        /* Create non-linear index pattern (Requirement 4) */
        indices[i] = (i * i + i * 3 + 7) % N;
    }
    
    /* Primary loop with various dependencies */
    int loop_carried = 0;
    float temp_float = 0.0f;
    volatile int result = 0;  /* Prevent dead code elimination */
    
    for (int i = 1; i < limit; i++) {
        /* 1. LOOP-CARRIED TRUE DEPENDENCY (RAW) - Requirement 1 */
        int_arr[i] = int_arr[i-1] + i;  /* True dependency across iterations */
        
        /* 2. ANTI-DEPENDENCY (WAR) - Requirement 2 */
        int tmp = int_arr[i];          /* Read */
        int_arr[i] = float_arr[i] > 0 ? tmp + 1 : tmp - 1;  /* Later write - WAR */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Requirement 2 */
        float_arr[i] = transform_value(float_arr[i], i);
        float_arr[i] = float_arr[i] * 0.8f;  /* WAW on float_arr[i] */
        
        /* 4. CONDITIONAL DEPENDENCIES - Requirement 3 */
        if (i % 3 == 0) {
            /* One dependency pattern */
            loop_carried = int_arr[i] + loop_carried;  /* Loop-carried */
            temp_float = float_arr[i];
        } else if (i % 3 == 1) {
            /* Different pattern creating alternative edges */
            float_arr[i] = loop_carried * 0.5f;  /* Uses loop_carried */
            loop_carried = int_arr[i];           /* WAR on loop_carried */
        } else {
            /* Third pattern with output dependency */
            double local = double_arr[i];
            local = local * local;  /* WAW on local */
            double_arr[i] = local;
        }
        
        /* 5. FUNCTION CALL WITH SIDE EFFECTS - Requirement 5 */
        update_global(int_arr, i % 100);
        
        /* 6. MIXED DATA TYPE OPERATIONS - Requirement 6 */
        float mixed = int_arr[i] * 0.7f + global_accumulator;
        double_arr[i] = mixed * 2.0 + double_arr[i-1];  /* Another true dep */
        
        /* Anti-dependency with different types */
        int int_tmp = float_arr[i];  /* Float to int conversion */
        float_arr[i] = int_tmp * global_counter;
    }
    
    /* Nested loop with non-affine array access (Requirement 4) */
    int sum = 0;
    for (int i = 0; i < limit/2; i++) {
        for (int j = 0; j < 4; j++) {
            /* Non-linear access pattern */
            int idx = indices[(i * 4 + j) % N];
            sum += int_arr[idx] * j;
            
            /* Create output dependency in inner loop */
            float tmp_val = float_arr[idx];
            tmp_val = tmp_val + j * 0.25f;  /* WAW */
            float_arr[idx] = tmp_val;
        }
    }
    
    /* Complex dependency chain with mixed types */
    double chain_result = 0.0;
    for (int i = 1; i < limit; i++) {
        /* Chain of dependencies */
        int step1 = int_arr[i] + global_counter;
        float step2 = step1 * 0.3f + float_arr[i-1];  /* RAW on float_arr */
        double step3 = step2 + double_arr[i];         /* RAW on double_arr */
        int step4 = step3 * 2;                        /* Type conversion */
        chain_result += step4;
        
        /* Function call creating memory dependencies */
        update_global(int_arr, step4 % 100);
    }
    
    /* Aggregate results to prevent optimization */
    result = sum + chain_result + global_counter + loop_carried;
    
    /* Use all arrays to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        result += int_arr[i] + float_arr[i] + double_arr[i];
    }
    
    return result % 255;
}
