/* Complex dependency pattern generator for DDG edge coverage */
#include <stdlib.h>

/* Global variables for memory dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Function with side effects (Requirement 5) */
static void update_globals(int *arr, float *farr, int idx) {
    global_counter += arr[idx];
    global_accumulator += farr[idx];
    arr[idx] = global_counter % 100;
}

/* Another function with memory access */
static inline int compute_index(int i, int j) {
    static int lookup[8] = {0, 1, 3, 2, 7, 4, 5, 6};
    return lookup[(i + j) & 7];
}

int main(void) {
    /* Declare arrays with different data types (Requirement 6) */
    int arr_int[256];
    float arr_float[256];
    double arr_double[256];
    volatile int limit = 128;  /* Prevent optimization */
    
    /* Non-linear index array (Requirement 4) */
    int nonlin_idx[256];
    for (int i = 0; i < 256; i++) {
        nonlin_idx[i] = (i * i + i * 3 + 7) % 256;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        arr_int[i] = i;
        arr_float[i] = i * 0.5f;
        arr_double[i] = i * 0.25;
    }
    
    volatile int result = 0;
    
    /* Primary loop with loop-carried dependency (Requirement 1) */
    for (int i = 1; i < limit; i++) {
        /* True dependency (RAW) - loop carried */
        arr_int[i] = arr_int[i-1] + arr_int[i];  /* Line A */
        
        /* Conditional creating different dependency patterns (Requirement 3) */
        if (i % 3 == 0) {
            /* Pattern 1: Anti-dependency (WAR) sequence */
            int tmp = arr_int[i];                /* Read arr_int[i] */
            arr_int[i] = arr_float[i] * 2;       /* Write arr_int[i] - anti-dep on tmp */
            arr_float[i] = tmp * 0.5f;           /* Use tmp */
            
            /* Output dependency (WAW) */
            float f = arr_float[i] * 2.0f;
            f = arr_float[i] * 3.0f;             /* WAW on f */
            arr_float[i] = f;
        } else if (i % 3 == 1) {
            /* Pattern 2: Chain of dependencies */
            float f1 = arr_float[i] + arr_int[i];
            float f2 = f1 * 1.5f;                /* RAW on f1 */
            int i1 = f2 + arr_int[i-1];          /* RAW on f2, loop-carried on arr_int */
            arr_int[i] = i1 + global_counter;    /* RAW on i1 */
        } else {
            /* Pattern 3: Mixed operations */
            double d = arr_double[i] + arr_int[i];
            arr_double[i] = d * 1.1;             /* WAW on arr_double[i] */
            int tmp2 = d * 2.0;
            
            /* Anti-dependency with array */
            float old_float = arr_float[i];      /* Read */
            arr_float[i] = tmp2 * 0.25f;         /* Write - anti-dep */
            arr_int[i] = old_float + tmp2;       /* Use old value */
        }
        
        /* Function call creating memory dependencies (Requirement 5) */
        update_globals(arr_int, arr_float, i % 128);
        
        /* Complex expression with mixed types */
        arr_double[i] = arr_int[i] * 0.5 + arr_float[i] * 0.3 + arr_double[i-1];
    }
    
    /* Nested loop with non-linear access (Requirement 4) */
    for (int i = 0; i < limit/2; i++) {
        for (int j = 0; j < 8; j++) {
            int idx = compute_index(i, j);
            /* Non-affine access pattern */
            int base = nonlin_idx[idx];
            arr_int[base] += arr_float[idx] + j;
            
            /* More dependencies */
            float ftmp = arr_float[nonlin_idx[(i + j) % 128]];
            arr_float[idx] = ftmp * (i + 1);
        }
    }
    
    /* Additional loop with output dependencies */
    for (int i = 0; i < 64; i++) {
        int x = arr_int[i] * 2;
        x = x + global_counter;      /* WAW on x */
        x = x % 1000;                /* Another WAW on x */
        arr_int[i] = x;
        
        /* Volatile to prevent optimization */
        volatile int v = x;
        result += v;
    }
    
    /* Use all arrays to prevent elimination */
    for (int i = 0; i < 128; i++) {
        result += arr_int[i] + arr_float[i] + arr_double[i];
    }
    
    return result % 255;
}
