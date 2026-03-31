#include <stdlib.h>

/* Global variables for memory dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Function with side effects (Requirement 5) */
static void update_globals(int *arr, float *farr, int idx) {
    global_counter++;
    global_accumulator += farr[idx % 8];
    arr[idx % 16] = global_counter;
}

/* Another function with memory side effects */
static inline void swap_values(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

int main() {
    /* Declare arrays with different data types (Requirement 6) */
    int arr_int[256];
    float arr_float[256];
    double arr_double[128];
    volatile int N = 128;  /* Prevent optimization */
    
    /* Non-linear index array (Requirement 4) */
    int indices[256];
    for (int i = 0; i < 256; i++) {
        indices[i] = (i * i + i * 3 + 7) % 256;  /* Quadratic non-affine */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        arr_int[i] = i;
        arr_float[i] = i * 1.5f;
        if (i < 128) arr_double[i] = i * 2.5;
    }
    
    int result_int = 0;
    float result_float = 0.0f;
    volatile int final_result = 0;  /* Prevent dead code elimination */
    
    /* PRIMARY LOOP with various dependencies */
    for (int i = 1; i < N; i++) {
        /* 1. Loop-carried true dependency (RAW) - Requirement 1 */
        arr_int[i] = arr_int[i-1] + 1;  /* Integer chain */
        arr_float[i] = arr_float[i-1] * 1.1f;  /* Float chain */
        
        /* 2. Anti-dependency (WAR) - Requirement 2 */
        int temp = arr_int[i];          /* Read arr_int[i] */
        arr_int[i] = i * 2;             /* Overwrite arr_int[i] later */
        result_int += temp;             /* Use the read value */
        
        /* Output dependency (WAW) - Requirement 2 */
        float f_temp = arr_float[i];
        f_temp = f_temp * 2.0f;         /* Multiple writes to f_temp */
        f_temp = f_temp + 1.0f;
        arr_float[i] = f_temp;
        
        /* 3. Conditional with different dependency patterns - Requirement 3 */
        if (i % 3 == 0) {
            /* Pattern A: More complex dependency chain */
            int x = arr_int[i];
            arr_int[i] = x + arr_int[i-1];
            result_float += arr_float[i] - arr_float[i-1];
        } else if (i % 3 == 1) {
            /* Pattern B: Different dependency pattern */
            arr_float[i] = arr_float[i] / 2.0f;
            arr_int[i] = (int)arr_float[i] + arr_int[i];
        } else {
            /* Pattern C: Cross-type dependencies */
            arr_int[i] = (int)(arr_float[i] * 2.0f);
            arr_float[i] = (float)arr_int[i] / 3.0f;
        }
        
        /* 4. Function call with side effects - Requirement 5 */
        update_globals(arr_int, arr_float, i);
        
        /* Mixed data type operations - Requirement 6 */
        double d_val = arr_double[i % 128];
        d_val = d_val * (double)arr_int[i];
        arr_double[i % 128] = d_val + (double)arr_float[i];
        
        /* Volatile to prevent optimization */
        volatile int barrier = i;
        (void)barrier;
    }
    
    /* SECONDARY LOOP with non-affine array access - Requirement 4 */
    for (int i = 0; i < N; i++) {
        int idx = indices[i];  /* Non-linear index */
        arr_int[idx] = arr_int[idx] * 2;
        
        /* Create anti-dependency in this loop too */
        float old_val = arr_float[idx];
        arr_float[idx] = old_val + (float)arr_int[idx];
        result_float += old_val;
        
        /* Call swap function creating memory dependencies */
        if (i > 0) {
            swap_values(&arr_int[indices[i]], &arr_int[indices[i-1]]);
        }
    }
    
    /* NESTED LOOP with complex dependencies */
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            /* Multi-dimensional like access with non-linear index */
            int idx = indices[i * 8 + j % 8];
            
            /* Loop-carried in inner loop */
            if (j > 0) {
                arr_float[idx] = arr_float[idx] + arr_float[indices[i * 8 + (j-1) % 8]];
            }
            
            /* Mixed type dependency chain */
            double d_temp = arr_double[j % 128];
            d_temp = d_temp * (i + 1);
            arr_int[idx] = (int)d_temp;
            arr_double[j % 128] = d_temp * 0.5;
        }
    }
    
    /* Aggregate results to prevent optimization */
    for (int i = 0; i < N; i++) {
        final_result += arr_int[i] + (int)arr_float[i];
    }
    
    final_result += global_counter + (int)global_accumulator;
    final_result += (int)result_float;
    
    return final_result % 256;
}
