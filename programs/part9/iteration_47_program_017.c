#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile globals to prevent dead code elimination */
volatile int g_result_int = 0;
volatile float g_result_float = 0.0f;

/* Function to generate deterministic data */
static inline int gen_val(int i, int seed) {
    return (i * 3 + seed) % 100;
}

static inline float gen_fval(int i, int seed) {
    return (float)((i * 7 + seed * 3) % 1000) / 10.0f;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    int checksum = 0;
    
    /* Arrays with different data types */
    int arr_i[64];
    unsigned short arr_us[64];
    float arr_f[64];
    double arr_d[64];
    
    /* Initialize arrays with deterministic but non-constant values */
    for (int i = 0; i < 64; i++) {
        arr_i[i] = gen_val(i, seed);
        arr_us[i] = (unsigned short)(gen_val(i, seed + 1) & 0xFFFF);
        arr_f[i] = gen_fval(i, seed);
        arr_d[i] = (double)gen_fval(i, seed + 2);
    }
    
    /* Loop-invariant thresholds from volatile sources */
    volatile int thresh_int = seed + 50;
    volatile float thresh_float = (float)(seed % 50) + 25.5f;
    volatile unsigned short thresh_us = (seed * 3) % 100;
    volatile double thresh_double = (double)(seed % 40) + 30.0;
    
    /* ========== GREATER-THAN (GT_EXPR) reductions ========== */
    {
        /* Conditional max with > */
        int max_val = arr_i[0];
        for (int i = 0; i < 64; i++) {
            /* Outer if to complicate control flow */
            if (i % 2 == 0) {
                if (arr_i[i] > max_val) {
                    max_val = arr_i[i];
                }
            }
        }
        checksum += max_val;
        
        /* Multiple reductions in one loop with > */
        float sum_gt = 0.0f;
        int count_gt = 0;
        for (int i = 0; i < 64; i++) {
            /* Combined condition with logical AND */
            if (arr_f[i] > thresh_float && i < 60) {
                sum_gt += arr_f[i];
                count_gt++;
            }
        }
        checksum += (int)sum_gt + count_gt;
    }
    
    /* ========== GREATER-THAN-OR-EQUAL (GE_EXPR) reductions ========== */
    {
        /* Conditional min with >= in a while loop */
        int i = 0;
        int min_val = arr_i[0];
        while (i < 64) {
            /* Nested conditional */
            if (arr_i[i] >= min_val) {
                /* Do nothing for max, but we'll do min with swapped logic */
            } else {
                min_val = arr_i[i];
            }
            i++;
        }
        checksum += min_val;
        
        /* Sum with >= condition and multiple data types */
        double sum_ge = 0.0;
        unsigned short max_us = arr_us[0];
        for (int i = 0; i < 64; i++) {
            /* Two different reductions with >= */
            if (arr_d[i] >= thresh_double) {
                sum_ge += arr_d[i];
            }
            if (arr_us[i] >= max_us) {
                max_us = arr_us[i];
            }
        }
        checksum += (int)sum_ge + max_us;
    }
    
    /* ========== LESS-THAN (LT_EXPR) reductions ========== */
    {
        /* Conditional reduction with < */
        int sum_lt = 0;
        for (int i = 0; i < 64; i++) {
            /* Using logical OR to combine conditions */
            if (arr_i[i] < thresh_int || (i % 3 == 0)) {
                sum_lt += arr_i[i];
            }
        }
        checksum += sum_lt;
        
        /* Find min with < comparison */
        float min_f = arr_f[0];
        for (int i = 0; i < 64; i++) {
            if (arr_f[i] < min_f) {
                min_f = arr_f[i];
            }
        }
        checksum += (int)(min_f * 100);
    }
    
    /* ========== LESS-THAN-OR-EQUAL (LE_EXPR) reductions ========== */
    {
        /* Multiple reductions with <= in one loop */
        int max_le = arr_i[0];
        int count_le = 0;
        for (int i = 0; i < 64; i++) {
            /* Complex condition */
            if (arr_i[i] <= max_le) {
                /* This is actually finding max with swapped logic */
                max_le = arr_i[i];
            }
            if (arr_us[i] <= thresh_us) {
                count_le++;
            }
        }
        checksum += max_le + count_le;
        
        /* While loop with <= condition */
        int j = 0;
        double prod_le = 1.0;
        while (j < 64) {
            if (arr_d[j] <= thresh_double * 2) {
                prod_le *= (arr_d[j] + 1.0);
            }
            j++;
        }
        checksum += (int)prod_le;
    }
    
    /* ========== MIXED COMPARISONS IN ONE LOOP ========== */
    {
        /* Four different conditional reductions in a single loop */
        int max_mixed = arr_i[0];
        int min_mixed = arr_i[0];
        int sum_gt_mixed = 0;
        int sum_lt_mixed = 0;
        
        for (int i = 0; i < 64; i++) {
            /* GT reduction */
            if (arr_i[i] > max_mixed) {
                max_mixed = arr_i[i];
            }
            
            /* LT reduction (for min) */
            if (arr_i[i] < min_mixed) {
                min_mixed = arr_i[i];
            }
            
            /* GE reduction */
            if (arr_i[i] >= thresh_int) {
                sum_gt_mixed += arr_i[i];
            }
            
            /* LE reduction */
            if (arr_i[i] <= thresh_int + 10) {
                sum_lt_mixed += arr_i[i];
            }
        }
        
        checksum += max_mixed + min_mixed + sum_gt_mixed + sum_lt_mixed;
    }
    
    /* Store to volatile to prevent optimization */
    g_result_int = checksum;
    g_result_float = (float)checksum;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
