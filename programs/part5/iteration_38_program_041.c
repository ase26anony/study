/* ddg_coverage.c - Program to exercise DDG edge creation in GCC scheduler */
#include <stdlib.h>

/* Global variables for function call dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Helper function with side effects (Requirement 5) */
static void update_globals(int *arr, int idx, float *farr) {
    global_counter++;
    global_accumulator += farr[idx % 8];
    arr[idx % 16] = global_counter;
}

/* Another helper with output dependency potential */
static inline float compute_value(float a, float b, int iter) {
    static float last_result;  /* Creates WAW dependencies across calls */
    float result;
    
    /* Create RAW dependency chain */
    result = a * b + iter;
    
    /* Output dependency on static variable */
    last_result = result;
    
    /* Anti-dependency on parameter through computation */
    a = result / (b + 1.0f);
    
    return last_result + a;
}

int main(void) {
    /* Declare arrays with different data types (Requirement 6) */
    int int_arr[256];
    float float_arr[256];
    double double_arr[128];
    volatile int N = 128;  /* Prevent optimization of loop bounds */
    
    /* Non-linear index array (Requirement 4) */
    int nonlin_idx[256];
    for (int i = 0; i < 256; i++) {
        nonlin_idx[i] = (i * i + i * 3 + 7) % 256;  /* Quadratic mapping */
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
    float temp_float = 0.0f;
    int temp_int = 0;
    
    /* PRIMARY LOOP with multiple dependency types */
    for (int i = 1; i < N; i++) {
        /* 1. LOOP-CARRIED TRUE DEPENDENCY (RAW) - Integer (Requirement 1) */
        int_arr[i] = int_arr[i-1] + i;  /* Classic loop-carried dependency */
        
        /* 2. LOOP-CARRIED TRUE DEPENDENCY - Floating point */
        float_arr[i] = float_arr[i-1] * 1.01f + float_arr[i];
        
        /* 3. ANTI-DEPENDENCY (WAR) on local variables (Requirement 2) */
        temp_int = int_arr[i];          /* Read int_arr[i] */
        int_arr[i] = i * 2;             /* Overwrite int_arr[i] - WAR with above */
        result += temp_int;             /* Use the read value */
        
        /* 4. OUTPUT DEPENDENCY (WAW) on local variable */
        temp_float = compute_value(float_arr[i], float_arr[i-1], i);
        temp_float = float_arr[i] / 3.0f;  /* Overwrite temp_float - WAW */
        
        /* 5. CONDITIONAL DEPENDENCY PATTERNS (Requirement 3) */
        if (i % 3 == 0) {
            /* Pattern A: Simple RAW chain */
            double_arr[i % 128] = double_arr[(i-1) % 128] * 0.99;
            temp_int = int_arr[i];
            int_arr[i] = temp_int + global_counter;
        } else if (i % 3 == 1) {
            /* Pattern B: WAR through array */
            float tmp = float_arr[i];
            float_arr[i] = global_accumulator;
            float_arr[(i+1) % 256] = tmp;  /* Anti-dependency on tmp */
        } else {
            /* Pattern C: WAW on multiple locations */
            int_arr[i] = global_counter * 2;
            int_arr[i] = global_counter + i;  /* Output dependency */
        }
        
        /* 6. FUNCTION CALL WITH SIDE EFFECTS (Requirement 5) */
        update_globals(int_arr, i, float_arr);
        
        /* 7. MIXED DATA TYPE OPERATIONS (Requirement 6) */
        volatile float mixed_calc;
        mixed_calc = int_arr[i] * 0.5f + float_arr[i];  /* int->float conversion */
        double_arr[i % 128] = mixed_calc * double_arr[i % 128];
        
        /* 8. COMPLEX EXPRESSION WITH MULTIPLE USES */
        int complex_idx = (int_arr[i] + nonlin_idx[i]) % 256;
        float_arr[complex_idx] = float_arr[complex_idx] * 2.0f - 1.0f;
    }
    
    /* SECONDARY NESTED LOOP with non-linear access (Requirement 4) */
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 8; j++) {
            /* Non-affine access pattern */
            int idx = nonlin_idx[i * 8 + j];
            float_arr[idx] = float_arr[idx] + int_arr[i] * j;
            
            /* Cross-iteration dependency in inner loop */
            if (j > 0) {
                double_arr[idx % 128] = double_arr[idx % 128] * 
                                       double_arr[(nonlin_idx[i * 8 + j - 1]) % 128];
            }
        }
    }
    
    /* AGGREGATE RESULTS to prevent dead code elimination */
    volatile float final_result = 0.0f;
    for (int i = 0; i < 256; i++) {
        final_result += float_arr[i] + int_arr[i];
    }
    for (int i = 0; i < 128; i++) {
        final_result += double_arr[i];
    }
    
    /* Return value prevents optimization */
    return (int)final_result + result + global_counter;
}
