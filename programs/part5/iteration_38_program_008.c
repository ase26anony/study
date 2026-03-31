/* ddg_test.c - Complex dependency patterns to trigger DDG edge creation */
#include <stdlib.h>
#include <stdio.h>

/* Global variables for function call dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Requirement 5: Function with side effects */
static inline void update_global(int idx, float* arr) {
    global_counter += idx;
    global_accumulator += arr[idx % 16];
    arr[idx % 16] = global_accumulator * 0.5f;
}

/* Requirement 4: Non-affine index array */
static const int non_linear_indices[32] = {
    0, 1, 3, 6, 10, 15, 21, 28, 4, 9, 14, 19, 24, 29, 2, 5,
    8, 11, 13, 16, 18, 20, 22, 23, 25, 26, 27, 30, 31, 12, 17, 7
};

/* Requirement 6: Mixed data type operations */
static float mixed_operation(int a, float b, double c) {
    volatile float result; /* Prevent optimization */
    result = (float)(a * 2) + b + (float)c;
    return result;
}

int main(void) {
    /* Requirement 1 & 6: Arrays with different data types */
    int int_arr[256] = {0};
    float float_arr[256] = {0.0f};
    double temp_double[16] = {0.0};
    
    /* Requirement 2: Variables for anti/output dependencies */
    int tmp_storage;
    float reuse_var;
    
    /* Requirement 6: Volatile loop limit */
    volatile int N = 128;
    volatile int result = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        int_arr[i] = i % 37;
        float_arr[i] = (float)(i * 0.73f);
    }
    
    /* PRIMARY LOOP with complex dependencies */
    for (int i = 1; i < N; i++) {
        /* Requirement 1: Loop-carried true dependency (RAW) */
        int_arr[i] = int_arr[i-1] * 2 + 1;
        
        /* Requirement 2: Anti-dependency (WAR) on array element */
        tmp_storage = int_arr[i];          /* Read */
        int_arr[i] = float_arr[i] > 0 ? tmp_storage + 1 : tmp_storage - 1; /* Write later */
        
        /* Requirement 2: Output dependency (WAW) on local variable */
        reuse_var = mixed_operation(i, float_arr[i], temp_double[i % 16]);
        reuse_var = reuse_var * 0.8f + (float)global_counter; /* Second write */
        
        /* Requirement 3: Conditional dependency patterns */
        if (i % 3 == 0) {
            /* Pattern A: Chain of dependencies */
            float chain = float_arr[i];
            chain = chain * chain;
            float_arr[i] = chain + (float)int_arr[i];
            tmp_storage = (int)chain;
        } else if (i % 3 == 1) {
            /* Pattern B: Different dependency structure */
            float_arr[i] = (float)int_arr[i] * 0.5f;
            int_arr[i] = (int)(float_arr[i] * 2.0f);
        } else {
            /* Pattern C: Cross-type dependencies */
            double d = (double)int_arr[i] + (double)float_arr[i];
            temp_double[i % 16] = d;
            int_arr[i] = (int)(d * 0.3);
        }
        
        /* Requirement 5: Function call with side effects */
        update_global(i, float_arr);
        
        /* Requirement 6: Mixed type operation feeding next iteration */
        if (i < N-1) {
            float_arr[i+1] = mixed_operation(int_arr[i], float_arr[i], 
                                           temp_double[(i+1) % 16]);
        }
    }
    
    /* Requirement 4: Nested loop with non-affine accesses */
    for (int outer = 0; outer < 8; outer++) {
        for (int inner = 0; inner < 32; inner++) {
            int idx = non_linear_indices[inner] + outer * 4;
            if (idx < 256) {
                /* Complex addressing creates conservative dependencies */
                float_arr[idx] = float_arr[non_linear_indices[(inner + 1) % 32]] 
                               * (float)int_arr[idx];
                
                /* Requirement 2: Another anti-dependency */
                int tmp = int_arr[idx];
                int_arr[idx] = (int)(float_arr[idx] * 100.0f);
                result += tmp; /* Use the read value */
            }
        }
    }
    
    /* Requirement 8: Aggregate results to prevent elimination */
    volatile int final_sum = 0;
    for (int i = 0; i < 256; i++) {
        final_sum += int_arr[i] + (int)float_arr[i];
    }
    final_sum += global_counter + (int)global_accumulator;
    
    printf("Result: %d (check: %d)\n", final_sum, result);
    return final_sum > 0 ? 0 : 1;
}
