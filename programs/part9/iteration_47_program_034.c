#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile globals to prevent dead code elimination */
volatile int g_result_int;
volatile float g_result_float;
volatile double g_result_double;

/* Function to generate deterministic data */
static inline int gen_data(int i, int seed) {
    return (i * 3 + seed) % 1000;
}

static inline float gen_float_data(int i, int seed) {
    return (float)((i * 7 + seed * 3) % 1000) * 0.5f;
}

static inline double gen_double_data(int i, int seed) {
    return (double)((i * 11 + seed * 5) % 1000) * 0.25;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    
    /* Declare arrays with different types */
    int arr_int[64];
    unsigned short arr_ushort[64];
    float arr_float[64];
    double arr_double[64];
    
    /* Initialize arrays with deterministic data */
    for (int i = 0; i < 64; i++) {
        arr_int[i] = gen_data(i, seed);
        arr_ushort[i] = (unsigned short)gen_data(i, seed + 1);
        arr_float[i] = gen_float_data(i, seed);
        arr_double[i] = gen_double_data(i, seed);
    }
    
    /* Loop-invariant thresholds from volatile sources */
    volatile int v_threshold_int = seed + 100;
    volatile unsigned short v_threshold_ushort = (seed + 50) % 65535;
    volatile float v_threshold_float = seed * 0.7f;
    volatile double v_threshold_double = seed * 0.3;
    
    int threshold_int = v_threshold_int;
    unsigned short threshold_ushort = v_threshold_ushort;
    float threshold_float = v_threshold_float;
    double threshold_double = v_threshold_double;
    
    /* Reduction variables */
    int max_int = arr_int[0];  /* For GT_EXPR pattern */
    int min_int = arr_int[0];  /* For LT_EXPR pattern */
    float sum_float_ge = 0.0f; /* For GE_EXPR pattern */
    double sum_double_le = 0.0; /* For LE_EXPR pattern */
    unsigned short max_ushort_gt = arr_ushort[0];
    int count_int_gt = 0;
    float min_float_lt = arr_float[0];
    double max_double_ge = arr_double[0];
    
    /* Pattern 1: GT_EXPR (greater than) with multiple reductions in one loop */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (arr_int[i] > 0) {
            /* GT_EXPR: if (arr_int[i] > max_int) max_int = arr_int[i]; */
            if (arr_int[i] > max_int) {
                max_int = arr_int[i];
            }
            
            /* Another GT_EXPR with different variable */
            if (arr_int[i] > threshold_int) {
                count_int_gt++;
            }
        }
    }
    
    /* Pattern 2: GE_EXPR (greater than or equal) with float */
    for (int i = 0; i < 64; i++) {
        /* Combined condition with logical AND */
        if (i % 2 == 0 && arr_float[i] >= threshold_float) {
            sum_float_ge += arr_float[i];
        }
        
        /* Nested conditional for GE_EXPR on double */
        if (i < 50) {
            if (arr_double[i] >= max_double_ge) {
                max_double_ge = arr_double[i];
            }
        }
    }
    
    /* Pattern 3: LT_EXPR (less than) with while loop */
    int j = 0;
    while (j < 64) {
        /* LT_EXPR: if (arr_float[j] < min_float_lt) min_float_lt = arr_float[j]; */
        if (arr_float[j] < min_float_lt) {
            min_float_lt = arr_float[j];
        }
        
        /* Multiple reductions with LT_EXPR on different types */
        if (arr_int[j] < threshold_int) {
            min_int = (arr_int[j] < min_int) ? arr_int[j] : min_int;
        }
        j++;
    }
    
    /* Pattern 4: LE_EXPR (less than or equal) with unsigned short */
    for (int i = 0; i < 64; i++) {
        /* LE_EXPR with logical OR in condition */
        if (i % 3 == 0 || arr_ushort[i] <= threshold_ushort) {
            if (arr_ushort[i] <= max_ushort_gt) {
                /* This should trigger std::swap in the uncovered code */
                max_ushort_gt = arr_ushort[i];
            }
        }
        
        /* LE_EXPR on double for sum reduction */
        if (arr_double[i] <= threshold_double) {
            sum_double_le += arr_double[i];
        }
    }
    
    /* Pattern 5: Mixed comparisons in single loop (tests multiple reductions) */
    int mixed_max = arr_int[0];
    int mixed_min = arr_int[0];
    float mixed_sum_ge = 0.0f;
    int mixed_count_le = 0;
    
    for (int i = 0; i < 64; i++) {
        /* GT_EXPR for max */
        if (arr_int[i] > mixed_max) {
            mixed_max = arr_int[i];
        }
        
        /* LT_EXPR for min */
        if (arr_int[i] < mixed_min) {
            mixed_min = arr_int[i];
        }
        
        /* GE_EXPR for conditional sum */
        if (arr_float[i] >= threshold_float) {
            mixed_sum_ge += arr_float[i];
        }
        
        /* LE_EXPR for count */
        if (arr_int[i] <= threshold_int) {
            mixed_count_le++;
        }
    }
    
    /* Store results to volatile to prevent optimization */
    g_result_int = max_int;
    g_result_float = sum_float_ge;
    g_result_double = sum_double_le;
    
    /* Compute checksum from all reduction results */
    int checksum = 0;
    checksum += max_int;
    checksum += min_int;
    checksum += (int)sum_float_ge;
    checksum += (int)sum_double_le;
    checksum += max_ushort_gt;
    checksum += count_int_gt;
    checksum += (int)min_float_lt;
    checksum += (int)max_double_ge;
    checksum += mixed_max;
    checksum += mixed_min;
    checksum += (int)mixed_sum_ge;
    checksum += mixed_count_le;
    
    /* Add array elements to checksum for verification */
    for (int i = 0; i < 64; i++) {
        checksum += arr_int[i] % 256;
        checksum += arr_ushort[i] % 256;
        checksum += (int)arr_float[i];
        checksum += (int)arr_double[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
