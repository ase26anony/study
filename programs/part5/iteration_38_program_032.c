/* ddg_test.c - Test program to exercise DDG edge creation */
#include <stdlib.h>
#include <stdio.h>

/* Global variables for function call dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Requirement 5: Function with side effects */
static void update_global(int *arr, int idx) {
    global_counter++;
    global_accumulator += arr[idx] * 0.5f;
    arr[idx] = global_counter % 100;
}

/* Requirement 4: Non-linear index array */
static const int nonlin_idx[16] = {
    0, 3, 1, 2, 7, 5, 4, 6,
    15, 12, 13, 14, 8, 9, 10, 11
};

/* Requirement 6: Mixed data type operations */
static float mixed_operation(int a, float b) {
    volatile float temp = b;  /* Prevent optimization */
    return (a * 0.7f) + (temp * 1.3f);
}

int main(void) {
    /* Requirement 1 & 6: Arrays with different data types */
    int int_arr[256];
    float float_arr[256];
    double double_arr[128];
    
    /* Requirement 2: Variables for anti/output dependencies */
    int tmp1, tmp2;
    float ftmp1, ftmp2;
    
    /* Requirement 8: Volatile result to prevent elimination */
    volatile int final_result = 0;
    volatile float final_float = 0.0f;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        int_arr[i] = i % 37;
        float_arr[i] = i * 0.3f;
        if (i < 128) {
            double_arr[i] = i * 0.7;
        }
    }
    
    /* Primary loop with multiple dependency types */
    volatile int N = 100;  /* Volatile to prevent constant propagation */
    
    for (int i = 1; i < N; i++) {
        /* Requirement 1: Loop-carried true dependency (RAW) */
        int_arr[i] = int_arr[i-1] + i;  /* True dependency */
        
        /* Requirement 2: Anti-dependency (WAR) on array */
        tmp1 = int_arr[i];              /* Read */
        int_arr[i] = float_arr[i] > 0 ? tmp1 + 1 : tmp1 - 1;  /* Write later */
        
        /* Requirement 2: Output dependency (WAW) on local variable */
        ftmp1 = mixed_operation(i, float_arr[i]);  /* First write */
        ftmp1 = ftmp1 * 0.9f + global_accumulator; /* Second write to same var */
        
        /* Requirement 3: Conditional dependency patterns */
        if (i % 3 == 0) {
            /* Pattern A: Chain of dependencies */
            tmp2 = int_arr[i] * 2;
            float_arr[i] = tmp2 * 0.5f;
            tmp1 = (int)float_arr[i];  /* Convert back */
        } else if (i % 3 == 1) {
            /* Pattern B: Different dependency pattern */
            float_arr[i] = int_arr[i-1] * 0.3f;  /* Cross-iteration */
            tmp1 = (int)(float_arr[i] * 2.0f);
            int_arr[i] = tmp1 + global_counter;
        } else {
            /* Pattern C: More complex flow */
            ftmp2 = float_arr[i-1] + float_arr[i];
            float_arr[i] = ftmp2 * 0.7f;
            tmp1 = i * i % 256;
        }
        
        /* Requirement 5: Function call with side effects */
        update_global(int_arr, i % 128);
        
        /* Requirement 6: Mixed type operations feeding each other */
        double temp_dbl = double_arr[i % 128] + int_arr[i];
        float_arr[i] = (float)temp_dbl * 0.3f;
        
        /* Anti-dependency with float array */
        ftmp2 = float_arr[i];
        float_arr[i] = (ftmp2 > 100.0f) ? ftmp2 - 50.0f : ftmp2 + 50.0f;
    }
    
    /* Requirement 4: Nested loop with non-linear array access */
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 16; j++) {
            int idx = nonlin_idx[j] + i * 16;
            if (idx < 256) {
                /* Non-affine access pattern */
                int_arr[idx] = int_arr[nonlin_idx[(i + j) % 16]] * 2;
                
                /* Create dependencies with non-linear indices */
                if (j > 0) {
                    int_arr[idx] += int_arr[nonlin_idx[j-1] + i * 16];
                }
            }
        }
    }
    
    /* Additional loop with output dependencies */
    for (int i = 0; i < 50; i++) {
        /* Multiple writes to same variable (WAW) */
        tmp1 = int_arr[i % 64] * 3;
        tmp1 = tmp1 + float_arr[i % 64];  /* Overwrite */
        tmp1 = tmp1 % 1000;               /* Overwrite again */
        
        /* Anti-dependency chain */
        ftmp1 = float_arr[i % 64];
        float_arr[i % 64] = ftmp1 * 1.1f;
        ftmp1 = float_arr[i % 64] + 5.0f;  /* Reuse variable */
    }
    
    /* Final aggregation to prevent dead code elimination */
    for (int i = 0; i < 256; i++) {
        final_result += int_arr[i];
        final_float += float_arr[i];
        if (i < 128) {
            final_float += (float)double_arr[i];
        }
    }
    
    /* Use results to prevent optimization */
    printf("Result: %d, Float: %.2f\n", final_result, final_float);
    
    return final_result > 0 ? 0 : 1;
}
