/* Complex dependency pattern generator for DDG edge coverage */
#include <stdlib.h>

/* Global variables for side-effect dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Function with side effects (Requirement 5) */
static void update_global(int *arr, int idx) {
    global_counter++;
    global_accumulator += arr[idx] * 0.5f;
    arr[idx] = global_counter % 100;
}

/* Another function with memory side effects */
static inline void swap_values(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
    global_accumulator += 1.0f;
}

/* Main function with complex dependency patterns */
int main() {
    /* Declare arrays with different data types (Requirement 6) */
    int arr_int[256];
    float arr_float[256];
    double arr_double[256];
    volatile int N = 128;  /* Prevent optimization */
    
    /* Non-linear index array (Requirement 4) */
    int indices[256];
    for (int i = 0; i < 256; i++) {
        indices[i] = (i * i + i * 3 + 7) % 256;  /* Quadratic non-affine */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        arr_int[i] = i;
        arr_float[i] = i * 0.5f;
        arr_double[i] = i * 0.25;
    }
    
    volatile int result = 0;
    float temp_float = 0.0f;
    int temp_int = 0;
    
    /* PRIMARY LOOP with multiple dependency types */
    for (int i = 1; i < N; i++) {
        /* 1. Loop-carried true dependency (RAW) - Requirement 1 */
        arr_int[i] = arr_int[i-1] + 1;  /* Integer chain */
        arr_float[i] = arr_float[i-1] * 1.1f;  /* Float chain */
        
        /* 2. Anti-dependency (WAR) - Requirement 2 */
        int read_first = arr_int[i];      /* Read */
        arr_int[i] = i * 2;               /* Write after read */
        result += read_first;             /* Use the read value */
        
        /* 3. Output dependency (WAW) - Requirement 2 */
        temp_float = arr_float[i] * 2.0f; /* First write */
        temp_float = temp_float / 1.5f;   /* Second write to same variable */
        arr_float[i] = temp_float;
        
        /* 4. Conditional dependency patterns - Requirement 3 */
        if (i % 3 == 0) {
            /* Pattern A: Cross iteration dependency */
            arr_double[i] = arr_double[i-2] + arr_double[i-1];
            temp_int = arr_int[i];
        } else if (i % 3 == 1) {
            /* Pattern B: Different dependency chain */
            arr_int[i] = temp_int * 2;  /* Uses value from previous pattern */
            temp_int = arr_int[i] / 2;
        } else {
            /* Pattern C: Circular dependency simulation */
            int tmp = arr_int[i];
            arr_int[i] = temp_int;
            temp_int = tmp;
        }
        
        /* 5. Function call with side effects - Requirement 5 */
        update_global(arr_int, i % 128);
        
        /* 6. Mixed data type operations - Requirement 6 */
        double mixed_calc = arr_int[i] * 0.5 + arr_float[i];
        arr_double[i] = mixed_calc + arr_double[i-1];
        
        /* Anti-dependency with pointer aliasing */
        int *ptr1 = &arr_int[i];
        int *ptr2 = &arr_int[(i + 1) % 128];
        swap_values(ptr1, ptr2);
    }
    
    /* SECONDARY LOOP with non-affine array accesses - Requirement 4 */
    for (int i = 0; i < N; i++) {
        int idx = indices[i];  /* Non-linear access pattern */
        
        /* Create dependencies through non-affine indices */
        if (idx > 0) {
            arr_int[idx] = arr_int[indices[idx-1]] + arr_int[idx];
        }
        
        /* Mixed type computation with non-affine access */
        arr_float[idx] = arr_float[idx] + arr_int[i] * 0.25f;
        
        /* Function call with non-affine parameter */
        update_global(arr_int, idx);
    }
    
    /* NESTED LOOP with complex dependencies */
    for (int i = 1; i < 64; i++) {
        for (int j = 1; j < 64; j++) {
            /* Multi-dimensional dependency */
            int linear_idx = i * 64 + j;
            int prev_idx = (i-1) * 64 + j;
            
            /* 2D loop-carried dependency */
            arr_int[linear_idx] = arr_int[prev_idx] + arr_int[linear_idx - 1];
            
            /* Anti-dependency in 2D */
            float old_val = arr_float[linear_idx];
            arr_float[linear_idx] = arr_int[linear_idx] * 0.5f;
            arr_double[linear_idx] = old_val;
            
            /* Conditional with 2D pattern */
            if ((i + j) % 4 == 0) {
                swap_values(&arr_int[linear_idx], &arr_int[prev_idx]);
            }
        }
    }
    
    /* Final aggregation to prevent dead code elimination */
    volatile int final_result = 0;
    for (int i = 0; i < 256; i++) {
        final_result += arr_int[i];
        final_result += (int)arr_float[i];
        final_result += (int)arr_double[i];
    }
    final_result += global_counter;
    final_result += (int)global_accumulator;
    
    return final_result % 256;
}
