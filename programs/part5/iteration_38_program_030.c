/* ddg_coverage.c
 * Complex dependency patterns to exercise DDG edge creation in GCC scheduler
 */

#include <stdlib.h>
#include <stdio.h>

/* Global variables for memory dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Function with side effects (Requirement 5) */
static void update_global(int *arr, int idx) {
    global_counter++;
    global_accumulator += arr[idx] * 0.5f;
    arr[idx] = global_counter;  /* Creates memory dependency */
}

/* Another function with output dependency */
static inline float transform_value(float x, int scale) {
    volatile float result;  /* Prevent optimization */
    result = x * scale;     /* First write */
    result = result / 2.0f; /* Second write to same location (WAW) */
    return result;
}

/* Non-linear index computation */
static int nonlinear_index(int i, int j) {
    return (i * i + j * 3) % 256;  /* Non-affine access pattern */
}

int main(void) {
    const int N = 256;
    volatile int limit = N;  /* Prevent loop unrolling */
    
    /* Arrays with different data types (Requirement 6) */
    int int_arr[N];
    float float_arr[N];
    double double_arr[N];
    int index_map[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i;
        float_arr[i] = i * 0.5f;
        double_arr[i] = i * 0.25;
        index_map[i] = (i * 17 + 31) % N;  /* Non-linear mapping */
    }
    
    volatile int result = 0;
    float tmp_float = 0.0f;
    int tmp_int = 0;
    
    /* Primary loop with complex dependencies */
    for (int i = 1; i < limit; i++) {
        /* 1. Loop-carried true dependency (RAW) - Requirement 1 */
        int_arr[i] = int_arr[i-1] + i;  /* True dependency chain */
        
        /* 2. Anti-dependency (WAR) on local variable - Requirement 2 */
        tmp_int = int_arr[i];           /* Read */
        int_arr[i] = float_arr[i] > 0 ? 1 : -1;  /* Write after read */
        result += tmp_int;              /* Use the read value */
        
        /* 3. Conditional dependency patterns - Requirement 3 */
        if (i % 3 == 0) {
            /* Pattern A: Longer dependency chain */
            float_arr[i] = float_arr[i-1] * 1.1f + float_arr[i-2] * 0.9f;
            tmp_float = float_arr[i];
            float_arr[i] = transform_value(tmp_float, i);  /* WAW inside function */
        } else if (i % 3 == 1) {
            /* Pattern B: Different dependency structure */
            float_arr[i] = (float_arr[i] + float_arr[i/2]) / 2.0f;
            int_arr[i] = (int)(float_arr[i] * 100);
        } else {
            /* Pattern C: Cross-type dependencies */
            double_arr[i] = double_arr[i-1] + int_arr[i] * 0.01;
            float_arr[i] = (float)(double_arr[i] + double_arr[i-1]);
        }
        
        /* 4. Output dependency (WAW) on local variable - Requirement 2 */
        tmp_float = float_arr[i] * 2.0f;    /* First write to tmp_float */
        tmp_float = sqrtf(fabsf(tmp_float)); /* Second write to same variable */
        
        /* 5. Function call with memory side effects - Requirement 5 */
        update_global(int_arr, i % 128);
        
        /* 6. Mixed data type operations - Requirement 6 */
        double intermediate = int_arr[i] * 0.5 + float_arr[i] * 0.3;
        float_arr[i] = (float)(intermediate + global_accumulator * 0.1);
    }
    
    /* Nested loop with non-linear array access - Requirement 4 */
    for (int i = 0; i < N/2; i++) {
        for (int j = 0; j < 8; j++) {
            int idx = nonlinear_index(i, j);
            
            /* Complex access pattern with multiple dependencies */
            int_arr[idx] = int_arr[idx] + int_arr[nonlinear_index(i-1, j)];
            float_arr[idx] = float_arr[nonlinear_index(i, j-1)] * 1.5f;
            
            /* Anti-dependency in nested loop */
            double tmp = double_arr[idx];
            double_arr[idx] = int_arr[idx] * 0.01;
            result += (int)(tmp * 100);
        }
    }
    
    /* Additional loop with pointer aliasing possibilities */
    int *ptr1 = int_arr;
    int *ptr2 = int_arr + N/2;
    for (int i = 0; i < N/2; i++) {
        /* Potential memory dependencies through pointers */
        ptr1[i] = ptr2[i] + global_counter;
        ptr2[i] = ptr1[i] * 2 - ptr2[i];
        
        /* Volatile access to prevent optimization */
        result += ptr1[i] - ptr2[i];
    }
    
    /* Final aggregation with volatile to prevent dead code elimination */
    volatile int final_result = 0;
    for (int i = 0; i < N; i++) {
        final_result += int_arr[i] % 256;
        final_result += (int)(float_arr[i] * 10);
        final_result += (int)(double_arr[i] * 100);
    }
    
    final_result += global_counter;
    
    /* Use results to prevent optimization */
    printf("Result: %d (counter: %d, accumulator: %.2f)\n", 
           final_result, global_counter, global_accumulator);
    
    return final_result > 0 ? 0 : 1;
}
