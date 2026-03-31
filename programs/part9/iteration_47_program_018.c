#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent dead code elimination */
volatile int g_result_int = 0;
volatile float g_result_float = 0.0f;

/* Function to generate deterministic data */
static inline int gen_value(int i, int seed) {
    return (i * 3 + seed) % 1000;
}

static inline float gen_float(int i, int seed) {
    return (float)((i * 7 + seed * 3) % 1000) / 10.0f;
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
    
    /* Initialize arrays with deterministic but non-constant values */
    for (int i = 0; i < 64; i++) {
        arr_i[i] = gen_value(i, seed);
        arr_us[i] = (unsigned short)(gen_value(i, seed + 1) & 0xFFFF);
        arr_f[i] = gen_float(i, seed);
        arr_d[i] = (double)gen_float(i, seed + 2);
    }
    
    /* Loop-invariant thresholds from volatile sources */
    volatile int thresh_int = seed + 100;
    volatile unsigned short thresh_us = (seed * 2) & 0xFFFF;
    volatile float thresh_f = (float)(seed % 50) + 25.0f;
    volatile double thresh_d = (double)(seed % 60) + 30.0;
    
    int actual_thresh_int = thresh_int;
    unsigned short actual_thresh_us = thresh_us;
    float actual_thresh_f = thresh_f;
    double actual_thresh_d = thresh_d;
    
    /* Reduction variables */
    int max_val = arr_i[0];
    int min_val = arr_i[0];
    unsigned short max_us = arr_us[0];
    float sum_gt = 0.0f;
    double sum_lt = 0.0;
    int count_ge = 0;
    int count_le = 0;
    int cond_sum_int = 0;
    float cond_sum_float = 0.0f;
    
    /* ====== Loop 1: GT_EXPR (greater-than) conditional reduction ====== */
    /* This should trigger: case GT_EXPR: bitop1 = BIT_NOT_EXPR; bitop2 = BIT_AND_EXPR; */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (i % 4 != 0) {
            /* Conditional max reduction with > */
            if (arr_i[i] > max_val) {
                max_val = arr_i[i];
            }
            
            /* Conditional sum with > */
            if (arr_f[i] > actual_thresh_f) {
                sum_gt += arr_f[i];
            }
        }
    }
    
    /* ====== Loop 2: GE_EXPR (greater-than-or-equal) conditional reduction ====== */
    /* This should trigger: case GE_EXPR: bitop1 = BIT_NOT_EXPR; bitop2 = BIT_IOR_EXPR; */
    int i = 0;
    while (i < 64) {
        /* Combined condition with logical AND */
        if (i < 60 && arr_i[i] >= actual_thresh_int) {
            cond_sum_int += arr_i[i];
            count_ge++;
        }
        
        /* Multiple reductions in one loop */
        if (arr_us[i] >= actual_thresh_us && arr_us[i] > max_us) {
            max_us = arr_us[i];
        }
        
        i++;
    }
    
    /* ====== Loop 3: LT_EXPR (less-than) conditional reduction ====== */
    /* This should trigger: case LT_EXPR with std::swap */
    for (int i = 0; i < 64; i++) {
        /* Nested conditional */
        if (arr_f[i] < 500.0f) {
            if (arr_f[i] < actual_thresh_f) {
                cond_sum_float += arr_f[i];
            }
            
            /* Conditional min with < */
            if (arr_i[i] < min_val) {
                min_val = arr_i[i];
            }
        }
    }
    
    /* ====== Loop 4: LE_EXPR (less-than-or-equal) conditional reduction ====== */
    /* This should trigger: case LE_EXPR with std::swap */
    for (int i = 0; i < 64; i++) {
        /* Complex condition with OR */
        if (arr_d[i] <= actual_thresh_d || i % 3 == 0) {
            sum_lt += arr_d[i];
            count_le++;
        }
    }
    
    /* ====== Loop 5: Mixed reductions with all comparison types ====== */
    int mixed_max = arr_i[0];
    int mixed_min = arr_i[0];
    float mixed_sum_gt = 0.0f;
    double mixed_sum_le = 0.0;
    
    for (int i = 0; i < 64; i++) {
        /* Multiple conditional reductions in one loop body */
        if (arr_i[i] > mixed_max) mixed_max = arr_i[i];           /* GT_EXPR */
        if (arr_i[i] < mixed_min) mixed_min = arr_i[i];           /* LT_EXPR */
        
        if (arr_f[i] >= actual_thresh_f) mixed_sum_gt += arr_f[i]; /* GE_EXPR */
        if (arr_d[i] <= actual_thresh_d) mixed_sum_le += arr_d[i]; /* LE_EXPR */
    }
    
    /* Aggregate results into checksum */
    int checksum = 0;
    checksum += max_val;
    checksum += min_val;
    checksum += max_us;
    checksum += (int)sum_gt;
    checksum += (int)sum_lt;
    checksum += count_ge;
    checksum += count_le;
    checksum += cond_sum_int;
    checksum += (int)cond_sum_float;
    checksum += mixed_max;
    checksum += mixed_min;
    checksum += (int)mixed_sum_gt;
    checksum += (int)mixed_sum_le;
    
    /* Store to volatile to prevent elimination */
    g_result_int = checksum;
    g_result_float = sum_gt + (float)sum_lt;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
