#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent dead code elimination */
volatile int global_sink;

/* Function to initialize arrays with deterministic but non-constant values */
void init_array(int *arr, int size, int seed) {
    for (int i = 0; i < size; i++) {
        arr[i] = (i * 3 + seed) % 100;
    }
}

void init_float_array(float *arr, int size, int seed) {
    for (int i = 0; i < size; i++) {
        arr[i] = (float)((i * 7 + seed) % 100) * 1.5f;
    }
}

void init_double_array(double *arr, int size, int seed) {
    for (int i = 0; i < size; i++) {
        arr[i] = (double)((i * 11 + seed) % 100) * 2.5;
    }
}

void init_ushort_array(unsigned short *arr, int size, int seed) {
    for (int i = 0; i < size; i++) {
        arr[i] = (unsigned short)((i * 5 + seed) % 100);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    const int N = 64;
    
    /* Declare and initialize arrays with different types */
    int arr_int[N];
    float arr_float[N];
    double arr_double[N];
    unsigned short arr_ushort[N];
    
    init_array(arr_int, N, seed);
    init_float_array(arr_float, N, seed);
    init_double_array(arr_double, N, seed);
    init_ushort_array(arr_ushort, N, seed);
    
    /* Loop-invariant thresholds from volatile sources */
    volatile int volatile_threshold = seed + 25;
    volatile float volatile_fthreshold = (float)(seed + 30) * 0.8f;
    volatile double volatile_dthreshold = (double)(seed + 20) * 1.2;
    
    int int_threshold = volatile_threshold;
    float float_threshold = volatile_fthreshold;
    double double_threshold = volatile_dthreshold;
    
    /* Reduction variables */
    int max_val_int = arr_int[0];
    int min_val_int = arr_int[0];
    float max_val_float = arr_float[0];
    float min_val_float = arr_float[0];
    double sum_cond_double = 0.0;
    unsigned short max_val_ushort = arr_ushort[0];
    unsigned short min_val_ushort = arr_ushort[0];
    int count_above_threshold = 0;
    int count_below_threshold = 0;
    float sum_above_threshold = 0.0f;
    
    /* ===== Loop 1: GT_EXPR (greater-than) with multiple reductions ===== */
    for (int i = 0; i < N; i++) {
        /* Outer if to complicate control flow */
        if (i % 3 != 0) {
            /* GT_EXPR: Find max int value */
            if (arr_int[i] > max_val_int) {
                max_val_int = arr_int[i];
            }
            
            /* GT_EXPR: Find max float value with logical AND */
            if (i < N-1 && arr_float[i] > max_val_float) {
                max_val_float = arr_float[i];
            }
        }
    }
    
    /* ===== Loop 2: GE_EXPR (greater-than-or-equal) ===== */
    int sum_ge = 0;
    for (int i = 0; i < N; i++) {
        /* GE_EXPR: Sum values >= threshold */
        if (arr_int[i] >= int_threshold) {
            sum_ge += arr_int[i];
        }
        
        /* Nested conditional with GE_EXPR */
        if (i % 4 == 0) {
            if (arr_float[i] >= float_threshold) {
                sum_above_threshold += arr_float[i];
            }
        }
    }
    
    /* ===== Loop 3: LT_EXPR (less-than) with while loop ===== */
    int i = 0;
    while (i < N) {
        /* LT_EXPR: Find min int value */
        if (arr_int[i] < min_val_int) {
            min_val_int = arr_int[i];
        }
        
        /* LT_EXPR: Find min float value with logical OR */
        if (i == 0 || arr_float[i] < min_val_float) {
            min_val_float = arr_float[i];
        }
        
        i++;
    }
    
    /* ===== Loop 4: LE_EXPR (less-than-or-equal) with multiple data types ===== */
    for (int i = 0; i < N; i++) {
        /* LE_EXPR: Count values <= threshold */
        if (arr_int[i] <= int_threshold) {
            count_below_threshold++;
        }
        
        /* LE_EXPR: Conditional sum with double */
        if (arr_double[i] <= double_threshold) {
            sum_cond_double += arr_double[i];
        }
        
        /* LE_EXPR: Find min unsigned short with outer if */
        if (i % 2 == 1) {
            if (arr_ushort[i] <= min_val_ushort) {
                min_val_ushort = arr_ushort[i];
            }
        }
    }
    
    /* ===== Loop 5: Mixed comparisons in single loop ===== */
    for (int i = 0; i < N; i++) {
        /* Multiple reductions with different comparisons */
        if (arr_int[i] > max_val_int) {  /* GT_EXPR */
            max_val_int = arr_int[i];
        }
        
        if (arr_int[i] < min_val_int) {  /* LT_EXPR */
            min_val_int = arr_int[i];
        }
        
        if (arr_ushort[i] >= max_val_ushort) {  /* GE_EXPR */
            max_val_ushort = arr_ushort[i];
        }
        
        if (arr_ushort[i] <= min_val_ushort) {  /* LE_EXPR */
            min_val_ushort = arr_ushort[i];
        }
        
        /* Complex conditional with logical operators */
        if ((i % 3 == 0 || i % 5 == 0) && arr_float[i] >= float_threshold) {
            count_above_threshold++;
        }
    }
    
    /* Prevent dead code elimination */
    global_sink = max_val_int;
    
    /* Compute checksum from all reduction results */
    int checksum = 0;
    checksum += max_val_int;
    checksum += min_val_int;
    checksum += (int)max_val_float;
    checksum += (int)min_val_float;
    checksum += sum_ge;
    checksum += count_above_threshold;
    checksum += count_below_threshold;
    checksum += (int)sum_above_threshold;
    checksum += (int)sum_cond_double;
    checksum += max_val_ushort;
    checksum += min_val_ushort;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
