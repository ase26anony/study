/* Complex dependency pattern generator for DDG edge coverage */
#include <stdlib.h>

/* Global variables for function call dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Function with side effects (Requirement 5) */
static void update_globals(int* arr, float* farr, int idx) {
    global_counter += arr[idx];
    global_accumulator += farr[idx] * 0.5f;
    arr[idx] = global_counter % 100;
}

/* Another static function for different dependency pattern */
static inline float transform_value(float x, int scale) {
    volatile float temp = x * scale;  /* Prevent optimization */
    return temp + global_accumulator;
}

int main() {
    /* Array declarations with different types (Requirement 6) */
    const int N = 1024;
    int arr_int[N];
    float arr_float[N];
    double arr_double[N];
    volatile int limit = N;  /* Volatile to prevent optimization */
    
    /* Non-linear index array (Requirement 4) */
    int nonlin_idx[N];
    for (int i = 0; i < N; i++) {
        nonlin_idx[i] = (i * i + i * 3 + 7) % N;  /* Quadratic non-affine */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr_int[i] = i;
        arr_float[i] = i * 0.5f;
        arr_double[i] = i * 0.25;
    }
    
    /* Primary loop with complex dependencies */
    for (int i = 1; i < limit; i++) {
        /* 1. Loop-carried true dependency (RAW) - Requirement 1 */
        arr_int[i] = arr_int[i-1] + global_counter;
        
        /* Mixed data type operations - Requirement 6 */
        float temp_float = arr_float[i] + arr_int[i];
        double temp_double = arr_double[i] * 2.0;
        
        /* 2. Anti-dependency (WAR) pattern - Requirement 2 */
        int read_first = arr_int[i];          /* Read */
        arr_int[i] = (int)temp_float;         /* Write to same location */
        arr_float[i] = read_first * 0.3f;     /* Use read value */
        
        /* 3. Conditional dependency switching - Requirement 3 */
        if (i % 3 == 0) {
            /* Pattern A: Chain of dependencies */
            float chain = arr_float[i];
            chain = transform_value(chain, 2);  /* Function call */
            arr_float[i] = chain * 0.8f;
            arr_double[i] = chain;
        } else if (i % 3 == 1) {
            /* Pattern B: Different dependency structure */
            int tmp = arr_int[i];
            arr_int[i] = (int)arr_double[i];  /* Output dependency source */
            arr_int[i] = tmp + 1;             /* Output dependency dest (WAW) */
            arr_float[i] = tmp * 0.5f;
        } else {
            /* Pattern C: Memory dependencies through globals */
            update_globals(arr_int, arr_float, i);
        }
        
        /* 5. Function call with side effects - Requirement 5 */
        if (i % 7 == 0) {
            update_globals(arr_int, arr_float, i % N);
        }
        
        /* Output dependency (WAW) on local variable - Requirement 2 */
        double local_var = arr_double[i];
        local_var = local_var * local_var;  /* First write */
        local_var = sqrt(local_var);        /* Second write to same variable */
        arr_double[i] = local_var;
    }
    
    /* Nested loop with non-linear array access - Requirement 4 */
    for (int i = 0; i < limit/2; i++) {
        for (int j = 0; j < 4; j++) {
            int idx = nonlin_idx[(i * 4 + j) % N];
            /* Complex index calculation creates conservative dependencies */
            arr_float[idx] = arr_float[nonlin_idx[(idx + j) % N]] + 1.0f;
            
            /* Additional true dependency chain */
            if (j > 0) {
                arr_int[idx] = arr_int[nonlin_idx[(i * 4 + j - 1) % N]] + j;
            }
        }
    }
    
    /* Mixed type operations in another loop - Requirement 6 */
    for (int i = 0; i < limit; i += 2) {
        /* Integer and float operations interleaved */
        int int_op = arr_int[i] * 3;
        float float_op = arr_float[i] / 2.0f;
        double double_op = arr_double[i] * 1.5;
        
        /* Cross-type dependencies */
        arr_float[i+1] = int_op + float_op;
        arr_int[i] = (int)(float_op * double_op);
        arr_double[i] = int_op + arr_float[i+1];
    }
    
    /* Prevent dead code elimination */
    volatile int result = 0;
    for (int i = 0; i < N; i++) {
        result += arr_int[i] + (int)arr_float[i] + (int)arr_double[i];
    }
    
    return result % 256;
}
