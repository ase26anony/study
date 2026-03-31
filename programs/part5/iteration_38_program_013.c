/* ddg_coverage.c
 * Program designed to exercise Data Dependency Graph edge creation
 * Compile with: gcc -O2 -fschedule-insns -fdump-rtl-sched1 -fdump-rtl-sched2 ddg_coverage.c -o ddg_coverage
 * For modulo scheduling: gcc -O3 -fmodulo-sched -fdump-rtl-sms ddg_coverage.c -o ddg_coverage
 */

#include <stdlib.h>
#include <stdio.h>

/* Global variables for function call dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Static helper function with side effects (Requirement 5) */
static void update_global(int* ptr, float* fptr) {
    *ptr += 1;
    global_accumulator += *fptr;
    global_counter = *ptr;
}

/* Another static function for output dependencies */
static inline float transform_value(float x, int scale) {
    static float last_result = 0.0f;  /* Static variable creates memory dependencies */
    float result = x * scale + last_result;
    last_result = result * 0.5f;
    return result;
}

int main(void) {
    const int N = 1000;
    volatile int limit = N;  /* Volatile to prevent optimization */
    
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
    
    /* Primary loop with loop-carried dependencies (Requirement 1) */
    for (int i = 1; i < limit; i++) {
        /* TRUE DEPENDENCY (RAW): Loop-carried on int_arr */
        int_arr[i] = int_arr[i-1] + i * 2;  /* e->type = TRUE_DEP, e->data_type = DT_INT */
        
        /* Conditional creating different dependency patterns (Requirement 3) */
        if (i % 3 == 0) {
            /* Pattern A: Chain of dependencies */
            float temp = float_arr[i-1];
            float_arr[i] = temp * 1.1f + i;
            
            /* ANTI DEPENDENCY (WAR): Read then write same location */
            double old_val = double_arr[i];
            double_arr[i] = sin(i * 0.01);
            /* Use old_val to create true dependency */
            float_arr[i] += (float)old_val;
        } else if (i % 3 == 1) {
            /* Pattern B: Different dependency structure */
            /* OUTPUT DEPENDENCY (WAW): Multiple writes */
            float tmp_calc = float_arr[i] * 2.0f;
            tmp_calc = tmp_calc / 1.5f;  /* WAW on tmp_calc */
            float_arr[i] = tmp_calc + int_arr[i] * 0.5f;
            
            /* Mixed type operations feeding each other (Requirement 6) */
            int int_val = int_arr[i] % 100;
            float float_val = int_val * 0.7f;
            double_arr[i] = float_val * 1.3;
        } else {
            /* Pattern C: Complex dependency web */
            /* Function call creating memory dependencies (Requirement 5) */
            update_global(&global_counter, &float_arr[i]);
            
            /* Anti-dependency sequence (Requirement 2) */
            int tmp_storage = int_arr[i];      /* Read */
            int_arr[i] = global_counter + i;   /* Write - WAR on int_arr[i] */
            double_arr[i] = tmp_storage * 0.33;
        }
        
        /* Additional output dependency in all paths */
        volatile int output_var = i;  /* Volatile prevents optimization */
        output_var = output_var * 2;  /* WAW on output_var */
        
        /* Call function that creates output dependencies via static variable */
        float transformed = transform_value(float_arr[i], i % 10 + 1);
        float_arr[i] = transformed * 0.9f;
    }
    
    /* Nested loop with non-linear array access (Requirement 4) */
    volatile double nested_result = 0.0;
    for (int i = 0; i < limit/2; i++) {
        int idx = indices[i];  /* Non-affine index */
        for (int j = 0; j < 4; j++) {
            /* Access with non-linear pattern */
            int complex_idx = indices[(i + j * j) % N];
            nested_result += double_arr[complex_idx] * int_arr[idx];
            
            /* Create loop-carried dependency in inner loop */
            if (j > 0) {
                float_arr[complex_idx] = float_arr[complex_idx] * 0.99f + 
                                        float_arr[indices[(i + (j-1)*(j-1)) % N]];
            }
        }
    }
    
    /* Another loop with pointer aliasing possibilities */
    int* ptr1 = int_arr;
    int* ptr2 = &int_arr[N/2];
    volatile int alias_test = 0;
    
    for (int i = 0; i < limit/4; i++) {
        /* Potential pointer aliasing creates conservative dependencies */
        *ptr1 = *ptr2 + i;
        ptr1++;
        ptr2--;
        alias_test += *ptr1;
        
        /* Conditional store creating control dependencies */
        if (alias_test % 7 == 0) {
            ptr1[0] = ptr2[1];
        }
    }
    
    /* Mixed type dependency chain across arrays (Requirement 6) */
    volatile float final_result = 0.0f;
    for (int i = 0; i < limit; i += 2) {
        /* Chain involving int -> float -> double -> int */
        int base = int_arr[i];
        float intermediate = float_arr[base % N] * 2.0f;
        double dbl_val = intermediate * 0.5 + double_arr[i % N];
        int_arr[(i + 1) % N] = (int)dbl_val + base;
        
        final_result += intermediate + (float)dbl_val;
    }
    
    /* Aggregate results to prevent dead code elimination */
    volatile int checksum = 0;
    volatile float fsum = 0.0f;
    volatile double dsum = 0.0;
    
    for (int i = 0; i < N; i++) {
        checksum += int_arr[i];
        fsum += float_arr[i];
        dsum += double_arr[i];
    }
    
    /* Use all results to ensure they're not optimized away */
    return (int)((checksum % 1000) + fsum + dsum + nested_result + final_result + alias_test);
}
