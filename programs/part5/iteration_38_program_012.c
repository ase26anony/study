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

/* Another function with anti-dependency pattern */
static inline float reuse_variable(float x, float y) {
    float tmp = x;      /* Read x */
    x = y * 2.0f;       /* Overwrite x - creates WAR */
    return tmp + x;     /* Use original value */
}

int main() {
    /* Volatile to prevent optimization (Requirement 6) */
    volatile int N = 100;
    const int M = 10;
    
    /* Arrays with different data types */
    int arr_int[100];
    float arr_float[100];
    double arr_double[100];
    int results[100];
    
    /* Non-linear index array (Requirement 4) */
    int nonlin_idx[100];
    for (int i = 0; i < 100; i++) {
        nonlin_idx[i] = (i * i + 3 * i + 7) % 100;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        arr_int[i] = i;
        arr_float[i] = i * 0.5f;
        arr_double[i] = i * 0.25;
    }
    
    volatile int final_result = 0;
    
    /* Primary loop with multiple dependency types */
    for (int i = 1; i < N; i++) {
        /* 1. Loop-carried true dependency (RAW) - Requirement 1 */
        arr_int[i] = arr_int[i-1] + i;  /* True dependency across iterations */
        
        /* 2. Anti-dependency (WAR) within same iteration - Requirement 2 */
        int tmp = arr_int[i];           /* Read */
        arr_int[i] = i * 2;             /* Write to same location */
        results[i] = tmp + arr_int[i];  /* Use original value */
        
        /* 3. Output dependency (WAW) - Requirement 2 */
        float x = arr_float[i];
        x = x * 2.0f;                   /* First write to x */
        x = x + 1.0f;                   /* Second write to x - WAW */
        arr_float[i] = x;
        
        /* 4. Conditional dependency patterns - Requirement 3 */
        if (i % 3 == 0) {
            /* Pattern A: Chain of dependencies */
            float a = arr_float[i];
            float b = a * 2.0f;
            float c = b + arr_float[i-1];  /* Cross-iteration dependency */
            arr_float[i] = c;
        } else if (i % 3 == 1) {
            /* Pattern B: Different dependency structure */
            int val1 = arr_int[i];
            int val2 = arr_int[i-1];
            arr_int[i] = (val1 > val2) ? val1 : val2;
        } else {
            /* Pattern C: Mixed type dependencies */
            double d = arr_double[i];
            float f = (float)d;
            int n = (int)f;
            arr_int[i] = n + arr_int[i-1];  /* Another true dependency */
        }
        
        /* 5. Function call with side effects - Requirement 5 */
        update_global(arr_int, i % M);
        
        /* 6. Mixed data type operations - Requirement 6 */
        double dbl_val = arr_double[i];
        float flt_val = arr_float[i] * 2.0f;
        int int_val = (int)(dbl_val + flt_val);
        
        /* Complex expression with multiple dependencies */
        arr_double[i] = (dbl_val * 0.75) + (flt_val * 1.25) + int_val;
    }
    
    /* Nested loop with non-linear array access - Requirement 4 */
    for (int i = 0; i < N - 5; i++) {
        for (int j = 0; j < 5; j++) {
            /* Access using non-linear index */
            int idx = nonlin_idx[i + j];
            
            /* Create dependencies with non-affine pattern */
            if (idx > 0) {
                arr_int[idx] = arr_int[idx-1] + arr_int[nonlin_idx[i]];
            }
            
            /* Mixed type operation chain */
            float base = arr_float[idx];
            double transformed = (double)base * 1.5;
            int rounded = (int)transformed;
            arr_float[idx] = (float)(transformed - rounded);
        }
    }
    
    /* Additional loop with pointer aliasing possibilities */
    int* ptr1 = arr_int;
    int* ptr2 = arr_int + 50;
    for (int i = 0; i < 25; i++) {
        /* Potential aliasing creates conservative dependencies */
        ptr1[i] = ptr2[i] * 2;
        ptr2[i] = ptr1[i] + i;
        
        /* Function call that might alias */
        update_global(ptr1, i % 10);
    }
    
    /* Aggregate results to prevent dead code elimination */
    for (int i = 0; i < 100; i++) {
        final_result += arr_int[i];
        final_result += (int)arr_float[i];
        final_result += (int)arr_double[i];
        final_result += results[i];
    }
    
    final_result += global_counter;
    
    return final_result % 256;
}
