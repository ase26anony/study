#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_sink;

/* Function to generate deterministic data */
static inline int gen_val(int i, int seed) {
    return (i * 3 + seed) & 0xFF;  /* Keep values in range */
}

static inline float gen_fval(int i, int seed) {
    return (float)((i * 5 + seed) & 0xFF) * 0.7f;
}

static inline double gen_dval(int i, int seed) {
    return (double)((i * 7 + seed) & 0xFF) * 0.3;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    
    /* Arrays with different types */
    int arr_i[64];
    unsigned short arr_us[64];
    float arr_f[64];
    double arr_d[64];
    
    /* Initialize arrays */
    for (int i = 0; i < 64; i++) {
        arr_i[i] = gen_val(i, seed);
        arr_us[i] = (unsigned short)gen_val(i, seed + 1);
        arr_f[i] = gen_fval(i, seed + 2);
        arr_d[i] = gen_dval(i, seed + 3);
    }
    
    /* Loop-invariant thresholds from volatile source */
    volatile int vi_thresh = seed + 10;
    volatile float vf_thresh = (float)(seed + 20) * 0.5f;
    volatile double vd_thresh = (double)(seed + 30) * 0.25;
    
    int thresh_i = vi_thresh;
    float thresh_f = vf_thresh;
    double thresh_d = vd_thresh;
    
    /* Reduction variables */
    int max_val_i = arr_i[0];
    int min_val_i = arr_i[0];
    unsigned short max_val_us = arr_us[0];
    float cond_sum_f = 0.0f;
    double cond_sum_d = 0.0;
    int count_gt = 0;
    int count_le = 0;
    
    /* Test 1: GT_EXPR (> operator) with nested conditional */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (arr_i[i] > 0) {
            /* Conditional max reduction with > */
            if (arr_i[i] > max_val_i) {
                max_val_i = arr_i[i];
            }
            
            /* Combined with another condition using && */
            if (arr_i[i] > thresh_i && arr_i[i] < 255) {
                count_gt++;
            }
        }
    }
    
    /* Test 2: GE_EXPR (>= operator) with multiple reductions */
    float max_val_f = arr_f[0];
    float min_val_f = arr_f[0];
    
    for (int i = 0; i < 64; i++) {
        /* Conditional max/min with >= and <= in same loop */
        if (arr_f[i] >= max_val_f) {
            max_val_f = arr_f[i];
        }
        
        if (arr_f[i] <= min_val_f) {
            min_val_f = arr_f[i];
        }
        
        /* Conditional sum with >= */
        if (arr_f[i] >= thresh_f) {
            cond_sum_f += arr_f[i];
        }
    }
    
    /* Test 3: LT_EXPR (< operator) with while loop */
    int j = 0;
    int limit = 64;
    while (j < limit) {
        /* Conditional min with < */
        if (arr_i[j] < min_val_i) {
            min_val_i = arr_i[j];
        }
        
        /* Another reduction with different type */
        if ((int)arr_us[j] < thresh_i) {
            cond_sum_d += (double)arr_us[j];
        }
        j++;
    }
    
    /* Test 4: LE_EXPR (<= operator) with logical OR */
    double max_val_d = arr_d[0];
    
    for (int i = 0; i < 64; i++) {
        /* Complex condition with || */
        if (arr_d[i] <= max_val_d || arr_d[i] <= thresh_d) {
            /* Actually update max with inverse logic */
            if (arr_d[i] > max_val_d) {
                max_val_d = arr_d[i];
            }
        }
        
        /* Count values <= threshold */
        if (arr_d[i] <= thresh_d) {
            count_le++;
        }
    }
    
    /* Test 5: Mixed reductions in single loop with all operators */
    int mixed_max = arr_i[0];
    int mixed_min = arr_i[0];
    int mixed_sum_gt = 0;
    int mixed_count_le = 0;
    
    for (int i = 0; i < 64; i++) {
        /* > comparison */
        if (arr_i[i] > mixed_max) {
            mixed_max = arr_i[i];
        }
        
        /* < comparison */
        if (arr_i[i] < mixed_min) {
            mixed_min = arr_i[i];
        }
        
        /* >= comparison for sum */
        if (arr_i[i] >= thresh_i) {
            mixed_sum_gt += arr_i[i];
        }
        
        /* <= comparison for count */
        if (arr_i[i] <= thresh_i + 5) {
            mixed_count_le++;
        }
    }
    
    /* Prevent dead code elimination */
    g_volatile_sink = max_val_i + min_val_i + count_gt;
    
    /* Compute checksum */
    int checksum = 0;
    checksum += max_val_i;
    checksum += min_val_i;
    checksum += max_val_us;
    checksum += (int)max_val_f;
    checksum += (int)min_val_f;
    checksum += (int)cond_sum_f;
    checksum += (int)max_val_d;
    checksum += (int)cond_sum_d;
    checksum += count_gt;
    checksum += count_le;
    checksum += mixed_max;
    checksum += mixed_min;
    checksum += mixed_sum_gt;
    checksum += mixed_count_le;
    
    printf("%d\n", checksum);
    
    return 0;
}
