/* ddg_test.c - Test program for DDG edge creation coverage */
#include <stdlib.h>

/* Global variables for memory dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Static function with side effects (Requirement 5) */
static void update_global(int* arr, int idx) {
    global_counter++;
    arr[idx] += global_counter;
    global_accumulator += arr[idx] * 0.5f;
}

/* Another static function for output dependencies */
static inline float transform_value(float x, int scale) {
    static float last_result = 0.0f;  /* Static local creates WAW dependencies */
    float result = x * scale + last_result;
    last_result = result;  /* Output dependency through static variable */
    return result;
}

int main() {
    /* Declare arrays with different data types (Requirement 6) */
    const int N = 100;
    volatile int limit = N;  /* Volatile to prevent optimization */
    int int_arr[N];
    float float_arr[N];
    double double_arr[N];
    
    /* Non-linear index array (Requirement 4) */
    int indices[N];
    for (int i = 0; i < N; i++) {
        indices[i] = (i * i + i * 3 + 7) % N;  /* Non-affine access pattern */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i;
        float_arr[i] = i * 0.5f;
        double_arr[i] = i * 0.25;
    }
    
    /* Primary loop with complex dependencies */
    for (int i = 1; i < limit; i++) {
        /* 1. Loop-carried true dependency (RAW) - Requirement 1 */
        int_arr[i] = int_arr[i-1] + 2;  /* True dependency chain */
        
        /* 2. Anti-dependency (WAR) - Requirement 2 */
        int temp = int_arr[i];          /* Read */
        int_arr[i] = float_arr[i] > 0 ? temp * 2 : temp / 2;  /* Later write */
        
        /* 3. Conditional dependency patterns - Requirement 3 */
        if (i % 3 == 0) {
            /* Pattern A: Chain of dependencies */
            float_arr[i] = float_arr[i-1] * 1.1f;
            double_arr[i] = double_arr[i-1] + float_arr[i];
        } else if (i % 3 == 1) {
            /* Pattern B: Different dependency structure */
            float_arr[i] = double_arr[i-1] * 2.0f;
            double_arr[i] = int_arr[i] * 0.5;
        } else {
            /* Pattern C: Output dependencies (WAW) - Requirement 2 */
            float tmp_val = transform_value(float_arr[i], i);
            tmp_val = tmp_val * 0.8f;  /* Output dependency on tmp_val */
            float_arr[i] = tmp_val;
        }
        
        /* 4. Function call with side effects - Requirement 5 */
        update_global(int_arr, i % 10);
        
        /* 5. Mixed data type operations - Requirement 6 */
        float mixed_result = int_arr[i] * 0.3f + float_arr[i];
        double_arr[i] = mixed_result * 1.5 + global_accumulator;
    }
    
    /* Nested loop with non-linear array access - Requirement 4 */
    volatile int sum = 0;
    for (int i = 0; i < N/2; i++) {
        for (int j = 0; j < 4; j++) {
            /* Access using non-linear indices */
            int idx = indices[(i * 4 + j) % N];
            sum += int_arr[idx] + j;
            
            /* Create anti-dependency in inner loop */
            float old_val = float_arr[idx];
            float_arr[idx] = old_val * (i + j) * 0.1f;
        }
    }
    
    /* Additional dependency patterns in separate loop */
    int output_dep_var = 0;
    for (int i = 0; i < N; i += 2) {
        /* Output dependency sequence */
        output_dep_var = int_arr[i] * 3;
        output_dep_var = output_dep_var - float_arr[i];  /* WAW on output_dep_var */
        
        /* Memory dependency through pointer */
        int* ptr = &int_arr[i];
        int read_val = *ptr;      /* Read */
        *ptr = read_val + i;      /* Write - creates memory dependency */
    }
    
    /* Complex loop with multiple dependency types */
    for (int i = 2; i < N; i++) {
        /* Interleaved dependencies */
        int a = int_arr[i-1] + int_arr[i-2];  /* Uses two previous values */
        float b = float_arr[i-1] * a;
        
        /* Conditional with dependencies */
        if (a > b) {
            int_arr[i] = a - int_arr[i-1];
        } else {
            int_arr[i] = b + int_arr[i-2];
        }
        
        /* Function call creating memory dependencies */
        update_global(int_arr, i % 5);
        
        /* Mixed-type chain */
        double_arr[i] = (double)int_arr[i] * float_arr[i] / (i + 1);
    }
    
    /* Final aggregation to prevent dead code elimination */
    volatile double final_result = 0.0;
    for (int i = 0; i < N; i++) {
        final_result += int_arr[i] + float_arr[i] + double_arr[i];
    }
    
    return (int)final_result % 256;
}
