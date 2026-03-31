/* ddg_edge_coverage.c
 * Program designed to exercise DDG edge creation in GCC's instruction scheduler
 * Compile with: gcc -O2 -fschedule-insns -fdump-rtl-sched1 -fdump-rtl-sched2 ddg_edge_coverage.c -o ddg_test
 * Or for modulo scheduling: gcc -O3 -fmodulo-sched -fdump-rtl-sms ddg_edge_coverage.c -o ddg_test
 */

#include <stdlib.h>
#include <stdio.h>

/* Global variables for function call dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Requirement 5: Function with side effects */
static inline void update_global(int *arr, int idx) {
    global_counter++;
    global_accumulator += arr[idx] * 0.5f;
    arr[idx] = global_counter % 100;
}

/* Another static function for mixed dependencies */
static double process_value(double x, int scale) {
    volatile static int call_count = 0;  /* Prevent optimization */
    call_count++;
    return x * scale * (1.0 + call_count * 0.001);
}

int main(void) {
    /* Requirement 1 & 6: Arrays with different data types */
    const int N = 1024;
    int int_arr[N];
    float float_arr[N];
    double double_arr[N];
    volatile int limit = N;  /* Volatile to prevent optimization */
    
    /* Requirement 4: Non-linear index array */
    int nonlin_idx[N];
    for (int i = 0; i < N; i++) {
        nonlin_idx[i] = (i * i + i * 3) % N;  /* Non-affine index calculation */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i % 100;
        float_arr[i] = i * 0.5f;
        double_arr[i] = i * 0.25;
    }
    
    /* Primary loop with multiple dependency types */
    for (int i = 1; i < limit; i++) {
        /* Requirement 1: Loop-carried true dependency (RAW) */
        int_arr[i] = int_arr[i-1] + 2;  /* True dependency chain */
        
        /* Requirement 2: Anti-dependency (WAR) on same array */
        int temp = int_arr[i];          /* Read */
        int_arr[i] = float_arr[i] > 0 ? temp * 2 : temp / 2;  /* Write later */
        
        /* Requirement 2: Output dependency (WAW) on local variable */
        float f_temp = float_arr[i] * 1.5f;
        f_temp = process_value(f_temp, i);  /* Second write to f_temp */
        
        /* Requirement 3: Conditional dependency patterns */
        if (i % 3 == 0) {
            /* Pattern A: More complex dependency chain */
            double_arr[i] = double_arr[i-1] * 1.01;
            float_arr[i] = float_arr[i-1] + float_arr[i-2];
        } else if (i % 3 == 1) {
            /* Pattern B: Different dependency pattern */
            float_arr[i] = double_arr[i] * 2.0f;
            int_arr[i] = float_arr[i] * 0.5;
        } else {
            /* Pattern C: Cross-type dependencies */
            int_arr[i] = float_arr[i] + double_arr[i];
            double_arr[i] = int_arr[i] * 0.75;
        }
        
        /* Requirement 5: Function call with side effects */
        update_global(int_arr, i % 100);
        
        /* Requirement 6: Mixed data type operations */
        double mixed_result = int_arr[i] * 0.5 + float_arr[i] * 1.5;
        volatile double vol_double = mixed_result;  /* Prevent optimization */
        
        /* Requirement 2: More anti-dependencies with array reuse */
        float old_float = float_arr[i % 50];  /* Read from one location */
        float_arr[i % 50] = old_float * global_accumulator;  /* Write to same */
        
        /* Nested loop for non-linear accesses */
        if (i % 100 == 0) {
            /* Requirement 4: Access with non-affine indices */
            for (int j = 0; j < 10; j++) {
                int idx = nonlin_idx[(i + j) % N];
                double_arr[idx] += int_arr[j] * 0.33;
                
                /* Create output dependency in nested loop */
                volatile int out_var = j * 2;
                out_var = j * 3;  /* WAW on out_var */
            }
        }
    }
    
    /* Additional loop with pointer aliasing possibilities */
    int *ptr1 = int_arr;
    int *ptr2 = int_arr + N/2;
    for (int i = 0; i < N/2; i++) {
        /* Potential pointer aliasing creates conservative dependencies */
        ptr1[i] = ptr2[i] + global_counter;
        ptr2[i] = ptr1[i] * 2;
    }
    
    /* Final aggregation to prevent dead code elimination */
    volatile int final_result = 0;
    volatile float final_float = 0.0f;
    volatile double final_double = 0.0;
    
    for (int i = 0; i < N; i++) {
        final_result += int_arr[i];
        final_float += float_arr[i];
        final_double += double_arr[i];
    }
    
    /* Use results to prevent optimization */
    printf("Results: %d %.2f %.2f\n", 
           final_result, final_float, final_double);
    
    return final_result % 256;
}
