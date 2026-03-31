/* Compile with: gcc -O2 -fschedule-insns -fdump-rtl-sched1 -fdump-rtl-sched2 -fmodulo-sched -fdump-rtl-sms -fsched-verbose=5 -fsel-sched-pipelining -fselective-scheduling2 -o ddg_test ddg_test.c */

#include <stdlib.h>
#include <stdio.h>

#define N 1024
#define M 32

/* Global variables for function call dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Requirement 5: Function with side effects */
static inline void update_global(int idx, float *arr) {
    global_counter += idx;
    global_accumulator += arr[idx % M];
    arr[idx % M] += 0.5f;
}

/* Requirement 4: Non-affine index array */
static const int non_linear_idx[M] = {
    0, 1, 3, 6, 10, 15, 21, 28, 4, 9,
    16, 25, 5, 12, 20, 29, 7, 14, 22, 31,
    8, 17, 27, 2, 11, 19, 30, 13, 23, 18, 26, 24
};

int main(void) {
    /* Requirement 1 & 6: Arrays with different data types */
    int int_arr[N] = {0};
    float float_arr[N] = {0.0f};
    double double_buffer[M] = {0.0};
    
    /* Requirement 2: Variables for anti/output dependencies */
    int tmp_int, tmp_int2;
    float tmp_float;
    volatile int loop_limit = N;  /* Prevent optimization */
    
    /* Requirement 1: Loop-carried true dependency (RAW) */
    for (int i = 1; i < loop_limit; i++) {
        /* Integer chain with RAW dependency */
        int_arr[i] = int_arr[i-1] + i * 2;  /* True dependency */
        
        /* Requirement 2: Anti-dependency (WAR) on local variable */
        tmp_int = int_arr[i];          /* Read */
        int_arr[i] = float_arr[i] * 2; /* Write to same location - creates anti-dep */
        float_arr[i] = tmp_int * 0.5f; /* Use the read value */
        
        /* Requirement 2: Output dependency (WAW) */
        tmp_float = global_accumulator * 0.1f;
        tmp_float = tmp_int * 0.25f;  /* Second write to tmp_float - WAW */
        
        /* Requirement 3: Conditional dependency patterns */
        if (i % 3 == 0) {
            /* Pattern A: Longer dependency chain */
            double_buffer[i % M] = double_buffer[(i-1) % M] * 1.1;
            tmp_int2 = int_arr[i] * 2;
        } else if (i % 3 == 1) {
            /* Pattern B: Different anti-dependency */
            tmp_int2 = float_arr[i];
            float_arr[i] = int_arr[i] * 0.3f;
            int_arr[i] = tmp_int2;
        } else {
            /* Pattern C: Output dependency chain */
            tmp_int2 = i * i;
            tmp_int2 = tmp_int2 + global_counter;  /* WAW */
        }
        
        /* Requirement 5: Function call with side effects */
        update_global(i, float_arr);
        
        /* Requirement 6: Mixed data type operations feeding each other */
        float_arr[i] = float_arr[i] + int_arr[i] * 0.7f;
        if (i % 8 == 0) {
            double_buffer[i % M] = float_arr[i] * int_arr[i] * 0.01;
        }
    }
    
    /* Requirement 4: Separate loop with non-affine array access */
    for (int i = 0; i < M * 4; i++) {
        int idx = non_linear_idx[i % M] + (i / M) * 7;
        if (idx < N) {
            float_arr[idx] = float_arr[idx] * 1.05f + i;
        }
    }
    
    /* Requirement 8: Aggregate results to prevent elimination */
    volatile int result = 0;
    volatile float f_result = 0.0f;
    
    for (int i = 0; i < N; i++) {
        result += int_arr[i];
        f_result += float_arr[i];
    }
    
    result += global_counter + (int)f_result;
    
    /* Also use double_buffer to prevent elimination */
    for (int i = 0; i < M; i++) {
        f_result += double_buffer[i];
    }
    
    printf("Result: %d\n", result);
    return result % 256;
}
