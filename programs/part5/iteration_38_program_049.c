/* ddg_coverage.c
 * Program designed to exercise Data Dependency Graph edge creation
 * in GCC's instruction scheduler
 */

#include <stdlib.h>
#include <stdio.h>

/* Global variables for function call dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Static function with side effects (Requirement 5) */
static void update_globals(int* arr, int idx, float* farr) {
    global_counter++;
    global_accumulator += farr[idx % 8];
    arr[idx % 16] += global_counter;
}

/* Another static function for output dependencies */
static inline float transform_value(float x, int scale) {
    static float last_result = 0.0f;
    float result = x * scale + last_result;
    last_result = result;  /* Creates output dependency between calls */
    return result;
}

/* Non-linear index mapping array (Requirement 4) */
static const int non_linear_indices[16] = {
    0, 1, 4, 9, 16, 25, 36, 49,
    3, 6, 10, 15, 21, 28, 36, 45
};

int main(void) {
    /* Declare arrays with different data types (Requirement 6) */
    int int_array[256] = {0};
    float float_array[256] = {0.0f};
    double double_array[128] = {0.0};
    volatile int N = 100;  /* Volatile to prevent optimization */
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        int_array[i] = i;
        float_array[i] = i * 0.5f;
        if (i < 128) {
            double_array[i] = i * 0.25;
        }
    }
    
    volatile int result = 0;
    
    /* PRIMARY LOOP with complex dependencies */
    for (int i = 1; i < N; i++) {
        /* 1. Loop-carried true dependency (RAW) - Requirement 1 */
        int_array[i] = int_array[i-1] + i * 2;
        
        /* Mixed data type operations feeding each other - Requirement 6 */
        float temp_float = float_array[i] * 1.5f;
        int temp_int = (int)temp_float + int_array[i];
        
        /* 2. Anti-dependency (WAR) pattern - Requirement 2 */
        int read_first = int_array[i];      /* Read */
        int_array[i] = temp_int * 3;        /* Write to same location */
        float_array[i] = read_first * 0.7f; /* Use the read value */
        
        /* 3. Conditional dependency patterns - Requirement 3 */
        if (i % 3 == 0) {
            /* Path A: Chain of dependencies */
            float chain_val = float_array[i] + global_accumulator;
            int_array[i] += (int)chain_val;
            float_array[i] = chain_val * 0.8f;
        } else if (i % 3 == 1) {
            /* Path B: Different dependency pattern */
            int tmp = int_array[i];
            int_array[i] = global_counter + i;
            global_counter = tmp;  /* Creates loop-carried dependency */
        } else {
            /* Path C: Output dependency (WAW) - Requirement 2 */
            float x = transform_value(float_array[i], i);
            x = transform_value(x, i+1);  /* Second write to x - WAW */
            float_array[i] = x;
        }
        
        /* 5. Function call with side effects - Requirement 5 */
        update_globals(int_array, i, float_array);
        
        /* More mixed-type operations */
        double_array[i % 128] = double_array[i % 128] + 
                               (double)int_array[i] * 0.01 +
                               (double)float_array[i] * 0.02;
    }
    
    /* SECONDARY LOOP with non-linear array access - Requirement 4 */
    for (int i = 0; i < 32; i++) {
        int idx = non_linear_indices[i % 16] % 256;
        
        /* Complex addressing with multiple dependencies */
        int base_idx = int_array[idx] % 128;
        float_array[idx] = float_array[base_idx] * 2.0f;
        
        /* Another anti-dependency pattern */
        double temp_dbl = double_array[base_idx];
        double_array[base_idx] = (double)int_array[idx] * 1.5;
        int_array[idx] += (int)temp_dbl;
        
        /* Call side-effect function again */
        update_globals(int_array, idx, float_array);
    }
    
    /* NESTED LOOP with output dependencies */
    for (int i = 0; i < 50; i++) {
        int output_var = i * 2;
        output_var = output_var + global_counter;  /* WAW on output_var */
        
        for (int j = 0; j < 10; j++) {
            /* Loop-carried in inner loop */
            float_array[(i*10 + j) % 256] = 
                float_array[(i*10 + j - 1) % 256] * 1.1f;
            
            /* Mixed operations creating various edges */
            double local_dbl = double_array[j % 128];
            int local_int = (int)local_dbl + output_var;
            output_var = local_int % 1000;  /* Another WAW */
        }
    }
    
    /* Aggregate results to prevent dead code elimination */
    for (int i = 0; i < 256; i++) {
        result += int_array[i];
        result += (int)float_array[i];
        if (i < 128) {
            result += (int)double_array[i];
        }
    }
    
    result += global_counter;
    
    printf("Result: %d\n", result);
    return result;
}
