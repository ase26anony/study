#include <stdlib.h>
#include <time.h>

/* Global variables for memory dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Function with side effects (Requirement 5) */
static void update_global(int* arr, int idx) {
    global_counter++;
    global_accumulator += arr[idx] * 0.5f;
    arr[idx] = global_counter;
}

/* Another function with side effects */
static inline float compute_with_side_effect(float a, float b, int* counter) {
    float result = a * b;
    *counter += 1;
    global_accumulator += result;
    return result;
}

int main() {
    const int N = 100;
    volatile int limit = N;  /* Prevent optimization */
    
    /* Arrays with different data types */
    int arr_int[N];
    float arr_float[N];
    double arr_double[N];
    int results[N];
    
    /* Non-linear index array (Requirement 4) */
    int nonlin_idx[N];
    for (int i = 0; i < N; i++) {
        nonlin_idx[i] = (i * i + i * 3) % N;  /* Non-affine index calculation */
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr_int[i] = i;
        arr_float[i] = i * 0.5f;
        arr_double[i] = i * 0.25;
    }
    
    /* Primary loop with various dependencies */
    int prev_result = 0;
    float prev_float = 0.0f;
    
    for (int i = 0; i < limit; i++) {
        /* 1. Loop-carried true dependency (RAW) (Requirement 1) */
        int current = prev_result + arr_int[i];
        results[i] = current;
        prev_result = current;  /* True dependency between iterations */
        
        /* 2. Anti-dependency (WAR) on local variable */
        float temp = arr_float[i];      /* Read arr_float[i] */
        arr_float[i] = prev_float * 2.0f; /* Overwrite arr_float[i] later */
        prev_float = temp;              /* Anti-dependency through temp */
        
        /* 3. Output dependency (WAW) on x */
        float x = compute_with_side_effect(temp, 1.5f, &global_counter);
        x = arr_float[i] * 3.0f;  /* Second write to x - output dependency */
        
        /* 4. Conditional with different dependency patterns (Requirement 3) */
        if (i % 3 == 0) {
            /* Pattern A: Chain of dependencies */
            int chain = arr_int[i];
            chain = chain * 2 + 1;
            arr_int[i] = chain;
        } else if (i % 3 == 1) {
            /* Pattern B: Different dependency through global */
            update_global(arr_int, i);
        } else {
            /* Pattern C: Mixed type dependencies */
            double d = arr_double[i];
            d = d * 1.1 + i;
            arr_float[i] = (float)d;  /* Cross-type dependency */
        }
        
        /* 5. Function call with side effects (Requirement 5) */
        update_global(results, i % 10);
        
        /* 6. Mixed data type operations (Requirement 6) */
        volatile int volatile_var = i;  /* Prevent optimization */
        int mixed = arr_int[i] + (int)arr_float[i];
        float mixed_float = mixed * 0.7f + arr_float[i];
        arr_double[i] = mixed_float * 1.5;
    }
    
    /* Nested loop with non-linear array access (Requirement 4) */
    double sum = 0.0;
    for (int i = 0; i < limit / 2; i++) {
        for (int j = 0; j < 5; j++) {
            /* Non-affine access pattern */
            int idx = nonlin_idx[(i * 7 + j * 3) % N];
            sum += arr_double[idx] * (i + j);
            
            /* Additional dependency chain */
            float f = arr_float[idx];
            arr_float[idx] = f * (i % 5 + 1);
        }
    }
    
    /* Complex dependency with multiple array accesses */
    int complex_result = 0;
    for (int i = 1; i < limit; i++) {
        /* Multiple interleaved dependencies */
        int a = results[i-1];           /* Loop-carried */
        int b = arr_int[i] + a;         /* True dependency */
        arr_int[i] = b;                 /* Output dependency */
        
        float c = arr_float[i];
        arr_float[i] = c + b;           /* Anti-dependency on c */
        
        complex_result += b;
        
        /* Volatile to prevent optimization */
        volatile int dummy = complex_result;
        (void)dummy;
    }
    
    /* Aggregate results to prevent dead code elimination */
    volatile int final_result = 0;
    final_result += prev_result;
    final_result += (int)prev_float;
    final_result += (int)sum;
    final_result += complex_result;
    final_result += global_counter;
    
    return final_result % 256;
}
