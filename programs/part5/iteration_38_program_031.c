/* ddg_edge_coverage.c
 * Program designed to exercise DDG edge creation in GCC's instruction scheduler
 * Compile with: gcc -O2 -fschedule-insns -fdump-rtl-sched1 -fdump-rtl-sched2 ddg_edge_coverage.c -o ddg_test
 * Or for modulo scheduling: gcc -O3 -fmodulo-sched -fdump-rtl-sms ddg_edge_coverage.c -o ddg_test
 */

#include <stdlib.h>

/* Global variables for function call dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Helper function with side effects (Requirement 5) */
static inline void update_global(int* ptr, float* fptr) {
    *ptr += 1;
    *fptr += 0.5f;
    global_counter = *ptr;
}

/* Another helper that creates register pressure */
static int transform_value(int x, float y) {
    volatile int v = x;  /* Prevent optimization */
    return (int)(v * y + 0.5f);
}

int main(void) {
    /* Declare arrays with different data types (Requirement 6) */
    const int N = 1024;
    int arr_int[N];
    float arr_float[N];
    double arr_double[N];
    volatile int limit = N;  /* Volatile to prevent constant propagation */
    
    /* Non-linear index array (Requirement 4) */
    int indices[N];
    for (int i = 0; i < N; i++) {
        indices[i] = (i * i + 3 * i + 7) % N;  /* Non-affine index pattern */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr_int[i] = i;
        arr_float[i] = i * 0.5f;
        arr_double[i] = i * 0.25;
    }
    
    /* PRIMARY LOOP with multiple dependency types */
    for (int i = 1; i < limit; i++) {
        /* 1. LOOP-CARRIED TRUE DEPENDENCY (RAW) - Requirement 1 */
        arr_int[i] = arr_int[i-1] + transform_value(i, arr_float[i]);
        
        /* 2. ANTI-DEPENDENCY (WAR) - Requirement 2 */
        int tmp = arr_int[i];           /* Read arr_int[i] */
        arr_int[i] = (int)arr_float[i]; /* Write arr_int[i] - WAR with previous read */
        arr_float[i] = tmp * 0.3f;      /* Use tmp */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Requirement 2 */
        float f_tmp = arr_float[i] * 2.0f;
        f_tmp = arr_float[i-1] + 1.0f;  /* Second write to f_tmp - WAW */
        arr_float[i] = f_tmp;
        
        /* 4. CONDITIONAL DEPENDENCIES - Requirement 3 */
        if (i % 3 == 0) {
            /* One dependency pattern */
            arr_double[i] = arr_double[i-1] * 1.1;
            arr_int[i] = (int)arr_double[i];
        } else if (i % 3 == 1) {
            /* Different pattern with reversed flow */
            arr_int[i] = arr_int[i] * 2;
            arr_double[i] = arr_int[i] * 0.5;
        } else {
            /* Third pattern with anti-dependency */
            double d_tmp = arr_double[i];
            arr_double[i] = arr_int[i] * 0.25;
            arr_float[i] = (float)d_tmp;
        }
        
        /* 5. FUNCTION CALL WITH SIDE EFFECTS - Requirement 5 */
        update_global(&global_counter, &global_accumulator);
        
        /* 6. MIXED DATA TYPE OPERATIONS - Requirement 6 */
        arr_int[i] = arr_int[i] + (int)(arr_float[i] * arr_double[i]);
        arr_float[i] = arr_float[i] + (float)(arr_int[i] % 17);
    }
    
    /* SECONDARY LOOP with non-linear array access */
    int sum = 0;
    for (int i = 0; i < limit - 10; i++) {
        /* Access using non-affine indices - Requirement 4 */
        int idx = indices[i];
        arr_int[idx] = arr_int[indices[i+1]] + arr_int[indices[i+2]];
        
        /* Create cross-iteration dependency with non-linear pattern */
        if (i > 0) {
            arr_float[idx] = arr_float[indices[i-1]] * 1.5f;
        }
        
        /* Mixed type chain */
        double d_val = arr_double[idx] + arr_int[idx];
        arr_double[(idx + 1) % N] = d_val * 0.9;
        
        sum += arr_int[idx];
    }
    
    /* NESTED LOOP for additional complexity */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int idx = i * 10 + j;
            if (idx < N) {
                /* Cross-iteration dependency in inner loop */
                arr_float[idx] = arr_float[idx] + (i > 0 ? arr_float[idx-10] : 0.0f);
                
                /* Output dependency */
                float tmp_calc = arr_float[idx] * j;
                tmp_calc = tmp_calc / (j + 1.0f);  /* WAW on tmp_calc */
                arr_int[idx] = (int)tmp_calc;
            }
        }
    }
    
    /* Prevent dead code elimination */
    volatile int result = 0;
    result += sum;
    result += (int)global_accumulator;
    result += global_counter;
    
    /* Use all arrays to prevent optimization */
    for (int i = 0; i < N; i += 64) {
        result += arr_int[i];
        result += (int)arr_float[i];
        result += (int)arr_double[i];
    }
    
    return result;
}
