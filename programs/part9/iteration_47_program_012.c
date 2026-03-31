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
    return (float)((i * 7 + seed * 3) % 100) / 2.0f;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    
    /* Arrays with different types */
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
    volatile int thresh_int = seed + 25;
    volatile float thresh_float = (float)(seed + 30) / 2.0f;
    volatile unsigned short thresh_us = (unsigned short)((seed + 15) & 0xFFFF);
    volatile double thresh_double = (double)(seed + 40) / 3.0;
    
    /* Reduction variables */
    int max_val = -1000;
    int min_val = 1000;
    unsigned short max_us = 0;
    float sum_ge = 0.0f;
    double sum_le = 0.0;
    int count_gt = 0;
    int count_lt = 0;
    
    /* ===== Loop 1: GT_EXPR (>) with multiple reductions ===== */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (i % 3 == 0) {
            /* GT_EXPR: if (arr_i[i] > thresh_int) */
            if (arr_i[i] > thresh_int) {
                max_val = (arr_i[i] > max_val) ? arr_i[i] : max_val;
                count_gt++;
            }
            
            /* Another GT_EXPR with different type */
            if (arr_us[i] > thresh_us) {
                max_us = (arr_us[i] > max_us) ? arr_us[i] : max_us;
            }
        }
    }
    
    /* ===== Loop 2: GE_EXPR (>=) with logical operators ===== */
    int temp = seed;
    while (temp < 64) {
        /* Combined condition with logical AND */
        if (temp % 4 == 0 && arr_f[temp] >= thresh_float) {
            sum_ge += arr_f[temp];
        }
        
        /* Nested conditional */
        if (arr_i[temp] >= thresh_int - 5) {
            if (temp % 2 == 0) {
                max_val = (arr_i[temp] > max_val) ? arr_i[temp] : max_val;
            }
        }
        temp++;
    }
    
    /* ===== Loop 3: LT_EXPR (<) with while loop ===== */
    int j = 0;
    int limit = 64 - seed % 10;
    while (j < limit) {
        /* LT_EXPR: if (arr_d[j] < thresh_double) */
        if (arr_d[j] < thresh_double) {
            sum_le += arr_d[j];
            count_lt++;
        }
        
        /* Multiple reductions with LT_EXPR */
        if (arr_i[j] < thresh_int + 10) {
            min_val = (arr_i[j] < min_val) ? arr_i[j] : min_val;
        }
        j++;
    }
    
    /* ===== Loop 4: LE_EXPR (<=) with mixed conditions ===== */
    for (int i = 0; i < 64; i += 2) {
        /* LE_EXPR: if (arr_f[i] <= thresh_float + 5.0f) */
        if (arr_f[i] <= thresh_float + 5.0f) {
            sum_ge += arr_f[i] * 0.5f;
        }
        
        /* Logical OR with LE_EXPR */
        if (i < 32 || arr_i[i] <= thresh_int - 3) {
            count_gt += (arr_i[i] > 0) ? 1 : 0;
        }
    }
    
    /* ===== Loop 5: All comparison operators in one loop ===== */
    int all_max = -1000;
    int all_min = 1000;
    float all_sum = 0.0f;
    int all_count = 0;
    
    for (int i = 0; i < 64; i++) {
        /* GT_EXPR */
        if (arr_i[i] > thresh_int) {
            all_max = (arr_i[i] > all_max) ? arr_i[i] : all_max;
        }
        
        /* GE_EXPR */
        if (arr_f[i] >= thresh_float - 2.0f) {
            all_sum += arr_f[i];
        }
        
        /* LT_EXPR */
        if (arr_i[i] < thresh_int + 5) {
            all_min = (arr_i[i] < all_min) ? arr_i[i] : all_min;
        }
        
        /* LE_EXPR */
        if (i <= 50 && arr_f[i] <= thresh_float + 3.0f) {
            all_count++;
        }
    }
    
    /* Aggregate results into checksum */
    int checksum = max_val + min_val + max_us + (int)sum_ge + (int)sum_le + 
                   count_gt + count_lt + all_max + all_min + (int)all_sum + all_count;
    
    /* Store to volatile to prevent elimination */
    g_result_int = checksum;
    g_result_float = sum_ge;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
