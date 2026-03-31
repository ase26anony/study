#include <stdlib.h>

/* Global variables for memory dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Helper function with side effects (Requirement 5) */
static void update_globals(int *arr, float *farr, int idx) {
    global_counter++;
    global_accumulator += farr[idx] * 0.5f;
    arr[idx] += global_counter;
}

/* Another helper with output dependency pattern */
static inline float compute_value(float a, float b, int *reuse_var) {
    /* Create output dependency through reuse_var */
    float result = a + b;
    *reuse_var = (int)result;      /* First write */
    *reuse_var += (int)(a * b);    /* Second write to same location (WAW) */
    return result;
}

int main(void) {
    const int N = 100;
    volatile int limit = N;  /* Prevent optimization */
    
    /* Arrays with different data types (Requirement 6) */
    int arr_int[N];
    float arr_float[N];
    double arr_double[N];
    int temp_results[N];
    
    /* Non-linear index array (Requirement 4) */
    int nonlin_idx[N];
    for (int i = 0; i < N; i++) {
        nonlin_idx[i] = (i * i + i * 3) % N;  /* Non-affine index calculation */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr_int[i] = i;
        arr_float[i] = i * 1.5f;
        arr_double[i] = i * 2.5;
    }
    
    /* Primary loop with multiple dependency types */
    for (int i = 1; i < limit; i++) {
        /* 1. Loop-carried true dependency (RAW) - Requirement 1 */
        arr_int[i] = arr_int[i-1] + 1;  /* Integer chain */
        arr_float[i] = arr_float[i-1] * 1.1f;  /* Float chain */
        
        /* 2. Anti-dependency (WAR) and Output dependency (WAW) - Requirement 2 */
        int tmp_storage;  /* Will be reused for WAR/WAW */
        
        /* Anti-dependency pattern: read then write */
        tmp_storage = arr_int[i];          /* Read */
        arr_int[i] = temp_results[i-1];    /* Write to same location */
        temp_results[i] = tmp_storage;     /* Use the read value */
        
        /* Output dependency pattern: multiple writes */
        float x = arr_float[i];
        x = compute_value(x, 2.0f, &tmp_storage);  /* WAW inside function */
        arr_float[i] = x;
        
        /* 3. Conditional dependency paths - Requirement 3 */
        if (i % 3 == 0) {
            /* Path A: a -> b dependency */
            int a = arr_int[i];
            int b = a * 2;
            arr_int[i] = b + 1;
        } else if (i % 3 == 1) {
            /* Path B: b -> a dependency (different pattern) */
            int b = temp_results[i];
            int a = b + arr_int[i-1];
            arr_int[i] = a;
        } else {
            /* Path C: complex dependency chain */
            float f1 = arr_float[i];
            float f2 = arr_float[i-1];
            arr_float[i] = f1 * f2 + (float)arr_int[i];
        }
        
        /* 4. Mixed data type operations - Requirement 6 */
        double dbl_val = arr_double[i];
        dbl_val += (double)arr_int[i] * 0.25;
        dbl_val -= (double)arr_float[i] * 0.1;
        arr_double[i] = dbl_val;
        
        /* 5. Function call with side effects - Requirement 5 */
        update_globals(arr_int, arr_float, i % 10);
    }
    
    /* Nested loop with non-linear array access - Requirement 4 */
    for (int i = 0; i < limit/2; i++) {
        for (int j = 0; j < 5; j++) {
            int idx = nonlin_idx[(i * 5 + j) % N];
            /* Complex index calculation */
            idx = (idx * 13 + 7) % N;
            
            /* Access with non-affine index creates conservative dependencies */
            arr_int[idx] += arr_float[i] > 0 ? 1 : -1;
            arr_float[i] += arr_int[idx] * 0.01f;
        }
    }
    
    /* Additional loop with volatile to prevent optimization */
    volatile int final_result = 0;
    for (int i = 0; i < N; i++) {
        final_result += arr_int[i];
        final_result += (int)arr_float[i];
        final_result += (int)arr_double[i];
    }
    
    /* Use results to prevent dead code elimination */
    return final_result % 256;
}
