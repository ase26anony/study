#include <stdlib.h>

/* Global variables for memory dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Function with side effects (Requirement 5) */
static void update_global(int* arr, int idx) {
    global_counter++;
    global_accumulator += arr[idx] * 0.5f;
    arr[idx] = global_counter % 100;
}

/* Another function with memory side effects */
static inline void modify_shared(float* a, float* b, int i) {
    static float last_value = 1.0f;
    *a = *b + last_value;
    last_value = *a * 0.8f;
    *b = last_value + i;
}

int main() {
    const int N = 1024;
    volatile int limit = N;  /* Prevent optimization */
    
    /* Arrays with different data types */
    int arr_int[N];
    float arr_float[N];
    double arr_double[N];
    int results[N];
    
    /* Non-linear index array (Requirement 4) */
    int nonlin_idx[N];
    for (int i = 0; i < N; i++) {
        nonlin_idx[i] = (i * i + i * 3 + 7) % N;  /* Quadratic index */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr_int[i] = i;
        arr_float[i] = i * 0.5f;
        arr_double[i] = i * 0.25;
        results[i] = 0;
    }
    
    /* Primary loop with various dependencies */
    for (int i = 1; i < limit; i++) {
        /* 1. Loop-carried true dependency (RAW) - Requirement 1 */
        arr_int[i] = arr_int[i-1] + i;  /* Integer chain */
        arr_float[i] = arr_float[i-1] * 1.1f;  /* Float chain */
        
        /* 2. Anti-dependency (WAR) - Requirement 2 */
        int tmp = arr_int[i];      /* Read arr_int[i] */
        arr_int[i] = i * 2;        /* Overwrite arr_int[i] */
        results[i] = tmp + i;      /* Use original value */
        
        /* 2. Output dependency (WAW) on local variable */
        float x = arr_float[i] * 2.0f;
        x = x / 1.5f;  /* Second write to x */
        arr_float[i] = x;
        
        /* 3. Conditional dependency patterns - Requirement 3 */
        if (i % 3 == 0) {
            /* Pattern A: More complex dependency chain */
            int a = arr_int[i];
            int b = results[i-1];
            arr_int[i] = b + a;
            results[i] = a * 2;
        } else if (i % 3 == 1) {
            /* Pattern B: Different dependency pattern */
            float f1 = arr_float[i];
            float f2 = arr_float[i-1];
            arr_float[i] = f1 + f2;
            results[i] = (int)(f1 * 100);
        } else {
            /* Pattern C: Cross-type dependencies */
            double d = arr_double[i];
            int r = results[i];
            arr_double[i] = d + r;
            results[i] = (int)(d * 2);
        }
        
        /* 5. Function call with side effects - Requirement 5 */
        update_global(arr_int, i % 100);
        
        /* 6. Mixed data type operations - Requirement 6 */
        volatile int vol_var = i % 16;  /* Prevent optimization */
        float mixed_result = arr_int[i] * 0.3f + arr_float[i];
        double double_result = mixed_result * arr_double[i];
        
        /* More mixed operations */
        int int_from_float = (int)arr_float[i];
        float float_from_int = (float)arr_int[i];
        results[i] += int_from_float + (int)float_from_int;
        
        /* Call inline function with memory side effects */
        modify_shared(&arr_float[i], &arr_float[(i+1)%N], i);
    }
    
    /* Nested loop with non-linear array access - Requirement 4 */
    for (int i = 0; i < limit/2; i++) {
        for (int j = 0; j < 4; j++) {
            int idx = nonlin_idx[(i * 4 + j) % N];
            /* Complex access pattern */
            arr_int[idx] += arr_int[nonlin_idx[(idx + j) % N]];
            arr_float[idx] *= 1.01f + j * 0.1f;
        }
    }
    
    /* Additional loop with output dependencies */
    for (int i = 0; i < limit; i += 2) {
        /* Multiple writes to same memory location */
        double d = i * 0.1;
        d = d * d;          /* WAW on d */
        d = sqrt(d + 1.0);  /* Another WAW on d */
        arr_double[i] = d;
        
        /* Register pressure and reuse */
        int r1 = arr_int[i];
        int r2 = arr_int[i+1];
        r1 = r1 + r2;      /* WAR on r1 */
        r2 = r1 * 2;       /* Use r1, WAR on r2 */
        arr_int[i] = r2;
    }
    
    /* Final aggregation to prevent dead code elimination */
    volatile int final_result = 0;
    for (int i = 0; i < limit; i++) {
        final_result += arr_int[i] + (int)arr_float[i] + (int)arr_double[i];
        final_result += results[i];
    }
    
    final_result += global_counter;
    
    return final_result % 256;
}
