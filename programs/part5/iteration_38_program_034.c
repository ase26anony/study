/* Complex dependency pattern generator for DDG edge coverage */
#include <stdlib.h>

/* Global variables for memory dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Function with side effects (Requirement 5) */
static void update_global(int* arr, int idx) {
    global_counter++;
    arr[idx] += global_counter;
    global_accumulator += arr[idx] * 0.5f;
}

/* Another function with side effects */
static inline float compute_with_side_effect(float* farr, int i, int* mod) {
    static float last_value = 1.0f;
    float result = farr[i] * last_value;
    last_value = result * 0.9f;
    *mod = (int)result % 16;
    return result;
}

int main() {
    /* Declare arrays with different data types (Requirement 6) */
    const int N = 1024;
    volatile int limit = N;  /* Prevent optimization */
    int arr_int[N];
    float arr_float[N];
    double arr_double[N];
    int results[N];
    
    /* Non-linear index array (Requirement 4) */
    int nonlin_idx[N];
    for (int i = 0; i < N; i++) {
        nonlin_idx[i] = (i * i + i * 3 + 7) % N;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr_int[i] = i;
        arr_float[i] = i * 0.5f;
        arr_double[i] = i * 0.25;
        results[i] = 0;
    }
    
    volatile int final_result = 0;
    
    /* Primary loop with complex dependencies */
    for (int i = 1; i < limit; i++) {
        /* 1. Loop-carried true dependency (RAW) (Requirement 1) */
        arr_int[i] = arr_int[i-1] + i * 2;
        
        /* 2. Anti-dependency (WAR) on local variable */
        int tmp = arr_int[i];          /* Read arr_int[i] */
        arr_int[i] = arr_float[i] * 2; /* Write arr_int[i] - anti-dep on tmp */
        arr_float[i] = tmp * 0.3f;     /* Use tmp */
        
        /* 3. Output dependency (WAW) */
        float x = arr_float[i] * 1.5f;
        x = arr_float[i-1] * 2.5f;     /* WAW on x */
        
        /* 4. Conditional dependency patterns (Requirement 3) */
        if (i % 3 == 0) {
            /* Pattern A: Chain of dependencies */
            int chain = arr_int[i];
            chain = chain * 2 + 1;
            arr_float[i] = chain * 0.1f;
            results[i] = chain;
        } else if (i % 3 == 1) {
            /* Pattern B: Different dependency pattern */
            float fchain = arr_float[i];
            fchain = fchain * 3.14f - arr_float[i-1];
            arr_int[i] = (int)fchain;
            results[i] = (int)(fchain * 100);
        } else {
            /* Pattern C: Cross-type dependencies */
            double dval = arr_double[i];
            dval = dval + arr_int[i] * 0.01;
            arr_double[i] = dval;
            results[i] = (int)dval;
        }
        
        /* 5. Function call with side effects (Requirement 5) */
        update_global(arr_int, i % 128);
        
        /* 6. Mixed data type operations (Requirement 6) */
        int mod_result;
        float computed = compute_with_side_effect(arr_float, i, &mod_result);
        
        /* Complex expression with mixed types */
        arr_double[i] = (double)arr_int[i] * 0.5 + 
                       (double)arr_float[i] * 0.3 + 
                       computed * 0.2;
        
        /* 7. Another anti-dependency pattern */
        double save = arr_double[i];
        arr_double[i] = arr_double[nonlin_idx[i % 64]] * 1.1;
        results[i] += (int)(save * mod_result);
    }
    
    /* Nested loop with non-linear array access (Requirement 4) */
    for (int i = 0; i < limit/2; i++) {
        for (int j = 0; j < 8; j++) {
            /* Non-affine access pattern */
            int idx = nonlin_idx[(i * 8 + j) % N];
            float val = arr_float[idx];
            
            /* Loop-carried in inner loop */
            if (j > 0) {
                val += arr_float[nonlin_idx[(i * 8 + j - 1) % N]] * 0.5f;
            }
            
            /* Mixed operation */
            arr_double[idx] = val * (i + j) * 0.01;
            
            /* Conditional store */
            if ((i + j) % 4 == 0) {
                results[idx] = (int)(arr_double[idx] * 1000);
            }
        }
    }
    
    /* Aggregate results to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        final_result += results[i] + (int)arr_int[i] + 
                       (int)arr_float[i] + (int)arr_double[i];
    }
    
    final_result += global_counter + (int)global_accumulator;
    
    return final_result % 256;
}
