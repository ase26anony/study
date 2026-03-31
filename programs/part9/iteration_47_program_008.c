#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile globals to prevent dead code elimination */
volatile int g_result_int = 0;
volatile float g_result_float = 0.0f;

/* Function to generate deterministic data */
static inline int gen_value(int i, int seed) {
    return (i * 3 + seed) % 1000;
}

static inline float gen_float(int i, int seed) {
    return (float)((i * 7 + seed * 13) % 1000) * 0.1f;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    
    /* Arrays with different data types */
    int arr_i[64];
    unsigned short arr_us[64];
    float arr_f[64];
    double arr_d[64];
    
    /* Initialize arrays deterministically */
    for (int i = 0; i < 64; i++) {
        arr_i[i] = gen_value(i, seed);
        arr_us[i] = (unsigned short)(gen_value(i, seed + 1) % 65535);
        arr_f[i] = gen_float(i, seed);
        arr_d[i] = (double)gen_float(i, seed + 2);
    }
    
    /* Loop-invariant thresholds from volatile sources */
    volatile int thresh_int = seed + 500;
    volatile float thresh_float = (float)(seed % 100) * 0.5f + 25.0f;
    volatile unsigned short thresh_us = 30000;
    volatile double thresh_double = (double)(seed % 200) * 0.25 + 50.0;
    
    int th_i = thresh_int;
    float th_f = thresh_float;
    unsigned short th_us = thresh_us;
    double th_d = thresh_double;
    
    /* Reduction variables */
    int max_val_i = arr_i[0];
    int min_val_i = arr_i[0];
    unsigned short max_val_us = arr_us[0];
    float sum_cond_f = 0.0f;
    double sum_cond_d = 0.0;
    int count_gt = 0;
    int count_le = 0;
    int sum_mixed = 0;
    
    /* ===== Loop 1: GT_EXPR (greater-than) conditional reduction ===== */
    /* Find maximum with > comparison */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (i % 3 != 0) {
            /* GT_EXPR pattern: if (arr_i[i] > max_val_i) max_val_i = arr_i[i] */
            if (arr_i[i] > max_val_i) {
                max_val_i = arr_i[i];
            }
            
            /* Additional reduction with different condition in same loop */
            if (arr_us[i] > th_us) {
                count_gt++;
            }
        }
    }
    
    /* ===== Loop 2: GE_EXPR (greater-than-or-equal) conditional reduction ===== */
    /* Sum values >= threshold */
    for (int i = 0; i < 64; i++) {
        /* GE_EXPR pattern with ternary operator */
        sum_cond_f += (arr_f[i] >= th_f) ? arr_f[i] : 0.0f;
        
        /* Nested conditional with logical OR */
        if (i < 32 || arr_f[i] >= th_f * 0.5f) {
            sum_mixed += arr_i[i % 32];
        }
    }
    
    /* ===== Loop 3: LT_EXPR (less-than) conditional reduction ===== */
    /* Find minimum with < comparison */
    int limit = 64;
    int j = 0;
    while (j < limit) {
        /* LT_EXPR pattern: if (arr_i[j] < min_val_i) min_val_i = arr_i[j] */
        if (arr_i[j] < min_val_i) {
            min_val_i = arr_i[j];
        }
        
        /* Multiple reductions with different conditions */
        if (arr_f[j] < th_f && j % 4 == 0) {
            sum_cond_d += arr_d[j];
        }
        j++;
    }
    
    /* ===== Loop 4: LE_EXPR (less-than-or-equal) conditional reduction ===== */
    /* Count values <= threshold and sum them */
    int sum_le = 0;
    for (int i = 0; i < 64; i++) {
        /* LE_EXPR pattern with combined logical AND */
        if (arr_i[i] <= th_i && i % 2 == 0) {
            count_le++;
            sum_le += arr_i[i];
        }
        
        /* Additional GE_EXPR reduction in same loop */
        if (arr_us[i] >= th_us) {
            max_val_us = (arr_us[i] > max_val_us) ? arr_us[i] : max_val_us;
        }
    }
    
    /* ===== Loop 5: Mixed reductions with all comparison types ===== */
    int max_mixed = arr_i[0];
    int min_mixed = arr_i[0];
    int sum_gt_mixed = 0;
    int sum_lt_mixed = 0;
    
    for (int i = 0; i < 64; i++) {
        /* GT_EXPR */
        if (arr_i[i] > max_mixed) {
            max_mixed = arr_i[i];
        }
        
        /* LT_EXPR */
        if (arr_i[i] < min_mixed) {
            min_mixed = arr_i[i];
        }
        
        /* GE_EXPR */
        if (arr_i[i] >= th_i - 100) {
            sum_gt_mixed += arr_i[i];
        }
        
        /* LE_EXPR */
        if (arr_i[i] <= th_i + 100) {
            sum_lt_mixed += arr_i[i];
        }
    }
    
    /* Aggregate results into checksum */
    int checksum = max_val_i;
    checksum += min_val_i;
    checksum += max_val_us;
    checksum += (int)sum_cond_f;
    checksum += (int)sum_cond_d;
    checksum += count_gt;
    checksum += count_le;
    checksum += sum_mixed;
    checksum += sum_le;
    checksum += max_mixed;
    checksum += min_mixed;
    checksum += sum_gt_mixed;
    checksum += sum_lt_mixed;
    
    /* Store to volatile to prevent elimination */
    g_result_int = checksum;
    g_result_float = sum_cond_f;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
