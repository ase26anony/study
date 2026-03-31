/* ddg_test.c - Test program for DDG edge creation coverage */
#include <stdlib.h>
#include <stdio.h>

/* Global variables for function call dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Static helper function with side effects (Requirement 5) */
static void update_global(int* arr, int idx) {
    global_counter++;
    global_accumulator += arr[idx] * 0.5f;
    arr[idx] = global_counter;  /* Creates memory dependency */
}

/* Another static function for mixed operations */
static float mixed_operation(int a, float b) {
    volatile float result;  /* Prevent optimization */
    result = (a * 2) + (b * 3.14f);
    return result;
}

int main(void) {
    const int N = 100;
    volatile int limit = N;  /* Volatile to prevent optimization */
    
    /* Arrays with different data types */
    int int_arr[N];
    float float_arr[N];
    double double_arr[N];
    
    /* Non-linear index array (Requirement 4) */
    int indices[N];
    for (int i = 0; i < N; i++) {
        indices[i] = (i * i + i * 3) % N;  /* Non-affine index calculation */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i;
        float_arr[i] = i * 1.5f;
        double_arr[i] = i * 2.5;
    }
    
    volatile int result = 0;
    float temp_float = 0.0f;
    int temp_int = 0;
    
    /* Primary loop with various dependencies */
    for (int i = 1; i < limit; i++) {
        /* 1. Loop-carried true dependency (RAW) - Requirement 1 */
        int_arr[i] = int_arr[i-1] + 1;  /* True dependency chain */
        
        /* 2. Anti-dependency (WAR) - Requirement 2 */
        temp_int = int_arr[i];          /* Read */
        int_arr[i] = float_arr[i] > 0 ? 1 : 0;  /* Write later - creates WAR */
        
        /* 3. Output dependency (WAW) - Requirement 2 */
        float_arr[i] = mixed_operation(i, temp_float);  /* First write */
        float_arr[i] = global_accumulator + i;          /* Second write - creates WAW */
        
        /* 4. Conditional dependency patterns - Requirement 3 */
        if (i % 3 == 0) {
            /* Pattern A: Chain of dependencies */
            double_arr[i] = double_arr[i-1] * 1.1;
            temp_float = double_arr[i] + float_arr[i];
        } else if (i % 3 == 1) {
            /* Pattern B: Different dependency pattern */
            float_arr[i] = int_arr[i] * 2.0f;
            temp_int = float_arr[i];  /* Type conversion dependency */
        } else {
            /* Pattern C: Complex dependency web */
            temp_int = int_arr[i] + int_arr[i-1];
            float_arr[i] = temp_int * 0.5f;
            int_arr[i] = float_arr[i];  /* Type conversion */
        }
        
        /* 5. Function call with side effects - Requirement 5 */
        update_global(int_arr, i % 10);
        
        /* 6. Mixed data type operations - Requirement 6 */
        volatile double temp_double;  /* Prevent optimization */
        temp_double = int_arr[i] * 1.5 + float_arr[i] * 2.5 + double_arr[i];
        
        /* Additional anti-dependency with local variable */
        int local_var = temp_int;     /* Read temp_int */
        temp_int = i * 2;             /* Write temp_int - creates REG DEP WAR */
        
        /* Use local_var to prevent dead code elimination */
        result += local_var;
    }
    
    /* Nested loop with non-linear array access - Requirement 4 */
    for (int i = 0; i < N/2; i++) {
        for (int j = 0; j < 3; j++) {
            /* Access using non-affine indices */
            int idx = indices[(i * 3 + j) % N];
            float_arr[idx] = float_arr[idx] * 1.1f + j;
            
            /* Create output dependency in inner loop */
            double temp = double_arr[idx];
            double_arr[idx] = temp * 0.9;
            double_arr[idx] = double_arr[idx] + 1.0;  /* WAW */
        }
    }
    
    /* Aggregate results to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        result += int_arr[i];
        result += float_arr[i];
        result += double_arr[i];
    }
    
    result += global_counter;
    
    printf("Result: %d\n", result);
    return result > 0 ? 0 : 1;
}
