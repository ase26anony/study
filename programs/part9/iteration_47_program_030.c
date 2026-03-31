#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile globals to prevent dead code elimination */
volatile int global_sink;
volatile float global_float_sink;

/* Function to generate deterministic data */
static inline int gen_val(int i, int seed) {
    return (i * 3 + seed) & 0xFF;  /* Keep values in range */
}

static inline float gen_float(int i, int seed) {
    return (float)((i * 7 + seed * 3) & 0xFF) * 0.5f;
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
        arr_us[i] = (unsigned short)gen_val(i, seed ^ 0x55);
        arr_f[i] = gen_float(i, seed);
        arr_d[i] = (double)gen_float(i, seed ^ 0xAA);
    }
    
    /* Loop-invariant thresholds from volatile sources */
    volatile int vi_thresh = seed + 100;
    volatile float vf_thresh = (float)(seed + 50) * 0.7f;
    volatile double vd_thresh = (double)(seed + 75) * 0.3;
    
    int thresh_i = vi_thresh;
    float thresh_f = vf_thresh;
    double thresh_d = vd_thresh;
    unsigned short thresh_us = (unsigned short)(seed + 60);
    
    /* Reduction variables */
    int max_val_i = arr_i[0];
    int min_val_i = arr_i[0];
    unsigned short max_val_us = arr_us[0];
    float cond_sum_f = 0.0f;
    double cond_sum_d = 0.0;
    int count_gt = 0;
    int count_le = 0;
    
    /* Loop 1: GT_EXPR (>), multiple reductions with nested if */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (arr_i[i] != 0) {
            /* Conditional max reduction with > */
            if (arr_i[i] > max_val_i) {
                max_val_i = arr_i[i];
            }
            
            /* Combined condition with logical AND */
            if (arr_i[i] > thresh_i && arr_i[i] < 255) {
                count_gt++;
            }
        }
    }
    
    /* Loop 2: GE_EXPR (>=), float conditional sum */
    float local_max_f = arr_f[0];
    for (int i = 0; i < 64; i++) {
        /* Conditional sum with >= */
        if (arr_f[i] >= thresh_f) {
            cond_sum_f += arr_f[i];
        }
        
        /* Also track max with >= in same loop */
        if (arr_f[i] >= local_max_f) {
            local_max_f = arr_f[i];
        }
    }
    
    /* Loop 3: LT_EXPR (<), while loop variant */
    int j = 0;
    int limit = 64 - (seed % 8);
    while (j < limit) {
        /* Conditional min reduction with < */
        if (arr_i[j] < min_val_i) {
            min_val_i = arr_i[j];
        }
        
        /* Another reduction with different condition */
        if (arr_us[j] < thresh_us) {
            cond_sum_d += (double)arr_us[j];
        }
        j++;
    }
    
    /* Loop 4: LE_EXPR (<=), multiple data types */
    double min_val_d = arr_d[0];
    for (int i = 0; i < 64; i++) {
        /* Conditional count with <= */
        if (arr_d[i] <= thresh_d) {
            count_le++;
        }
        
        /* Min reduction with <= on different array */
        if (arr_d[i] <= min_val_d) {
            min_val_d = arr_d[i];
        }
        
        /* Additional reduction on integer array */
        if (arr_i[i] <= thresh_i) {
            max_val_us = (arr_us[i] > max_val_us) ? arr_us[i] : max_val_us;
        }
    }
    
    /* Loop 5: Mixed comparisons in single loop */
    int sum_mixed = 0;
    short min_mixed = 32767;
    for (int i = 0; i < 64; i++) {
        /* GT and LT in same loop */
        if (arr_i[i] > thresh_i) {
            sum_mixed += arr_i[i];
        }
        if (arr_i[i] < thresh_i + 10) {
            if (arr_i[i] < min_mixed) {
                min_mixed = (short)arr_i[i];
            }
        }
    }
    
    /* Prevent optimization */
    global_sink = max_val_i + min_val_i + count_gt + count_le + sum_mixed + min_mixed;
    global_float_sink = cond_sum_f + (float)cond_sum_d + (float)local_max_f + (float)min_val_d;
    
    /* Compute checksum */
    uint32_t checksum = 0;
    checksum += (uint32_t)max_val_i;
    checksum += (uint32_t)min_val_i;
    checksum += (uint32_t)max_val_us;
    checksum += (uint32_t)count_gt;
    checksum += (uint32_t)count_le;
    checksum += (uint32_t)sum_mixed;
    checksum += (uint32_t)min_mixed;
    checksum += (uint32_t)cond_sum_f;
    checksum += (uint32_t)cond_sum_d;
    checksum += (uint32_t)local_max_f;
    checksum += (uint32_t)min_val_d;
    
    printf("Checksum: %u\n", checksum);
    
    return 0;
}
