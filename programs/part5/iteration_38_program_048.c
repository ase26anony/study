/* ddg_test.c - Complex dependency patterns to exercise DDG edge creation */
#include <stdlib.h>

/* Global variables for function call dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Static helper function with side effects (Requirement 5) */
static void update_global(int *arr, int idx) {
    global_counter++;
    global_accumulator += arr[idx] * 0.5f;
    arr[idx] = global_counter % 100;
}

/* Another static function for output dependencies */
static inline float transform_value(float x, int scale) {
    static float last_result = 0.0f;  /* Creates WAW dependencies across calls */
    float result = x * scale + last_result;
    last_result = result;
    return result;
}

int main(void) {
    /* Declare arrays with different data types (Requirement 6) */
    const int N = 1024;
    volatile int limit = N;  /* Volatile to prevent optimization */
    int int_array[N];
    float float_array[N];
    double double_array[N];
    int index_map[N];  /* For non-affine accesses */
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_array[i] = i;
        float_array[i] = i * 0.5f;
        double_array[i] = i * 0.25;
        /* Create non-linear index mapping (Requirement 4) */
        index_map[i] = (i * i + 3 * i + 7) % N;
    }
    
    /* Primary loop with complex dependencies */
    volatile int result = 0;
    
    for (int i = 1; i < limit; i++) {
        /* 1. LOOP-CARRIED TRUE DEPENDENCY (RAW) - Requirement 1 */
        int_array[i] = int_array[i-1] + i;  /* True dependency chain */
        
        /* 2. ANTI-DEPENDENCY (WAR) on local variable - Requirement 2 */
        int temp = int_array[i];           /* Read */
        int_array[i] = float_array[i] * 2; /* Write to same location */
        float_array[i] = temp * 0.3f;      /* Use the read value */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Requirement 2 */
        float fvalue = transform_value(float_array[i], i);
        fvalue = transform_value(fvalue, i+1);  /* Second write to same variable */
        
        /* 4. CONDITIONAL DEPENDENCY PATTERNS - Requirement 3 */
        if (i % 3 == 0) {
            /* Pattern A: More complex RAW chain */
            double_array[i] = double_array[i-1] * 1.1 + float_array[i];
            int_array[i] = (int)(double_array[i]) % 1000;
        } else if (i % 3 == 1) {
            /* Pattern B: WAR with array elements */
            float old_val = float_array[i];
            float_array[i] = int_array[i] * 0.7f;
            double_array[i] = old_val + double_array[i-2];
        } else {
            /* Pattern C: WAW on multiple variables */
            int tmp = int_array[i] * 2;
            tmp = int_array[i-1] + tmp;  /* Overwrite tmp */
            int_array[i] = tmp;
        }
        
        /* 5. FUNCTION CALL WITH SIDE EFFECTS - Requirement 5 */
        update_global(int_array, i);
        
        /* 6. MIXED DATA TYPE OPERATIONS - Requirement 6 */
        volatile float mixed_calc = int_array[i] * 0.25f + global_accumulator;
        double_array[i] += (double)mixed_calc * 1.5;
        
        /* Anti-dependency through pointer aliasing */
        int *alias_ptr = &int_array[i];
        int read_alias = *alias_ptr;
        *alias_ptr = read_alias ^ 0xFF;  /* WAR through pointer */
    }
    
    /* Secondary loop with non-affine array accesses - Requirement 4 */
    for (int i = 0; i < limit/2; i++) {
        /* Access using non-linear index */
        int idx = index_map[i];
        
        /* Create dependencies through non-affine access */
        float_array[idx] = float_array[index_map[i+1]] * 1.2f;
        
        /* Loop-carried dependency with non-unit stride */
        if (idx > 0) {
            double_array[idx] = double_array[index_map[idx-1]] + 0.1;
        }
        
        /* Function call creating memory dependencies */
        update_global(int_array, idx);
    }
    
    /* Nested loops for additional DDG complexity */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int idx = i * 10 + j;
            if (idx < N) {
                /* Cross-iteration dependency in nested loop */
                float_array[idx] = (j == 0) ? 
                    float_array[(i-1)*10 + 9] * 0.8f : 
                    float_array[idx-1] + 0.1f;
                    
                /* Output dependency in inner loop */
                double temp_dbl = double_array[idx];
                temp_dbl = temp_dbl * temp_dbl;  /* WAW */
                double_array[idx] = temp_dbl;
            }
        }
    }
    
    /* Aggregate results to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        result += int_array[i];
        result += (int)float_array[i];
        result += (int)double_array[i];
    }
    
    result += global_counter;
    
    return result % 255;
}
