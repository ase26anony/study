/* ddg_test.c - Test program for DDG edge creation coverage */
#include <stdlib.h>

/* Global variables for memory dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Static function with side effects (Requirement 5) */
static void update_global(int *arr, int idx) {
    global_counter++;
    global_accumulator += arr[idx] * 0.5f;
    arr[idx] = global_counter;
}

/* Another static function for output dependencies */
static inline float transform_value(float x, int scale) {
    static float last_result = 0.0f;
    float result = x * scale + last_result;
    last_result = result;  /* Creates output dependency between calls */
    return result;
}

int main(void) {
    /* Declare arrays with different data types (Requirement 6) */
    const int N = 100;
    int int_arr[N];
    float float_arr[N];
    double double_arr[N];
    volatile int limit = N;  /* Prevent optimization */
    
    /* Non-linear index array (Requirement 4) */
    int indices[N];
    for (int i = 0; i < N; i++) {
        indices[i] = (i * i + i * 3) % N;  /* Non-affine access pattern */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i;
        float_arr[i] = i * 0.5f;
        double_arr[i] = i * 0.25;
    }
    
    volatile int result = 0;
    
    /* Primary loop with various dependencies */
    for (int i = 1; i < limit; i++) {
        /* 1. Loop-carried true dependency (RAW) - Requirement 1 */
        int_arr[i] = int_arr[i-1] + 2;  /* True dependency chain */
        
        /* 2. Anti-dependency (WAR) - Requirement 2 */
        float temp = float_arr[i];      /* Read */
        float_arr[i] = int_arr[i] * 0.7f; /* Write to same location */
        double_arr[i] = temp;           /* Use the read value */
        
        /* 3. Output dependency (WAW) - Requirement 2 */
        float f1 = transform_value(float_arr[i], i);
        float f1_again = transform_value(float_arr[i], i+1); /* WAW on last_result */
        
        /* 4. Conditional dependency patterns - Requirement 3 */
        if (i % 3 == 0) {
            /* Pattern A: More complex dependency chain */
            int_arr[i] = int_arr[i] * 2 - int_arr[i-1];
            float_arr[i] = float_arr[i] + float_arr[i-1];
        } else if (i % 3 == 1) {
            /* Pattern B: Different dependency pattern */
            float_arr[i-1] = int_arr[i] * 0.3f;
            int_arr[i] = (int)float_arr[i-1] + 1;
        } else {
            /* Pattern C: Cross-type dependencies */
            double_arr[i] = int_arr[i] + float_arr[i];
            int_arr[i] = (int)(double_arr[i] * 1.5);
        }
        
        /* 5. Function call with side effects - Requirement 5 */
        update_global(int_arr, i % 10);
        
        /* 6. Mixed data type operations - Requirement 6 */
        float mixed = int_arr[i] * 0.25f + float_arr[i];
        double_arr[i] = mixed * 1.1 + global_accumulator;
    }
    
    /* Nested loop with non-linear array access - Requirement 4 */
    for (int i = 0; i < limit/2; i++) {
        for (int j = 0; j < 5; j++) {
            int idx = indices[(i * 7 + j * 3) % N];  /* Complex index */
            float_arr[idx] = float_arr[idx] * 1.1f + j;
            
            /* Create anti-dependency in nested loop */
            double temp_d = double_arr[idx];
            double_arr[idx] = float_arr[idx] * 2.0;
            int_arr[idx] = (int)(temp_d * 3);
        }
    }
    
    /* Additional loop with output dependencies on local variables */
    int output_var = 0;
    for (int i = 0; i < limit; i++) {
        /* Multiple writes to same variable - WAW dependencies */
        output_var = int_arr[i] * 2;
        output_var = output_var + float_arr[i];  /* Output dependency */
        output_var = (int)(output_var * 1.5);
        
        /* Use volatile to prevent optimization */
        result += output_var;
    }
    
    /* Final aggregation to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        result += int_arr[i] + (int)float_arr[i] + (int)double_arr[i];
    }
    
    result += global_counter + (int)global_accumulator;
    
    return result % 256;  /* Return non-zero to prevent optimization */
}
