#include <stdio.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_sink;
volatile float g_volatile_float_sink;

/* Function to generate deterministic data */
static inline int gen_value(int i, int seed) {
    return (i * 3 + seed) & 0xFF;  /* Keep values in range */
}

static inline float gen_float(int i, int seed) {
    return (float)((i * 5 + seed) & 0xFF) * 0.75f;
}

int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Arrays with different types */
    int arr_i[64];
    unsigned short arr_us[64];
    float arr_f[64];
    double arr_d[64];
    
    /* Initialize arrays */
    for (int i = 0; i < 64; i++) {
        arr_i[i] = gen_value(i, seed);
        arr_us[i] = (unsigned short)gen_value(i, seed + 1);
        arr_f[i] = gen_float(i, seed);
        arr_d[i] = (double)gen_float(i, seed + 2);
    }
    
    /* Loop-invariant thresholds from volatile sources */
    volatile int vi_thresh = seed % 100;
    volatile float vf_thresh = (seed % 100) * 0.5f;
    volatile double vd_thresh = (seed % 100) * 0.25;
    
    int thresh_i = vi_thresh;
    float thresh_f = vf_thresh;
    double thresh_d = vd_thresh;
    unsigned short thresh_us = (unsigned short)(vi_thresh & 0xFFFF);
    
    /* Reduction variables */
    int max_val_i = -1000;
    int min_val_i = 1000;
    unsigned short max_val_us = 0;
    float cond_sum_f = 0.0f;
    double cond_sum_d = 0.0;
    int count_gt = 0;
    int count_le = 0;
    int mixed_max = -1000;
    float mixed_min_f = 1000.0f;
    
    /* ===== Loop 1: GT_EXPR (greater-than) pattern ===== */
    /* Conditional max reduction with > comparison */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (i % 3 != 0) {
            /* GT_EXPR: if (arr_i[i] > max_val_i) max_val_i = arr_i[i] */
            if (arr_i[i] > max_val_i) {
                max_val_i = arr_i[i];
            }
            
            /* Additional reduction with different condition */
            if (arr_i[i] > thresh_i) {
                count_gt++;
            }
        }
    }
    
    /* ===== Loop 2: GE_EXPR (greater-than-or-equal) pattern ===== */
    /* Conditional sum with >= comparison */
    for (int i = 0; i < 64; i++) {
        /* GE_EXPR: sum values >= threshold */
        if (arr_f[i] >= thresh_f) {
            cond_sum_f += arr_f[i];
        }
        
        /* Nested conditional with logical OR */
        if (i < 32 || arr_f[i] >= thresh_f * 0.5f) {
            /* Another GE_EXPR inside complex condition */
            if (arr_f[i] >= thresh_f * 0.75f) {
                mixed_max = (arr_i[i] > mixed_max) ? arr_i[i] : mixed_max;
            }
        }
    }
    
    /* ===== Loop 3: LT_EXPR (less-than) pattern ===== */
    /* Conditional min reduction with < comparison */
    for (int i = 0; i < 64; i++) {
        /* LT_EXPR: if (arr_d[i] < mixed_min_f) mixed_min_f = arr_d[i] */
        if (arr_d[i] < (double)mixed_min_f) {
            mixed_min_f = (float)arr_d[i];
        }
        
        /* Multiple reductions in one loop */
        if (arr_i[i] < thresh_i) {
            min_val_i = (arr_i[i] < min_val_i) ? arr_i[i] : min_val_i;
        }
    }
    
    /* ===== Loop 4: LE_EXPR (less-than-or-equal) pattern ===== */
    /* Conditional sum with <= comparison */
    int i = 0;
    while (i < 64) {
        /* LE_EXPR: sum values <= threshold */
        if (arr_us[i] <= thresh_us) {
            cond_sum_d += arr_d[i];
            count_le++;
        }
        
        /* Combined condition with logical AND */
        if (i > 0 && arr_us[i] <= thresh_us + 10) {
            max_val_us = (arr_us[i] > max_val_us) ? arr_us[i] : max_val_us;
        }
        i++;
    }
    
    /* ===== Loop 5: Mixed comparisons in single loop ===== */
    /* Multiple conditional reductions with different comparison operators */
    int sum_gt = 0, sum_lt = 0, count_ge = 0, count_le2 = 0;
    for (int i = 0; i < 64; i++) {
        /* GT_EXPR */
        if (arr_i[i] > thresh_i + 10) {
            sum_gt += arr_i[i];
        }
        
        /* LT_EXPR */
        if (arr_i[i] < thresh_i - 10) {
            sum_lt += arr_i[i];
        }
        
        /* GE_EXPR */
        if (arr_i[i] >= thresh_i) {
            count_ge++;
        }
        
        /* LE_EXPR */
        if (arr_i[i] <= thresh_i + 5) {
            count_le2++;
        }
    }
    
    /* Prevent dead code elimination */
    g_volatile_sink = max_val_i;
    g_volatile_float_sink = cond_sum_f;
    
    /* Compute checksum */
    int checksum = max_val_i;
    checksum += min_val_i;
    checksum += max_val_us;
    checksum += (int)cond_sum_f;
    checksum += (int)cond_sum_d;
    checksum += count_gt;
    checksum += count_le;
    checksum += (int)mixed_min_f;
    checksum += mixed_max;
    checksum += sum_gt;
    checksum += sum_lt;
    checksum += count_ge;
    checksum += count_le2;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
