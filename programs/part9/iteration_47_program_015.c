#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile globals to prevent dead code elimination */
volatile int global_sink_int;
volatile float global_sink_float;

/* Function to generate deterministic data */
static inline int gen_val(int i, int seed) {
    return (i * 3 + seed) & 0xFF;  /* Keep values in range */
}

static inline float gen_float(int i, int seed) {
    return (float)((i * 5 + seed) & 0xFF) * 0.7f;
}

int main(int argc, char **argv) {
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Arrays with different types */
    int arr_i[64];
    unsigned short arr_us[64];
    float arr_f[64];
    double arr_d[64];
    
    /* Initialize arrays deterministically */
    for (int i = 0; i < 64; i++) {
        arr_i[i] = gen_val(i, seed);
        arr_us[i] = (unsigned short)gen_val(i, seed + 1);
        arr_f[i] = gen_float(i, seed + 2);
        arr_d[i] = (double)gen_float(i, seed + 3);
    }
    
    /* Loop-invariant thresholds from volatile sources */
    volatile int vi_thresh = seed + 10;
    volatile float vf_thresh = seed * 0.5f + 5.0f;
    volatile double vd_thresh = seed * 0.3 + 3.0;
    
    int thresh_i = vi_thresh;
    float thresh_f = vf_thresh;
    double thresh_d = vd_thresh;
    
    /* Reduction variables */
    int max_val_i = arr_i[0];
    int min_val_i = arr_i[0];
    unsigned short max_val_us = arr_us[0];
    unsigned short min_val_us = arr_us[0];
    float max_val_f = arr_f[0];
    float min_val_f = arr_f[0];
    double max_val_d = arr_d[0];
    double min_val_d = arr_d[0];
    int cond_sum_i = 0;
    float cond_sum_f = 0.0f;
    double cond_sum_d = 0.0;
    int count_gt = 0, count_lt = 0;
    
    /* ===== Loop 1: GT_EXPR pattern (>) ===== */
    /* Mixed reductions with > comparison */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (i % 3 != 0) {
            /* GT_EXPR: Find max with > */
            if (arr_i[i] > max_val_i) {
                max_val_i = arr_i[i];
            }
            
            /* Another GT_EXPR with different type */
            if (arr_f[i] > max_val_f) {
                max_val_f = arr_f[i];
            }
            
            /* Conditional sum with > */
            if (arr_i[i] > thresh_i) {
                cond_sum_i += arr_i[i];
            }
        }
    }
    
    /* ===== Loop 2: GE_EXPR pattern (>=) ===== */
    /* While loop variant with >= comparisons */
    int j = 0;
    while (j < 64) {
        /* GE_EXPR: Find min with >= (inverted logic) */
        if (!(arr_us[j] >= min_val_us)) {
            min_val_us = arr_us[j];
        }
        
        /* Conditional sum with >= and logical AND */
        if (j > 0 && arr_f[j] >= thresh_f) {
            cond_sum_f += arr_f[j];
        }
        
        /* Multiple reductions with same comparison */
        if (arr_d[j] >= max_val_d) {
            max_val_d = arr_d[j];
        }
        
        j++;
    }
    
    /* ===== Loop 3: LT_EXPR pattern (<) ===== */
    /* Loop with multiple reductions using < */
    for (int i = 0; i < 64; i++) {
        /* LT_EXPR: Find min with < */
        if (arr_i[i] < min_val_i) {
            min_val_i = arr_i[i];
        }
        
        /* Another LT_EXPR with float */
        if (arr_f[i] < min_val_f) {
            min_val_f = arr_f[i];
        }
        
        /* Count values less than threshold */
        if (arr_i[i] < thresh_i) {
            count_lt++;
        }
        
        /* Conditional sum with logical OR */
        if (i < 32 || arr_d[i] < thresh_d) {
            cond_sum_d += arr_d[i];
        }
    }
    
    /* ===== Loop 4: LE_EXPR pattern (<=) ===== */
    /* Loop with <= comparisons and complex conditions */
    for (int i = 0; i < 64; i++) {
        /* Outer if with compound condition */
        if (i % 2 == 0 && i % 3 != 0) {
            /* LE_EXPR: Find max with <= (inverted logic) */
            if (!(arr_us[i] <= max_val_us)) {
                max_val_us = arr_us[i];
            }
            
            /* Conditional count with <= */
            if (arr_i[i] <= thresh_i) {
                count_gt++;  /* Actually counting <=, but reuse variable */
            }
        }
        
        /* Another LE_EXPR with float and logical AND */
        if (i > 10 && i < 50 && arr_f[i] <= max_val_f) {
            /* This maintains max_val_f from earlier */
        }
    }
    
    /* ===== Loop 5: Mixed comparisons in single loop ===== */
    /* Single loop with all four comparison types */
    int mixed_max = arr_i[0];
    int mixed_min = arr_i[0];
    float mixed_sum_gt = 0.0f;
    float mixed_sum_lt = 0.0f;
    
    for (int i = 0; i < 64; i++) {
        /* GT_EXPR */
        if (arr_i[i] > mixed_max) {
            mixed_max = arr_i[i];
        }
        
        /* GE_EXPR with logical OR */
        if (i % 4 == 0 || arr_i[i] >= thresh_i) {
            /* Do nothing, just test the comparison */
        }
        
        /* LT_EXPR */
        if (arr_i[i] < mixed_min) {
            mixed_min = arr_i[i];
        }
        
        /* LE_EXPR with logical AND */
        if (i > 0 && arr_i[i] <= thresh_i + 5) {
            /* Do nothing, just test the comparison */
        }
        
        /* Conditional sums with different comparisons */
        if (arr_f[i] > thresh_f) {
            mixed_sum_gt += arr_f[i];
        }
        if (arr_f[i] < thresh_f) {
            mixed_sum_lt += arr_f[i];
        }
    }
    
    /* Prevent optimization */
    global_sink_int = max_val_i;
    global_sink_float = max_val_f;
    
    /* Compute checksum */
    int checksum = 0;
    checksum += max_val_i;
    checksum += min_val_i;
    checksum += (int)max_val_us;
    checksum += (int)min_val_us;
    checksum += (int)max_val_f;
    checksum += (int)min_val_f;
    checksum += (int)max_val_d;
    checksum += (int)min_val_d;
    checksum += cond_sum_i;
    checksum += (int)cond_sum_f;
    checksum += (int)cond_sum_d;
    checksum += count_gt;
    checksum += count_lt;
    checksum += mixed_max;
    checksum += mixed_min;
    checksum += (int)mixed_sum_gt;
    checksum += (int)mixed_sum_lt;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
