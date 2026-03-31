/* ddg_test.c - Program to exercise Data Dependency Graph edge creation */
#include <stdlib.h>

/* Global variables for function call dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Requirement 5: Function with side effects */
static inline void update_globals(int* arr, int idx, float* farr) {
    global_counter++;
    *farr += (float)(arr[idx] * global_counter);
    global_accumulator = *farr * 0.5f;
}

/* Another static function for anti-dependency patterns */
static void process_value(int* val, float* fval) {
    int temp = *val;          /* Read original value */
    *val = (*val) * 2 + 1;    /* Overwrite it - creates WAR */
    *fval = (float)temp / 2.0f; /* Use original value */
}

int main(void) {
    /* Requirement 1 & 6: Arrays with different data types */
    const int N = 1024;
    int int_arr[N];
    float float_arr[N];
    double double_arr[N];
    volatile int limit = N;  /* Prevent optimization */
    
    /* Requirement 4: Non-linear index array */
    int nonlin_idx[N];
    for (int i = 0; i < N; i++) {
        nonlin_idx[i] = (i * i + i * 3) % N;  /* Non-affine index calculation */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i;
        float_arr[i] = (float)i * 0.5f;
        double_arr[i] = (double)i * 0.25;
    }
    
    volatile int result = 0;
    
    /* Primary loop with multiple dependency types */
    for (int i = 1; i < limit; i++) {
        /* Requirement 1: Loop-carried true dependency (RAW) */
        int_arr[i] = int_arr[i-1] + i;  /* True dependency chain */
        
        /* Requirement 2: Anti-dependency (WAR) on array elements */
        int temp_val = int_arr[i];           /* Read */
        int_arr[i] = float_arr[i] > 0 ? temp_val * 2 : 0;  /* Write later - WAR */
        
        /* Requirement 2: Output dependency (WAW) on local variable */
        float local_var = (float)int_arr[i] * 1.5f;  /* First write */
        if (i % 3 == 0) {
            local_var = float_arr[i] * 2.0f;         /* Second write - WAW */
        }
        
        /* Requirement 3: Conditional dependency patterns */
        if (i % 2 == 0) {
            /* Pattern A: Chain of operations */
            float_arr[i] = float_arr[i-1] + local_var;
            double_arr[i] = (double)float_arr[i] * 0.75;
        } else {
            /* Pattern B: Different dependency structure */
            double_arr[i] = double_arr[i-1] * 1.1;
            float_arr[i] = (float)double_arr[i] + int_arr[i];
            
            /* Requirement 2: More anti-dependencies */
            process_value(&int_arr[i], &float_arr[i]);
        }
        
        /* Requirement 5: Function call with side effects */
        update_globals(int_arr, i, &float_arr[i]);
        
        /* Requirement 6: Mixed data type operations */
        double mixed_result = (double)int_arr[i] * 0.33 
                            + (double)float_arr[i] * 0.67
                            + double_arr[i];
        
        /* Use result in next iteration for additional dependency */
        if (i > 1) {
            double_arr[i] += mixed_result * (double)(i % 5);
        }
    }
    
    /* Requirement 4: Separate loop with non-linear array access */
    for (int i = 0; i < limit / 2; i++) {
        int idx = nonlin_idx[i];
        
        /* Complex dependency chain with non-affine access */
        float_arr[idx] = float_arr[nonlin_idx[i+1]] * 1.1f;
        
        /* Output dependency on same location */
        if (idx % 4 == 0) {
            float_arr[idx] = (float)int_arr[idx] * 2.5f;  /* WAW */
        }
        
        /* Update using function call */
        update_globals(int_arr, idx, &float_arr[idx]);
    }
    
    /* Nested loops for additional DDG complexity */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int index = i * 10 + j;
            if (index < N) {
                /* Cross-iteration dependency in inner loop */
                float_arr[index] += (j > 0) ? float_arr[index - 1] * 0.5f : 0.0f;
                
                /* Memory dependency through global */
                int_arr[index] += global_counter;
            }
        }
    }
    
    /* Aggregate results to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        result += int_arr[i];
        result += (int)float_arr[i];
        result += (int)double_arr[i];
    }
    
    result += global_counter;
    
    return result;
}
