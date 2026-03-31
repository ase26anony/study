/* ddg_test.c - Test program to trigger DDG edge creation in GCC scheduler */
#include <stdlib.h>

/* Global variables for function call dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Static function with side effects (Requirement 5) */
static void update_global(int *arr, int idx) {
    global_counter++;
    global_accumulator += arr[idx] * 0.5f;
    arr[idx] = global_counter % 100;
}

/* Another static function for output dependency demonstration */
static float process_value(float x, int i) {
    static float last_value = 0.0f;
    float result = x + last_value;
    last_value = result * 0.8f;
    return result;
}

int main(void) {
    const int N = 1024;
    volatile int limit = N;  /* Prevent optimization */
    
    /* Arrays with different data types (Requirement 6) */
    int int_arr[N];
    float float_arr[N];
    double double_arr[N];
    
    /* Non-linear index array (Requirement 4) */
    int indices[N];
    for (int i = 0; i < N; i++) {
        indices[i] = (i * i + i * 3 + 7) % N;  /* Non-affine index pattern */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i;
        float_arr[i] = i * 0.5f;
        double_arr[i] = i * 0.25;
    }
    
    volatile int result = 0;
    float temp_float = 0.0f;
    int temp_int = 0;
    
    /* Primary loop with various dependencies */
    for (int i = 1; i < limit; i++) {
        /* 1. Loop-carried true dependency (RAW) - Requirement 1 */
        int_arr[i] = int_arr[i-1] + 1;  /* True dependency chain */
        
        /* 2. Anti-dependency (WAR) - Requirement 2 */
        temp_int = float_arr[i];        /* Read float_arr[i] */
        float_arr[i] = int_arr[i] * 0.7f; /* Overwrite float_arr[i] */
        double_arr[i] = temp_int;       /* Use the read value */
        
        /* 3. Output dependency (WAW) - Requirement 2 */
        temp_float = process_value(float_arr[i], i);  /* First write to temp_float */
        temp_float = global_accumulator + i;          /* Second write to temp_float */
        
        /* 4. Conditional dependencies - Requirement 3 */
        if (i % 3 == 0) {
            /* One dependency pattern */
            int_arr[i] = float_arr[i] * 2;
            global_accumulator += int_arr[i];
        } else if (i % 3 == 1) {
            /* Different pattern creating alternative dependencies */
            float_arr[i] = int_arr[i] * 0.3f;
            int_arr[i] = global_counter;
        } else {
            /* Third pattern with reversed flow */
            global_counter += int_arr[i];
            float_arr[i] = global_counter * 0.1f;
        }
        
        /* 5. Function call with side effects - Requirement 5 */
        update_global(int_arr, i % N);
        
        /* Mixed data type operations - Requirement 6 */
        double intermediate = int_arr[i] * 0.25 + float_arr[i];
        float_arr[i] = intermediate * 2.0f;
        
        /* Volatile to prevent optimization */
        result += int_arr[i];
    }
    
    /* Nested loop with non-linear array access - Requirement 4 */
    for (int i = 0; i < limit/2; i++) {
        for (int j = 0; j < 4; j++) {
            /* Access using non-affine indices */
            int idx = indices[(i * 4 + j) % N];
            float_arr[idx] = float_arr[idx] * 1.1f + j;
            
            /* Create cross-iteration dependency */
            if (j > 0) {
                double_arr[idx] = double_arr[indices[(i * 4 + j - 1) % N]] * 0.9;
            }
        }
    }
    
    /* Additional loop with complex dependencies */
    for (int i = 0; i < limit; i += 2) {
        /* Multiple writes to same location (output dependency) */
        int_arr[i] = i * 2;
        int_arr[i] = int_arr[i] + global_counter;
        int_arr[i] = int_arr[i] % 1000;
        
        /* Chain of dependencies with different types */
        float f1 = int_arr[i];
        float f2 = f1 * 0.5f;
        double d1 = f2;
        float_arr[i] = d1;
        
        /* Anti-dependency with array element */
        int temp = int_arr[i+1];
        int_arr[i+1] = float_arr[i] * 100;
        double_arr[i] = temp;
    }
    
    /* Final aggregation to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        result += int_arr[i] + float_arr[i] + double_arr[i];
    }
    
    return result;
}
