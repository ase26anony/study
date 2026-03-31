/* ddg_test.c - Test program to exercise DDG edge creation in GCC scheduler */

#include <stdlib.h>
#include <math.h>

/* Global variables for memory dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Requirement 5: Function with side effects */
static void update_global(int *arr, int idx) {
    global_counter++;
    global_accumulator += arr[idx] * 0.5f;
    arr[idx] = global_counter % 100;
}

/* Another static function for mixed operations */
static double compute_value(int i, float f) {
    return (i * 1.5) + (f * 2.3);
}

int main(void) {
    /* Requirement 1 & 6: Arrays with different data types */
    const int N = 1024;
    int int_arr[N];
    float float_arr[N];
    double double_arr[N];
    volatile int limit = N;  /* Prevent optimization */
    
    /* Requirement 4: Non-linear index array */
    int indices[N];
    for (int i = 0; i < N; i++) {
        indices[i] = (i * i + i * 3) % N;  /* Non-affine access pattern */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i;
        float_arr[i] = i * 0.5f;
        double_arr[i] = i * 0.25;
    }
    
    volatile int result = 0;
    
    /* Primary loop with various dependencies */
    for (int i = 1; i < limit; i++) {
        /* Requirement 1: Loop-carried true dependency (RAW) */
        int_arr[i] = int_arr[i-1] + 1;  /* True dependency chain */
        
        /* Requirement 2: Anti-dependency (WAR) on local variable */
        int tmp = int_arr[i];           /* Read */
        int_arr[i] = float_arr[i] > 0 ? tmp : 0;  /* Later write - anti-dependency */
        
        /* Requirement 2: Output dependency (WAW) */
        float f1 = sinf(i * 0.01f);     /* First write to f1 */
        f1 = cosf(i * 0.01f);           /* Second write to f1 - output dependency */
        float_arr[i] = f1;
        
        /* Requirement 3: Conditional dependency patterns */
        if (i % 3 == 0) {
            /* Pattern A: Chain of dependencies */
            double d1 = compute_value(i, float_arr[i]);
            double d2 = d1 * 1.1;        /* RAW on d1 */
            double_arr[i] = d2;
        } else if (i % 3 == 1) {
            /* Pattern B: Different dependency pattern */
            double_arr[i] = double_arr[i-1] * 0.9;  /* Another loop-carried dep */
        } else {
            /* Pattern C: Anti-dependency pattern */
            int old_val = int_arr[i];    /* Read */
            int_arr[i] = i * 2;          /* Write - anti-dependency */
            result += old_val;           /* Use the read value */
        }
        
        /* Requirement 5: Function call with side effects */
        update_global(int_arr, i % 100);
        
        /* Requirement 6: Mixed data type operations */
        float mixed = int_arr[i] * 0.3f + float_arr[i];
        double_arr[i] += mixed * 0.5;
        
        /* More output dependencies */
        volatile int output_var = i;     /* volatile to prevent optimization */
        output_var = i * 2;              /* Output dependency */
        output_var = i * 3;              /* Another output dependency */
    }
    
    /* Requirement 4: Nested loop with non-linear array access */
    for (int i = 0; i < N/2; i++) {
        for (int j = 0; j < 4; j++) {
            /* Non-affine access using pre-computed indices */
            int idx = indices[(i * 4 + j) % N];
            float_arr[idx] = float_arr[idx] * 1.1f + j;
            
            /* Create cross-iteration dependency */
            if (j > 0) {
                float_arr[idx] += float_arr[indices[(i * 4 + j - 1) % N]] * 0.5f;
            }
        }
    }
    
    /* Additional dependency patterns in separate loop */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        /* Complex dependency chain */
        int a = int_arr[i];
        int b = a * 2;          /* RAW on a */
        int c = b + i;          /* RAW on b */
        int d = c - a;          /* RAW on c, also WAR on a? */
        a = d / 2;              /* WAR on a (original a not used again) */
        sum += a;
        
        /* Memory anti-dependency */
        float old_float = float_arr[i];
        float_arr[i] = sqrtf(fabsf(old_float));
        double_arr[i] += old_float;
    }
    
    /* Use all results to prevent dead code elimination */
    result += sum;
    result += (int)global_accumulator;
    result += (int)double_arr[N-1];
    
    /* Final volatile store */
    volatile int final_result = result;
    
    return final_result % 255;
}
