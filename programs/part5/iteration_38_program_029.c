/* ddg_coverage.c
 * Program designed to exercise Data Dependency Graph edge creation
 * in GCC's instruction scheduler
 */

#include <stdlib.h>
#include <math.h>

/* Global variables for function call dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Static helper function with side effects (Requirement 5) */
static void update_global(int* arr, int idx, float* farr) {
    global_counter++;
    *arr += global_counter;
    global_accumulator += *farr * 0.5f;
    *farr = global_accumulator;
}

/* Another static function for output dependencies */
static inline float transform_value(float x, int scale) {
    volatile float result; /* Prevent optimization */
    result = x * scale;
    result = sinf(result); /* WAW dependency on result */
    return result;
}

int main(void) {
    const int N = 1024;
    volatile int limit = N; /* Prevent constant propagation */
    
    /* Arrays with different data types (Requirement 6) */
    int int_arr[N];
    float float_arr[N];
    double double_arr[N];
    int results[N/2];
    
    /* Non-linear index array (Requirement 4) */
    int nonlin_idx[N];
    for (int i = 0; i < N; i++) {
        nonlin_idx[i] = (i * i + i * 3 + 7) % N; /* Non-affine index calculation */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i;
        float_arr[i] = i * 0.5f;
        double_arr[i] = i * 0.25;
    }
    
    int sum = 0;
    float fsum = 0.0f;
    
    /* Primary loop with various dependencies */
    for (int i = 1; i < limit; i++) {
        /* 1. Loop-carried true dependency (RAW) - Requirement 1 */
        int_arr[i] = int_arr[i-1] + i * 2; /* Integer chain */
        float_arr[i] = float_arr[i-1] * 1.1f + float_arr[i]; /* Float chain */
        
        /* 2. Anti-dependency (WAR) and Output dependency (WAW) - Requirement 2 */
        int tmp = int_arr[i]; /* Read */
        int_arr[i] = results[i % (N/2)]; /* Write - creates WAR */
        results[i % (N/2)] = tmp;
        
        float ftmp = float_arr[i];
        float_arr[i] = transform_value(ftmp, i); /* Function call with WAW inside */
        
        /* Output dependency on local variable */
        double dval = double_arr[i];
        dval = dval * 2.0; /* WAW on dval */
        double_arr[i] = dval;
        
        /* 3. Conditional dependencies - Requirement 3 */
        if (i % 3 == 0) {
            /* One dependency pattern */
            int_arr[i] = results[i % (N/2)] + global_counter;
            results[i % (N/2)] = int_arr[i] / 2;
        } else if (i % 3 == 1) {
            /* Different pattern with reversed flow */
            results[i % (N/2)] = int_arr[i] * 2;
            int_arr[i] = results[i % (N/2)] - global_counter;
        } else {
            /* Third pattern with memory dependencies */
            update_global(&int_arr[i], i, &float_arr[i]);
        }
        
        /* 5. Function call with side effects - Requirement 5 */
        update_global(&int_arr[i], i, &float_arr[i]);
        
        /* 6. Mixed data type operations - Requirement 6 */
        float mixed = int_arr[i] * 0.5f + float_arr[i];
        int_arr[i] = (int)(mixed * 2.0f) + global_counter;
        
        /* Accumulate for final result */
        sum += int_arr[i];
        fsum += float_arr[i];
    }
    
    /* Secondary loop with non-linear array access - Requirement 4 */
    for (int i = 0; i < N/2; i++) {
        int idx = nonlin_idx[i];
        
        /* Complex dependency chain with non-affine access */
        int_arr[idx] = int_arr[nonlin_idx[(i + 1) % N]] * 2;
        
        /* Cross-iteration dependency with non-linear pattern */
        if (i > 0) {
            float_arr[idx] = float_arr[nonlin_idx[i-1]] + int_arr[idx] * 0.01f;
        }
        
        /* Update results array */
        results[i] = int_arr[idx] + (int)float_arr[idx];
        
        /* Another function call */
        update_global(&results[i], idx, &float_arr[idx]);
    }
    
    /* Nested loop for additional DDG complexity */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < N/10; j++) {
            int idx = i * (N/10) + j;
            
            /* Multi-dimensional like access pattern */
            double_arr[idx] = double_arr[idx] + int_arr[idx] * 0.5;
            
            /* Conditional with dependencies */
            if (j % 2 == 0) {
                float_arr[idx] = float_arr[idx] * 2.0f - double_arr[idx];
            } else {
                double_arr[idx] = double_arr[idx] / 2.0 + float_arr[idx];
            }
        }
    }
    
    /* Final aggregation to prevent dead code elimination */
    volatile int final_result = 0;
    for (int i = 0; i < N; i++) {
        final_result += int_arr[i];
        final_result += (int)float_arr[i];
        final_result += (int)double_arr[i];
    }
    for (int i = 0; i < N/2; i++) {
        final_result += results[i];
    }
    
    final_result += global_counter + (int)global_accumulator;
    
    return final_result % 256; /* Prevent overflow in return */
}
