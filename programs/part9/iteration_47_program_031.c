#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_seed;
volatile int g_result_store[10];

/* Function to create loop-invariant thresholds */
int get_threshold(int base) {
    volatile int v = base;
    return v + g_volatile_seed % 5;
}

float get_fthreshold(float base) {
    volatile float v = base;
    return v + (g_volatile_seed % 5) * 0.5f;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    g_volatile_seed = seed;
    
    /* Initialize arrays with deterministic but non-constant values */
    int arr_i[64];
    unsigned short arr_us[64];
    float arr_f[64];
    double arr_d[64];
    
    for (int i = 0; i < 64; i++) {
        arr_i[i] = (i * 3 + seed) % 100;
        arr_us[i] = (unsigned short)((i * 7 + seed) % 65535);
        arr_f[i] = (float)((i * 11 + seed) % 100) * 0.7f;
        arr_d[i] = (double)((i * 13 + seed) % 100) * 0.9;
    }
    
    /* Loop-invariant thresholds */
    int thresh_i = get_threshold(50);
    float thresh_f = get_fthreshold(40.0f);
    double thresh_d = get_fthreshold(35.0);
    
    /* Reduction variables */
    int max_val_i = arr_i[0];
    int min_val_i = arr_i[0];
    unsigned short max_val_us = arr_us[0];
    unsigned short min_val_us = arr_us[0];
    float cond_sum_f = 0.0f;
    double cond_sum_d = 0.0;
    int count_gt = 0;
    int count_le = 0;
    
    /* ====== Loop 1: GT_EXPR and LT_EXPR with multiple reductions ====== */
    /* This should trigger GT_EXPR -> BIT_NOT_EXPR, BIT_AND_EXPR
       and LT_EXPR -> BIT_NOT_EXPR, BIT_AND_EXPR with swap */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (i % 3 != 0) {
            /* GT_EXPR conditional reduction for max */
            if (arr_i[i] > max_val_i) {
                max_val_i = arr_i[i];
            }
            
            /* LT_EXPR conditional reduction for min */
            if (arr_i[i] < min_val_i) {
                min_val_i = arr_i[i];
            }
            
            /* Combined condition with logical AND */
            if (arr_i[i] > thresh_i && i % 2 == 0) {
                count_gt++;
            }
        }
    }
    
    /* ====== Loop 2: GE_EXPR conditional sum ====== */
    /* This should trigger GE_EXPR -> BIT_NOT_EXPR, BIT_IOR_EXPR */
    float sum_ge = 0.0f;
    for (int i = 0; i < 64; i++) {
        /* GE_EXPR conditional sum */
        if (arr_f[i] >= thresh_f) {
            sum_ge += arr_f[i];
        }
        
        /* Additional reduction with different type */
        if (arr_us[i] >= (unsigned short)thresh_i) {
            max_val_us = (arr_us[i] > max_val_us) ? arr_us[i] : max_val_us;
        }
    }
    cond_sum_f += sum_ge;
    
    /* ====== Loop 3: LE_EXPR with while loop ====== */
    /* This should trigger LE_EXPR -> BIT_NOT_EXPR, BIT_IOR_EXPR with swap */
    int idx = 0;
    double temp_min = arr_d[0];
    while (idx < 64) {
        /* LE_EXPR conditional reduction */
        if (arr_d[idx] <= thresh_d) {
            cond_sum_d += arr_d[idx];
            count_le++;
        }
        
        /* Nested conditional */
        if (idx % 4 == 0) {
            if (arr_d[idx] <= temp_min || idx == 0) {
                temp_min = arr_d[idx];
            }
        }
        idx++;
    }
    
    /* ====== Loop 4: Mixed comparisons in single loop ====== */
    /* Contains all four comparison operators */
    int mixed_sum = 0;
    int mixed_max = arr_i[0];
    int mixed_min = arr_i[0];
    
    for (int i = 0; i < 64; i++) {
        /* GT_EXPR */
        if (arr_i[i] > mixed_max) {
            mixed_max = arr_i[i];
        }
        
        /* GE_EXPR with ternary */
        mixed_sum += (arr_i[i] >= thresh_i) ? arr_i[i] : 0;
        
        /* LT_EXPR */
        if (arr_i[i] < mixed_min && i % 3 != 0) {
            mixed_min = arr_i[i];
        }
        
        /* LE_EXPR with logical OR */
        if (arr_i[i] <= thresh_i || i % 5 == 0) {
            count_gt++;  /* Reuse counter */
        }
    }
    
    /* ====== Loop 5: Floating-point comparisons ====== */
    float fp_max = arr_f[0];
    float fp_min = arr_f[0];
    for (int i = 0; i < 64; i++) {
        /* GT_EXPR for float */
        if (arr_f[i] > fp_max) {
            fp_max = arr_f[i];
        }
        
        /* LE_EXPR for float */
        if (arr_f[i] <= fp_min) {
            fp_min = arr_f[i];
        }
    }
    
    /* Aggregate results into checksum */
    int checksum = 0;
    checksum += max_val_i;
    checksum += min_val_i;
    checksum += max_val_us;
    checksum += min_val_us;
    checksum += (int)cond_sum_f;
    checksum += (int)cond_sum_d;
    checksum += count_gt;
    checksum += count_le;
    checksum += mixed_sum;
    checksum += mixed_max;
    checksum += mixed_min;
    checksum += (int)fp_max;
    checksum += (int)fp_min;
    
    /* Store to volatile to prevent dead code elimination */
    g_result_store[0] = checksum;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
