/* Complex dependency pattern generator for DDG edge coverage */
#include <stdlib.h>
#include <math.h>

/* Global variables for function call dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Static helper function with side effects (Requirement 5) */
static void update_global(int* arr, int idx, float* farr) {
    global_counter++;
    *arr += global_counter;
    *farr += (float)global_counter * 0.5f;
    global_accumulator += *farr;
}

/* Another static function for output dependency pattern */
static inline float transform_value(float x, int scale) {
    volatile float result; /* Prevent optimization */
    result = x * (float)scale;
    result = sinf(result); /* Overwrite result - output dependency */
    return result;
}

int main(void) {
    const int N = 100;
    volatile int limit = N; /* Prevent optimization (Requirement 6) */
    
    /* Arrays with different data types */
    int int_arr[N];
    float float_arr[N];
    double double_arr[N];
    
    /* Non-linear index array (Requirement 4) */
    int nonlin_idx[N];
    for (int i = 0; i < N; i++) {
        nonlin_idx[i] = (i * i + i * 3) % N; /* Non-affine index calculation */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i;
        float_arr[i] = (float)i * 1.5f;
        double_arr[i] = (double)i * 2.5;
    }
    
    volatile int result = 0; /* Prevent dead code elimination */
    
    /* Primary loop with complex dependencies */
    for (int i = 1; i < limit; i++) {
        /* 1. Loop-carried true dependency (RAW) - Requirement 1 */
        int_arr[i] = int_arr[i-1] + i * 2; /* Integer chain */
        float_arr[i] = float_arr[i-1] * 1.1f + (float)int_arr[i]; /* Mixed type */
        
        /* 2. Anti-dependency (WAR) pattern - Requirement 2 */
        int tmp_storage = int_arr[i]; /* Read */
        float tmp_float = float_arr[i];
        
        /* Conditional for different dependency patterns - Requirement 3 */
        if (i % 3 == 0) {
            /* Path 1: More complex dependency chain */
            int_arr[i] = tmp_storage * 2; /* Overwrite - WAR */
            float_arr[i] = tmp_float + transform_value(float_arr[i], i);
            
            /* Output dependency (WAW) on local variable */
            double local_var = (double)int_arr[i];
            local_var = cos(local_var); /* Overwrite - WAW */
            double_arr[i] = local_var;
        } else if (i % 3 == 1) {
            /* Path 2: Different pattern with anti-dependency */
            int_arr[i] = tmp_storage + nonlin_idx[i]; /* Uses non-linear index */
            float_arr[i] = transform_value(tmp_float, i % 5);
            
            /* Another output dependency */
            float reused = float_arr[i] * 2.0f;
            reused = reused / 3.0f; /* WAW */
            float_arr[i] = reused;
        } else {
            /* Path 3: Simple chain with function call */
            int_arr[i] = tmp_storage - i;
            
            /* Function call creating memory dependencies - Requirement 5 */
            update_global(&int_arr[i], i, &float_arr[i]);
        }
        
        /* 3. Mixed data type operations - Requirement 6 */
        /* Integer operations feeding float operations */
        int int_op = int_arr[i] * 3 - i;
        float float_op = (float)int_op / 7.0f;
        
        /* Float operations feeding double operations */
        double_arr[i] = (double)float_op * 1.75;
        
        /* Volatile operations to prevent optimization */
        volatile float volatile_sink = float_op;
        (void)volatile_sink;
    }
    
    /* Nested loop with non-linear array access - Requirement 4 */
    for (int i = 0; i < limit / 2; i++) {
        for (int j = 0; j < 5; j++) {
            /* Access using non-affine index */
            int idx = nonlin_idx[(i * 7 + j * 3) % N];
            
            /* Complex dependency chain within nested loop */
            float_arr[idx] = float_arr[idx] + (float)(int_arr[i] * j);
            
            /* Anti-dependency pattern in nested context */
            double temp = double_arr[idx];
            double_arr[idx] = sin(temp * 0.1);
            double_arr[(idx + 1) % N] = cos(temp); /* Different index */
        }
    }
    
    /* Additional loop with output dependencies on array elements */
    for (int i = 0; i < limit; i += 2) {
        /* Multiple writes to same array element - WAW */
        float_arr[i] = (float)i * 2.0f;
        float_arr[i] = float_arr[i] * float_arr[i]; /* Overwrite */
        float_arr[i] = sqrtf(float_arr[i]); /* Another overwrite */
        
        /* Register pressure with anti-dependencies */
        int r1 = int_arr[i];
        int r2 = r1 * 2; /* Uses r1 */
        r1 = i * 3; /* Overwrites r1 - WAR on register */
        int_arr[i] = r1 + r2;
    }
    
    /* Aggregate results to prevent elimination */
    for (int i = 0; i < N; i++) {
        result += int_arr[i];
        result += (int)float_arr[i];
        result += (int)double_arr[i];
    }
    
    return result % 256;
}
