/* ddg_test.c - Test program for DDG edge creation coverage */
#include <stdlib.h>

/* Global variables for memory dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Function with side effects (Requirement 5) */
static void update_global(int *arr, int idx) {
    global_counter++;
    global_accumulator += arr[idx] * 0.5f;
    arr[idx] = global_counter;
}

/* Another function with different data type */
static double compute_value(int a, float b) {
    return (double)a * (double)b + 1.234;
}

int main(void) {
    /* Declare arrays with different data types (Requirement 6) */
    const int N = 1024;
    volatile int limit = N;  /* Prevent optimization */
    int int_arr[N];
    float float_arr[N];
    double double_arr[N];
    int index_map[N];  /* For non-affine accesses */
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i;
        float_arr[i] = i * 0.5f;
        index_map[i] = (i * i) % N;  /* Non-linear mapping */
    }
    
    /* Primary loop with loop-carried dependency (Requirement 1) */
    int loop_carried = 0;
    for (int i = 0; i < limit; i++) {
        /* Loop-carried true dependency (RAW) */
        int_arr[i] = loop_carried + i;
        loop_carried = int_arr[i];  /* Feeds next iteration */
        
        /* Anti-dependency (WAR) on local variable (Requirement 2) */
        float temp = float_arr[i];      /* Read */
        float_arr[i] = temp * 2.0f;     /* Write after read */
        
        /* Output dependency (WAW) */
        double_arr[i] = compute_value(i, temp);
        double_arr[i] = double_arr[i] + 1.0;  /* Second write */
        
        /* Conditional creating different dependency patterns (Requirement 3) */
        if (i % 3 == 0) {
            /* Pattern A: Chain of dependencies */
            int x = int_arr[i];
            x = x * 2 + 1;
            float_arr[i] = x * 0.25f;
        } else if (i % 3 == 1) {
            /* Pattern B: Different dependency chain */
            float y = float_arr[i];
            int_arr[i] = (int)y + global_counter;
        } else {
            /* Pattern C: Cross-type dependencies */
            double z = double_arr[i];
            int_arr[i] = (int)z;
            float_arr[i] = (float)(z * 0.5);
        }
        
        /* Function call with side effects (Requirement 5) */
        update_global(int_arr, i % 128);
        
        /* Mixed data type operations (Requirement 6) */
        volatile int vol_var = global_counter;  /* Prevent optimization */
        float mixed_result = int_arr[i] * 0.3f + float_arr[i];
        double_arr[i] = (double)mixed_result * 1.1;
    }
    
    /* Nested loop with non-affine array accesses (Requirement 4) */
    for (int i = 0; i < N/2; i++) {
        for (int j = 0; j < 4; j++) {
            /* Non-linear index calculation */
            int idx = index_map[i] + j * j + i;
            if (idx >= N) idx = idx % N;
            
            /* Complex dependency chain */
            float_arr[idx] = float_arr[idx] + int_arr[i] * 0.1f;
            
            /* Another anti-dependency pattern */
            int old_val = int_arr[idx];
            int_arr[idx] = global_counter + j;
            double_arr[idx] = old_val * 2.0;
        }
    }
    
    /* Additional loop with output dependencies */
    int output_var = 0;
    for (int i = 0; i < 100; i++) {
        output_var = i * 2;          /* First write */
        output_var = output_var + 1; /* Second write (WAW) */
        int_arr[i % 64] = output_var;
    }
    
    /* Final aggregation to prevent dead code elimination */
    volatile int final_result = 0;
    for (int i = 0; i < N; i++) {
        final_result += int_arr[i];
        final_result += (int)float_arr[i];
        final_result += (int)double_arr[i];
    }
    
    return final_result % 256;
}
