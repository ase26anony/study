/* ddg_coverage.c
 * Program designed to exercise Data Dependency Graph edge creation
 * Compile with: gcc -O2 -fschedule-insns -fdump-rtl-sched1 -fdump-rtl-sched2 ddg_coverage.c -o ddg_coverage
 * Or for modulo scheduling: gcc -O3 -fmodulo-sched -fdump-rtl-sms ddg_coverage.c -o ddg_coverage
 */

#include <stdlib.h>

/* Global variables for memory dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Static function with side effects (Requirement 5) */
static void update_global(int* arr, int idx) {
    global_counter++;
    arr[idx] += global_counter;
    global_accumulator += (float)arr[idx] * 0.5f;
}

/* Another static function for output dependencies */
static inline float compute_value(float a, float b) {
    volatile float result;  /* Prevent optimization */
    result = a * b;
    return result;
}

int main(void) {
    const int N = 1024;
    volatile int limit = N;  /* Volatile to prevent optimization */
    
    /* Arrays with different data types (Requirement 6) */
    int int_arr[N];
    float float_arr[N];
    double double_arr[N];
    
    /* Non-linear index array (Requirement 4) */
    int indices[N];
    for (int i = 0; i < N; i++) {
        indices[i] = (i * i + i * 3) % N;  /* Non-affine index pattern */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i;
        float_arr[i] = (float)i * 0.5f;
        double_arr[i] = (double)i * 0.25;
    }
    
    volatile int result = 0;
    
    /* Primary loop with various dependencies */
    for (int i = 1; i < limit; i++) {
        /* 1. Loop-carried true dependency (RAW) - Requirement 1 */
        int_arr[i] = int_arr[i-1] + 1;  /* True dependency across iterations */
        
        /* 2. Anti-dependency (WAR) and Output dependency (WAW) - Requirement 2 */
        int tmp = int_arr[i];           /* Read arr[i] */
        int_arr[i] = float_arr[i] > 0 ? 1 : 0;  /* Overwrite arr[i] - WAR */
        float tmp2 = float_arr[i];      /* Read float_arr[i] */
        float_arr[i] = tmp2 * 2.0f;     /* Overwrite float_arr[i] - WAR */
        
        /* Output dependency (WAW) on local variable */
        tmp = compute_value(float_arr[i], 1.5f);  /* First write to tmp */
        tmp = tmp + int_arr[i];                   /* Second write to tmp - WAW */
        
        /* 3. Control flow with potential dependencies - Requirement 3 */
        if (i % 3 == 0) {
            /* One dependency pattern */
            float_arr[i] = float_arr[i-1] * 1.1f;
            int_arr[i] = (int)float_arr[i];
        } else if (i % 3 == 1) {
            /* Different pattern with reversed flow */
            int_arr[i] = int_arr[i] * 2;
            float_arr[i] = (float)int_arr[i] / 3.0f;
        } else {
            /* Third pattern with both dependencies */
            float tmp_float = float_arr[i-1];
            int_arr[i] = (int)tmp_float + int_arr[i];
            float_arr[i] = (float)int_arr[i] * 0.5f;
        }
        
        /* 5. Function call with side effects - Requirement 5 */
        update_global(int_arr, i % 100);
        
        /* 6. Mixed data type operations - Requirement 6 */
        double dval = (double)int_arr[i] * 0.33;
        float fval = (float)dval + float_arr[i];
        int ival = (int)fval;
        double_arr[i] = dval + (double)fval + (double)ival;
    }
    
    /* Secondary loop with non-linear array access - Requirement 4 */
    for (int i = 0; i < limit - 10; i++) {
        int idx = indices[i];  /* Non-affine index access */
        float_arr[idx] = float_arr[indices[i+1]] * 0.9f;
        
        /* Create chain of dependencies */
        if (idx > 0) {
            int_arr[idx] = int_arr[indices[i-1]] + int_arr[idx];
        }
    }
    
    /* Nested loop with complex dependencies */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int index = i * 10 + j;
            if (index < N) {
                /* Cross-iteration dependency in inner loop */
                float_arr[index] = (j > 0) ? 
                    float_arr[index] + float_arr[index-1] * 0.5f :
                    float_arr[index] * 2.0f;
                
                /* Output dependency */
                double temp = double_arr[index];
                temp = temp * 1.1;
                temp = temp + 1.0;  /* WAW on temp */
                double_arr[index] = temp;
            }
        }
    }
    
    /* Aggregate results to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        result += int_arr[i];
        result += (int)float_arr[i];
        result += (int)double_arr[i];
    }
    
    result += global_counter;
    result += (int)global_accumulator;
    
    return result;
}
