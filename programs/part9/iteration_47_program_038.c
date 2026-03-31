#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_seed = 0;
volatile int g_volatile_result = 0;

/* Function to generate deterministic but non-constant data */
static inline int gen_data(int i, int seed) {
    return (i * 3 + seed) ^ (seed >> 3);
}

static inline float gen_float_data(int i, int seed) {
    return (float)((i * 7 + seed * 11) % 100) / 10.0f;
}

static inline double gen_double_data(int i, int seed) {
    return (double)((i * 13 + seed * 17) % 200) / 20.0;
}

int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    g_volatile_seed = seed;  /* Prevent constant propagation */
    
    /* Arrays with different data types */
    int arr_i[64];
    unsigned short arr_us[64];
    float arr_f[64];
    double arr_d[64];
    
    /* Initialize arrays with deterministic but non-constant data */
    for (int i = 0; i < 64; i++) {
        arr_i[i] = gen_data(i, seed);
        arr_us[i] = (unsigned short)(gen_data(i, seed) & 0xFFFF);
        arr_f[i] = gen_float_data(i, seed);
        arr_d[i] = gen_double_data(i, seed);
    }
    
    /* Loop-invariant thresholds from volatile source */
    volatile int vi_thresh = g_volatile_seed + 100;
    volatile float vf_thresh = (float)(g_volatile_seed % 50) / 2.0f + 2.5f;
    volatile double vd_thresh = (double)(g_volatile_seed % 100) / 4.0 + 1.0;
    
    int thresh_i = vi_thresh;      /* > 0 comparison */
    float thresh_f = vf_thresh;    /* >= comparison */
    double thresh_d = vd_thresh;   /* < comparison */
    unsigned short thresh_us = (unsigned short)(vi_thresh & 0x7FFF); /* <= comparison */
    
    /* Reduction variables */
    int max_val_i = arr_i[0];
    int min_val_i = arr_i[0];
    float max_val_f = arr_f[0];
    double min_val_d = arr_d[0];
    unsigned short max_val_us = arr_us[0];
    int cond_sum_i = 0;
    float cond_sum_f = 0.0f;
    int count_gt = 0;
    int count_lt = 0;
    
    /* ===== Loop 1: GT_EXPR pattern (>) with multiple reductions ===== */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (arr_i[i] > 0) {
            /* GT_EXPR: if (arr_i[i] > max_val_i) max_val_i = arr_i[i]; */
            if (arr_i[i] > max_val_i) {
                max_val_i = arr_i[i];
            }
            
            /* Another GT_EXPR with different variable */
            if (arr_i[i] > thresh_i) {
                count_gt++;
            }
            
            /* Combined with logical AND */
            if (arr_i[i] > 0 && arr_i[i] > max_val_i / 2) {
                cond_sum_i += arr_i[i];
            }
        }
    }
    
    /* ===== Loop 2: GE_EXPR pattern (>=) with while loop ===== */
    int j = 0;
    while (j < 64) {
        /* GE_EXPR: if (arr_f[j] >= max_val_f) max_val_f = arr_f[j]; */
        if (arr_f[j] >= max_val_f) {
            max_val_f = arr_f[j];
        }
        
        /* GE_EXPR with conditional sum */
        if (arr_f[j] >= thresh_f) {
            cond_sum_f += arr_f[j];
        }
        
        /* Nested conditionals */
        if (j % 2 == 0) {
            if (arr_f[j] >= thresh_f / 2.0f && arr_f[j] < thresh_f * 2.0f) {
                /* Additional operation to make loop body non-trivial */
                cond_sum_f += 0.5f;
            }
        }
        j++;
    }
    
    /* ===== Loop 3: LT_EXPR pattern (<) with multiple data types ===== */
    for (int i = 0; i < 64; i++) {
        /* LT_EXPR: if (arr_d[i] < min_val_d) min_val_d = arr_d[i]; */
        if (arr_d[i] < min_val_d) {
            min_val_d = arr_d[i];
        }
        
        /* LT_EXPR with unsigned short */
        if (arr_us[i] < thresh_us) {
            count_lt++;
        }
        
        /* Logical OR combined with LT_EXPR */
        if (arr_d[i] < thresh_d || arr_d[i] < min_val_d * 1.1) {
            /* Additional computation */
            min_val_d = (arr_d[i] < min_val_d) ? arr_d[i] : min_val_d;
        }
    }
    
    /* ===== Loop 4: LE_EXPR pattern (<=) with complex conditions ===== */
    unsigned short current_min = arr_us[0];
    unsigned short current_max = arr_us[0];
    int sum_le = 0;
    
    for (int i = 0; i < 64; i++) {
        /* LE_EXPR: if (arr_us[i] <= current_min) current_min = arr_us[i]; */
        if (arr_us[i] <= current_min) {
            current_min = arr_us[i];
        }
        
        /* LE_EXPR for max with different threshold */
        if (arr_us[i] <= thresh_us) {
            sum_le += arr_us[i];
        }
        
        /* Multiple reductions in one loop */
        if (arr_us[i] >= current_max) {
            current_max = arr_us[i];
        }
        
        /* Combined conditions with logical operators */
        if ((arr_us[i] <= thresh_us || arr_us[i] >= thresh_us / 2) && 
            arr_us[i] > 0) {
            sum_le += 1;
        }
    }
    
    /* ===== Loop 5: Mixed comparisons in single loop ===== */
    int mixed_sum = 0;
    float mixed_max_f = arr_f[0];
    double mixed_min_d = arr_d[0];
    
    for (int i = 0; i < 64; i++) {
        /* All four comparison types in one loop */
        if (arr_i[i] > max_val_i / 2) {          /* GT_EXPR */
            mixed_sum += arr_i[i];
        }
        
        if (arr_f[i] >= thresh_f) {              /* GE_EXPR */
            mixed_max_f = (arr_f[i] > mixed_max_f) ? arr_f[i] : mixed_max_f;
        }
        
        if (arr_d[i] < thresh_d * 1.5) {         /* LT_EXPR */
            mixed_min_d = (arr_d[i] < mixed_min_d) ? arr_d[i] : mixed_min_d;
        }
        
        if (arr_us[i] <= thresh_us + 100) {      /* LE_EXPR */
            mixed_sum += 1;
        }
    }
    
    /* Compute checksum from all results */
    int checksum = 0;
    checksum += max_val_i;
    checksum += min_val_i;
    checksum += (int)max_val_f;
    checksum += (int)(min_val_d * 100);
    checksum += max_val_us;
    checksum += cond_sum_i;
    checksum += (int)cond_sum_f;
    checksum += count_gt;
    checksum += count_lt;
    checksum += current_min;
    checksum += current_max;
    checksum += sum_le;
    checksum += mixed_sum;
    checksum += (int)(mixed_max_f * 10);
    checksum += (int)(mixed_min_d * 100);
    
    /* Store to volatile to prevent dead code elimination */
    g_volatile_result = checksum;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
