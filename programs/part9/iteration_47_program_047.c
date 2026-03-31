#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent dead code elimination */
volatile int global_sink;

/* Function to initialize arrays with deterministic but non-constant values */
static void init_array(int *arr, int size, int seed) {
    for (int i = 0; i < size; i++) {
        arr[i] = (i * 3 + seed) % 100;
    }
}

static void init_float_array(float *arr, int size, int seed) {
    for (int i = 0; i < size; i++) {
        arr[i] = (float)((i * 7 + seed) % 100) / 2.0f;
    }
}

static void init_double_array(double *arr, int size, int seed) {
    for (int i = 0; i < size; i++) {
        arr[i] = (double)((i * 11 + seed) % 100) / 3.0;
    }
}

static void init_ushort_array(unsigned short *arr, int size, int seed) {
    for (int i = 0; i < size; i++) {
        arr[i] = (unsigned short)((i * 5 + seed) % 65535);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    const int N = 64;
    
    /* Declare arrays with different types */
    int arr_int[N];
    float arr_float[N];
    double arr_double[N];
    unsigned short arr_ushort[N];
    
    /* Initialize arrays */
    init_array(arr_int, N, seed);
    init_float_array(arr_float, N, seed);
    init_double_array(arr_double, N, seed);
    init_ushort_array(arr_ushort, N, seed);
    
    /* Loop-invariant thresholds from volatile sources */
    volatile int v_threshold_int = 30;
    volatile float v_threshold_float = 15.5f;
    volatile double v_threshold_double = 25.0;
    volatile unsigned short v_threshold_ushort = 20000;
    
    int threshold_int = v_threshold_int;
    float threshold_float = v_threshold_float;
    double threshold_double = v_threshold_double;
    unsigned short threshold_ushort = v_threshold_ushort;
    
    /* Reduction variables */
    int max_val_int = arr_int[0];
    int min_val_int = arr_int[0];
    float max_val_float = arr_float[0];
    float min_val_float = arr_float[0];
    double sum_cond_double = 0.0;
    unsigned int count_cond_ushort = 0;
    int cond_sum_int = 0;
    float cond_sum_float = 0.0f;
    
    /* Example 1: GT_EXPR pattern - find maximum with > comparison */
    printf("Processing GT_EXPR patterns...\n");
    for (int i = 0; i < N; i++) {
        /* Outer if to complicate control flow */
        if (i % 2 == 0) {
            /* GT_EXPR: if (arr_int[i] > max_val_int) */
            if (arr_int[i] > max_val_int) {
                max_val_int = arr_int[i];
            }
            
            /* Combined with another condition using logical AND */
            if (arr_float[i] > threshold_float && i < N - 1) {
                cond_sum_float += arr_float[i];
            }
        }
    }
    
    /* Example 2: GE_EXPR pattern - conditional sum with >= comparison */
    printf("Processing GE_EXPR patterns...\n");
    int i = 0;
    while (i < N) {
        /* GE_EXPR: sum values >= threshold */
        if (arr_int[i] >= threshold_int) {
            cond_sum_int += arr_int[i];
        }
        
        /* Multiple reductions in one loop */
        if (arr_float[i] >= threshold_float) {
            if (arr_float[i] > max_val_float) {
                max_val_float = arr_float[i];
            }
        }
        
        i++;
    }
    
    /* Example 3: LT_EXPR pattern - find minimum with < comparison */
    printf("Processing LT_EXPR patterns...\n");
    for (int i = 0; i < N; i++) {
        /* LT_EXPR: if (arr_double[i] < min_val_double) */
        double current = arr_double[i];
        if (current < threshold_double) {
            sum_cond_double += current;
            
            /* Nested conditional */
            if (i > 0 && current < arr_double[i-1]) {
                /* Additional operation to make body non-trivial */
                global_sink = i;
            }
        }
        
        /* Multiple conditions with logical OR */
        if (arr_int[i] < threshold_int || arr_int[i] % 3 == 0) {
            if (arr_int[i] < min_val_int) {
                min_val_int = arr_int[i];
            }
        }
    }
    
    /* Example 4: LE_EXPR pattern - count with <= comparison */
    printf("Processing LE_EXPR patterns...\n");
    for (int i = 0; i < N; i++) {
        /* LE_EXPR: count values <= threshold */
        if (arr_ushort[i] <= threshold_ushort) {
            count_cond_ushort++;
        }
        
        /* Multiple reductions with different data types */
        if (arr_float[i] <= threshold_float) {
            if (arr_float[i] < min_val_float) {
                min_val_float = arr_float[i];
            }
        }
    }
    
    /* Example 5: Mixed patterns in single loop */
    printf("Processing mixed patterns...\n");
    int mixed_max = arr_int[0];
    int mixed_min = arr_int[0];
    int mixed_sum = 0;
    int mixed_count = 0;
    
    for (int i = 0; i < N; i++) {
        /* GT_EXPR for max */
        if (arr_int[i] > mixed_max) {
            mixed_max = arr_int[i];
        }
        
        /* LT_EXPR for min */
        if (arr_int[i] < mixed_min) {
            mixed_min = arr_int[i];
        }
        
        /* GE_EXPR for conditional sum */
        if (arr_int[i] >= threshold_int) {
            mixed_sum += arr_int[i];
        }
        
        /* LE_EXPR for count */
        if (arr_int[i] <= threshold_int + 10) {
            mixed_count++;
        }
    }
    
    /* Compute checksum from all results */
    int checksum = 0;
    checksum += max_val_int;
    checksum += min_val_int;
    checksum += (int)max_val_float;
    checksum += (int)min_val_float;
    checksum += (int)sum_cond_double;
    checksum += count_cond_ushort;
    checksum += cond_sum_int;
    checksum += (int)cond_sum_float;
    checksum += mixed_max;
    checksum += mixed_min;
    checksum += mixed_sum;
    checksum += mixed_count;
    
    /* Prevent optimization */
    global_sink = checksum;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
