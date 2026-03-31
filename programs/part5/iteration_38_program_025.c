/* ddg_coverage.c - Program to exercise DDG edge creation in GCC scheduler */

#include <stdlib.h>
#include <stdio.h>

/* Global variables for memory dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Static function with side effects (Requirement 5) */
static void update_globals(int* arr, float* farr, int idx) {
    global_counter++;
    global_accumulator += farr[idx % 16];
    arr[idx % 32] = global_counter;
}

/* Another static function for output dependencies */
static float compute_value(float a, float b, int scale) {
    static float last_result = 0.0f;
    float result = a * b + scale;
    
    /* Create output dependency through static variable */
    last_result = result;  /* First write */
    last_result = result * 0.5f;  /* Second write - WAW */
    
    return last_result;
}

/* Non-affine index array (Requirement 4) */
static const int non_linear_indices[16] = {
    0, 1, 4, 9, 16, 25, 36, 49,
    3, 6, 10, 15, 21, 28, 36, 45
};

int main(void) {
    /* Declare arrays with different data types (Requirement 6) */
    int int_array[256];
    float float_array[256];
    double double_buffer[128];
    volatile int N = 128;  /* Volatile loop limit */
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        int_array[i] = i % 37;
        float_array[i] = (float)(i % 29) * 0.1f;
    }
    
    /* Primary loop with loop-carried dependency (Requirement 1) */
    int prev_value = int_array[0];
    float prev_float = float_array[0];
    
    for (int i = 1; i < N; i++) {
        /* Loop-carried true dependency (RAW) - integer */
        int current = prev_value + int_array[i] + i;
        int_array[i] = current;
        prev_value = current;  /* Feeds next iteration */
        
        /* Loop-carried true dependency (RAW) - floating point */
        float fcurrent = prev_float * 1.1f + float_array[i];
        float_array[i] = fcurrent;
        prev_float = fcurrent;  /* Feeds next iteration */
        
        /* Conditional creating different dependency patterns (Requirement 3) */
        if (i % 3 == 0) {
            /* Pattern A: Anti-dependency (WAR) sequence */
            int tmp = int_array[i];           /* Read */
            int_array[i] = i * 2;             /* Write after read */
            float tmp2 = float_array[i];      /* Read */
            float_array[i] = tmp * 0.5f;      /* Write after read */
            double_buffer[i % 128] = tmp2;    /* Use read value */
        } else if (i % 3 == 1) {
            /* Pattern B: Output dependency (WAW) sequence */
            double x = compute_value(float_array[i], prev_float, i);
            x = x * 2.0;                      /* Second write to x - WAW */
            double_buffer[i % 128] = x;
            
            /* Mixed data type operations (Requirement 6) */
            int mixed = (int)x + int_array[i];
            float mixed_f = (float)mixed * 0.25f;
            float_array[i] = mixed_f;
        } else {
            /* Pattern C: Complex chain with all dependency types */
            volatile int v = int_array[i];    /* Volatile read */
            
            /* Output dependency on local variable */
            int local = v * 2;
            local = local + 1;                /* WAW on local */
            
            /* Anti-dependency through array */
            float fread = float_array[i];     /* Read */
            float_array[i] = (float)local;    /* Write after read */
            
            /* True dependency chain */
            double dval = (double)fread + (double)local;
            double_buffer[i % 128] = dval;
        }
        
        /* Function call creating memory dependencies (Requirement 5) */
        update_globals(int_array, float_array, i);
    }
    
    /* Nested loop with non-affine array accesses (Requirement 4) */
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 16; j++) {
            /* Non-linear index calculation */
            int idx = non_linear_indices[j] + i * 16;
            if (idx < 256) {
                /* Access with non-affine index */
                sum += float_array[idx % 256];
                
                /* Additional non-affine write */
                int write_idx = (i * i + j) % 256;
                float_array[write_idx] = sum * 0.01f;
            }
        }
    }
    
    /* Secondary loop demonstrating register pressure and dependencies */
    int acc = 0;
    float facc = 0.0f;
    for (int i = 0; i < 64; i++) {
        /* Multiple writes to same variable creating output dependencies */
        int mult = int_array[i] * 2;
        mult = mult + global_counter;         /* WAW */
        mult = mult % 17;                     /* WAW */
        
        /* Anti-dependency chain */
        float old = float_array[i];           /* Read */
        float_array[i] = (float)mult * 0.33f; /* Write after read */
        facc += old;                          /* Use read value */
        
        /* True dependency with mixed types */
        acc += mult;
        double dtemp = (double)acc + (double)facc;
        double_buffer[i % 64] = dtemp;
    }
    
    /* Final aggregation to prevent dead code elimination */
    volatile int final_result = 0;
    for (int i = 0; i < 256; i++) {
        final_result += int_array[i];
        final_result += (int)float_array[i];
    }
    final_result += (int)sum + acc + (int)facc + global_counter;
    
    /* Use results to prevent optimization */
    printf("Result: %d (global_counter: %d)\n", final_result, global_counter);
    
    return final_result > 0 ? 0 : 1;
}
