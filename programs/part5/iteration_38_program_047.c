#include <stdlib.h>
#include <stdio.h>

/* Global variables for memory dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Function with side effects (Requirement 5) */
static void update_global(int* arr, int idx) {
    global_counter++;
    global_accumulator += (float)arr[idx];
    arr[idx] += global_counter % 7;  /* Modify array element */
}

/* Another static function for different data type operations */
static double compute_value(int i, float f) {
    return (double)i * 0.5 + (double)f * 1.5;
}

int main(void) {
    const int N = 100;
    volatile int limit = N;  /* Prevent optimization */
    
    /* Arrays with different data types */
    int arr_int[N];
    float arr_float[N];
    double arr_double[N];
    int results[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr_int[i] = i % 23;
        arr_float[i] = (float)i * 0.7f;
        arr_double[i] = (double)i * 1.3;
        results[i] = 0;
    }
    
    /* Non-linear index array (Requirement 4) */
    int nonlin_idx[N];
    for (int i = 0; i < N; i++) {
        nonlin_idx[i] = (i * i + i * 3 + 7) % N;  /* Quadratic mapping */
    }
    
    volatile int final_result = 0;
    
    /* PRIMARY LOOP with multiple dependency types */
    for (int i = 1; i < limit; i++) {
        /* 1. LOOP-CARRIED TRUE DEPENDENCY (RAW) - Requirement 1 */
        arr_int[i] = arr_int[i-1] + arr_int[i];  /* True dependency across iterations */
        
        /* Mixed data type operations - Requirement 6 */
        float temp_float = arr_float[i] * 2.0f;
        int temp_int = arr_int[i] + (int)temp_float;
        
        /* 2. ANTI-DEPENDENCY (WAR) - Requirement 2 */
        int read_first = arr_int[i];      /* Read */
        arr_int[i] = temp_int * 3;        /* Write later - anti-dependency */
        int use_read = read_first / 2;    /* Use the read value */
        
        /* 3. CONDITIONAL DEPENDENCY PATTERNS - Requirement 3 */
        if (i % 3 == 0) {
            /* Pattern A: Chain of dependencies */
            float chain = arr_float[i];
            chain = chain * 1.1f + (float)i;
            arr_float[i] = chain;
            results[i] = (int)chain + use_read;
        } else if (i % 3 == 1) {
            /* Pattern B: Different dependency structure */
            int tmp = results[i-1];       /* Another loop-carried dependency */
            tmp = tmp + arr_int[i];       /* Output dependency on tmp */
            results[i] = tmp * 2;
            
            /* Output dependency (WAW) - Requirement 2 */
            double x = compute_value(i, arr_float[i]);
            x = x * 0.8;                  /* Second write to x - output dependency */
            arr_double[i] = x;
        } else {
            /* Pattern C: More complex flow */
            volatile int vol_var = i;     /* Prevent optimization */
            arr_int[i] = vol_var + results[i-1];
            
            /* Function call with side effects - Requirement 5 */
            update_global(arr_int, i % 10);
        }
        
        /* 2. OUTPUT DEPENDENCY (WAW) within iteration */
        int output_var = arr_int[i] * 2;
        output_var = output_var + i;      /* Second write - output dependency */
        output_var = output_var % 17;     /* Third write - output dependency */
        results[i] += output_var;
        
        /* Mixed type dependency chain - Requirement 6 */
        double mixed = (double)arr_int[i] + (double)arr_float[i];
        arr_double[i] = mixed * 0.5;
    }
    
    /* SECONDARY LOOP with non-linear array access - Requirement 4 */
    for (int i = 0; i < limit - 5; i++) {
        /* Access using non-affine indices */
        int idx1 = nonlin_idx[i];
        int idx2 = nonlin_idx[i + 1];
        int idx3 = nonlin_idx[i + 2];
        
        /* Create dependencies through non-linear accesses */
        int val = arr_int[idx1] + arr_int[idx2];
        arr_int[idx3] = val * 2;
        
        /* Additional anti-dependency in this loop */
        float f_read = arr_float[idx1];
        arr_float[idx1] = (float)val * 0.3f;
        arr_double[i] += (double)f_read;
    }
    
    /* NESTED LOOP for more complex DDG */
    for (int i = 0; i < limit/2; i++) {
        for (int j = 1; j < 5; j++) {
            /* Cross-iteration dependency in inner loop */
            arr_float[i*5 + j] = arr_float[i*5 + j - 1] * 1.05f;
            
            /* Mixed operations */
            double dbl = (double)arr_float[i*5 + j] * 2.0;
            arr_double[i] += dbl;
        }
    }
    
    /* Aggregate results to prevent dead code elimination */
    for (int i = 0; i < limit; i++) {
        final_result += results[i] + (int)arr_float[i] + (int)arr_double[i];
    }
    
    final_result += global_counter;
    
    printf("Result: %d\n", final_result);
    return final_result % 256;
}
