#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_seed = 0;
volatile int g_result_store[8] = {0};

/* Function to generate deterministic but non-constant data */
static inline int gen_data(int i, int seed) {
    return (i * 3 + seed) ^ 0x5A5A;
}

static inline float gen_float_data(int i, int seed) {
    return (float)((i * 7 + seed * 3) % 100) * 0.1f;
}

int main(int argc, char *argv[]) {
    int seed = 1;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize arrays with deterministic but non-constant data */
    int arr_i[64];
    unsigned short arr_us[64];
    float arr_f[64];
    double arr_d[64];
    
    for (int i = 0; i < 64; i++) {
        arr_i[i] = gen_data(i, seed);
        arr_us[i] = (unsigned short)(gen_data(i, seed) & 0xFFFF);
        arr_f[i] = gen_float_data(i, seed);
        arr_d[i] = (double)gen_float_data(i, seed) * 2.0;
    }
    
    /* Loop-invariant thresholds from volatile source */
    volatile int vi_thresh = g_volatile_seed + 50;
    volatile float vf_thresh = (float)(g_volatile_seed + 25) * 0.5f;
    volatile double vd_thresh = (double)(g_volatile_seed + 30) * 0.3;
    
    int thresh_i = vi_thresh;
    float thresh_f = vf_thresh;
    double thresh_d = vd_thresh;
    unsigned short thresh_us = (unsigned short)(vi_thresh & 0xFFFF);
    
    /* Reduction variables */
    int max_val_i = arr_i[0];
    int min_val_i = arr_i[0];
    unsigned short max_val_us = arr_us[0];
    float cond_sum_f = 0.0f;
    double cond_sum_d = 0.0;
    int count_gt = 0;
    int count_le = 0;
    int mixed_max = arr_i[0];
    int mixed_min = arr_i[0];
    
    /* ===== TEST 1: GT_EXPR (greater-than) conditional reduction ===== */
    /* Finding maximum with condition: if (arr_i[i] > current_max) */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (i % 2 == 0) {
            /* GT_EXPR pattern */
            if (arr_i[i] > max_val_i) {
                max_val_i = arr_i[i];
            }
        }
    }
    
    /* ===== TEST 2: GE_EXPR (greater-than-or-equal) conditional reduction ===== */
    /* Sum values >= threshold */
    for (int i = 0; i < 64; i++) {
        /* GE_EXPR pattern with ternary operator */
        cond_sum_f += (arr_f[i] >= thresh_f) ? arr_f[i] : 0.0f;
        
        /* Combined with logical OR in condition */
        if (arr_f[i] >= thresh_f || i % 3 == 0) {
            count_gt++;
        }
    }
    
    /* ===== TEST 3: LT_EXPR (less-than) conditional reduction ===== */
    /* Finding minimum with condition: if (arr_i[i] < current_min) */
    int i = 0;
    while (i < 64) {
        /* LT_EXPR pattern inside nested conditional */
        if (arr_i[i] < 10 || arr_i[i] > 100) {
            if (arr_i[i] < min_val_i) {
                min_val_i = arr_i[i];
            }
        }
        i++;
    }
    
    /* ===== TEST 4: LE_EXPR (less-than-or-equal) conditional reduction ===== */
    /* Sum values <= threshold with multiple reductions in same loop */
    double local_max_d = arr_d[0];
    double local_min_d = arr_d[0];
    
    for (int i = 0; i < 64; i++) {
        /* LE_EXPR pattern for sum */
        if (arr_d[i] <= thresh_d) {
            cond_sum_d += arr_d[i];
            count_le++;
        }
        
        /* Multiple reductions in same loop */
        if (arr_d[i] > local_max_d) {
            local_max_d = arr_d[i];
        }
        if (arr_d[i] < local_min_d) {
            local_min_d = arr_d[i];
        }
    }
    
    /* ===== TEST 5: Mixed comparisons in single loop ===== */
    /* Test all four comparison operators in one loop with unsigned short */
    unsigned short mixed_max_us = arr_us[0];
    unsigned short mixed_min_us = arr_us[0];
    int sum_gt_us = 0, sum_lt_us = 0;
    
    for (int i = 0; i < 64; i++) {
        /* GT_EXPR */
        if (arr_us[i] > mixed_max_us) {
            mixed_max_us = arr_us[i];
        }
        
        /* GE_EXPR with logical AND */
        if (arr_us[i] >= thresh_us && i % 4 == 0) {
            sum_gt_us += arr_us[i];
        }
        
        /* LT_EXPR */
        if (arr_us[i] < mixed_min_us) {
            mixed_min_us = arr_us[i];
        }
        
        /* LE_EXPR with logical OR */
        if (arr_us[i] <= thresh_us || i % 5 == 0) {
            sum_lt_us += arr_us[i];
        }
    }
    
    /* ===== TEST 6: Complex nested conditionals with float ===== */
    float complex_max_f = arr_f[0];
    float complex_sum_f = 0.0f;
    
    for (int i = 0; i < 64; i++) {
        /* Outer if with multiple conditions */
        if (i > 10 && i < 50) {
            /* Inner GT_EXPR */
            if (arr_f[i] > complex_max_f) {
                complex_max_f = arr_f[i];
            }
            
            /* GE_EXPR with float threshold */
            if (arr_f[i] >= (thresh_f * 0.8f)) {
                complex_sum_f += arr_f[i];
            }
        }
    }
    
    /* Prevent dead code elimination */
    g_result_store[0] = max_val_i;
    g_result_store[1] = min_val_i;
    g_result_store[2] = max_val_us;
    g_result_store[3] = (int)cond_sum_f;
    g_result_store[4] = (int)cond_sum_d;
    g_result_store[5] = count_gt;
    g_result_store[6] = count_le;
    g_result_store[7] = mixed_max_us;
    
    /* Compute checksum */
    int checksum = max_val_i + min_val_i + max_val_us + (int)cond_sum_f + 
                   (int)cond_sum_d + count_gt + count_le + mixed_max_us +
                   (int)complex_max_f + (int)complex_sum_f + sum_gt_us + sum_lt_us;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
