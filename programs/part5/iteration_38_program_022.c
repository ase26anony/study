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

/* Static helper function with side effects (Requirement 5) */
static void update_global(int* arr, int idx) {
    global_counter++;
    global_accumulator += (float)arr[idx];
    arr[idx] += global_counter % 7;  /* Modify array element */
}

/* Another static function for mixed operations */
static float mixed_operation(int a, float b) {
    volatile float result;  /* Prevent optimization */
    result = (float)a * 1.5f + b * 2.3f;
    return result;
}

int main(void) {
    /* Declare arrays with different data types (Requirement 6) */
    const int N = 1024;
    int int_array[N];
    float float_array[N];
    double double_buffer[64];
    volatile int loop_limit = N;  /* Prevent constant propagation */
    
    /* Non-linear index array (Requirement 4) */
    int nonlin_idx[N];
    for (int i = 0; i < N; i++) {
        nonlin_idx[i] = (i * i + i * 3 + 7) % N;  /* Quadratic index mapping */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_array[i] = i % 100;
        float_array[i] = (float)i * 0.5f;
    }
    
    /* Primary loop with multiple dependency types */
    int prev_val = int_array[0];
    float prev_float = float_array[0];
    
    for (int i = 1; i < loop_limit; i++) {
        /* 1. LOOP-CARRIED TRUE DEPENDENCY (RAW) - Requirement 1 */
        int_array[i] = prev_val + int_array[i-1];  /* True dependency chain */
        prev_val = int_array[i];  /* Feed forward to next iteration */
        
        /* 2. ANTI-DEPENDENCY (WAR) - Requirement 2 */
        float temp = float_array[i];  /* Read before write */
        float_array[i] = (float)int_array[i] * 0.7f;  /* Overwrite same location */
        double_buffer[i % 64] = (double)temp;  /* Use the read value */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Requirement 2 */
        float_array[i] = float_array[i] * 2.0f;  /* Multiple writes to same location */
        
        /* 4. CONDITIONAL DEPENDENCIES - Requirement 3 */
        if (i % 3 == 0) {
            /* One dependency pattern */
            prev_float = float_array[i] + prev_float;
            int_array[i] = (int)prev_float;
        } else if (i % 3 == 1) {
            /* Different pattern creating cross-iteration dependency */
            float_array[i] = prev_float * 0.5f;
            prev_float = float_array[i] + 1.0f;
        } else {
            /* Third pattern with anti-dependency */
            float tmp = prev_float;
            prev_float = float_array[i];
            float_array[i] = tmp;
        }
        
        /* 5. FUNCTION CALL WITH SIDE EFFECTS - Requirement 5 */
        update_global(int_array, i % 128);
        
        /* 6. MIXED DATA TYPE OPERATIONS - Requirement 6 */
        float mixed_result = mixed_operation(int_array[i], float_array[i]);
        double_buffer[i % 64] += (double)mixed_result;
        
        /* Additional output dependency on local variable */
        volatile int local_var;  /* Prevent optimization */
        local_var = i * 2;
        local_var = i * 3;  /* WAW on local_var */
    }
    
    /* Nested loop with non-linear array access (Requirement 4) */
    double sum = 0.0;
    for (int i = 0; i < N; i += 8) {
        for (int j = 0; j < 8; j++) {
            /* Access using non-linear indices */
            int idx = nonlin_idx[i + j];
            sum += (double)int_array[idx] + (double)float_array[idx];
            
            /* Create loop-carried dependency in inner loop */
            if (j > 0) {
                int_array[idx] += int_array[nonlin_idx[i + j - 1]] % 17;
            }
        }
    }
    
    /* Complex dependency chain with multiple data types */
    float chain_result = 0.0f;
    for (int i = 1; i < 100; i++) {
        /* Chain of dependencies through different operations */
        int int_val = int_array[i % N] * 2 - 5;
        float float_val = (float)int_val / 3.0f + chain_result;
        double double_val = (double)float_val * 1.7;
        chain_result = (float)(double_val + (double)(i % 10));
        
        /* Anti-dependency in the chain */
        float old_chain = chain_result;
        chain_result = mixed_operation(i, chain_result);
        double_buffer[i % 64] = (double)old_chain;  /* Use old value */
    }
    
    /* Prevent dead code elimination */
    volatile double final_result = 0.0;
    final_result += sum;
    final_result += (double)chain_result;
    final_result += (double)global_accumulator;
    final_result += (double)global_counter;
    
    /* Use all arrays to prevent optimization */
    for (int i = 0; i < N; i++) {
        final_result += (double)int_array[i] + (double)float_array[i];
    }
    
    printf("Result: %f\n", final_result);
    return (int)final_result % 256;
}
