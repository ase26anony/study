#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int g_result_int = 0;
volatile float g_result_float = 0.0f;

/* Function to create loop-invariant thresholds */
int get_threshold(int seed) {
    volatile int v = seed;
    return v % 100 + 50;  /* Returns 50-149 based on seed */
}

float get_float_threshold(int seed) {
    volatile float v = (float)(seed % 100);
    return v * 0.5f + 25.0f;  /* Returns 25.0-74.5 */
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    
    /* Initialize arrays with deterministic but non-constant values */
    int arr_i[64];
    unsigned short arr_us[64];
    float arr_f[64];
    double arr_d[64];
    
    for (int i = 0; i < 64; i++) {
        arr_i[i] = (i * 3 + seed) % 200 - 100;      /* Values between -100 and 99 */
        arr_us[i] = (unsigned short)((i * 5 + seed) % 65535);
        arr_f[i] = (float)((i * 7 + seed) % 100) * 0.7f - 35.0f;  /* -35.0 to 34.3 */
        arr_d[i] = (double)((i * 11 + seed) % 200) * 0.3 - 30.0;  /* -30.0 to 29.7 */
    }
    
    /* Get loop-invariant thresholds */
    int thresh_i = get_threshold(seed);
    float thresh_f = get_float_threshold(seed);
    unsigned short thresh_us = (unsigned short)(seed % 1000 + 1000);
    double thresh_d = (double)(seed % 200) * 0.25 + 10.0;
    
    /* Reduction variables */
    int max_val_i = arr_i[0];
    int min_val_i = arr_i[0];
    unsigned short max_val_us = arr_us[0];
    float cond_sum_f = 0.0f;
    double cond_sum_d = 0.0;
    int count_gt = 0;
    int count_le = 0;
    int mixed_sum = 0;
    
    /* ===== Loop 1: GT_EXPR (greater-than) with multiple reductions ===== */
    /* This should trigger the GT_EXPR -> BIT_NOT_EXPR, BIT_AND_EXPR transformation */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (arr_i[i] > -1000) {  /* Always true, but not trivially eliminable */
            /* Conditional max reduction with > */
            if (arr_i[i] > max_val_i) {
                max_val_i = arr_i[i];
            }
            
            /* Another conditional with > using logical AND */
            if (arr_i[i] > thresh_i && arr_i[i] < 100) {
                count_gt++;
            }
            
            /* Mixed type comparison */
            if ((float)arr_i[i] > thresh_f) {
                mixed_sum += arr_i[i];
            }
        }
    }
    
    /* ===== Loop 2: GE_EXPR (greater-than-or-equal) ===== */
    /* This should trigger the GE_EXPR -> BIT_NOT_EXPR, BIT_IOR_EXPR transformation */
    int sum_ge = 0;
    int max_ge = arr_i[0];
    for (int i = 0; i < 64; i++) {
        /* Conditional sum with >= */
        if (arr_i[i] >= thresh_i) {
            sum_ge += arr_i[i];
        }
        
        /* Nested conditional with logical OR */
        if (arr_i[i] >= (thresh_i - 10) || arr_i[i] >= 0) {
            if (arr_i[i] > max_ge) {
                max_ge = arr_i[i];
            }
        }
    }
    
    /* ===== Loop 3: LT_EXPR (less-than) ===== */
    /* This should trigger the LT_EXPR -> BIT_NOT_EXPR, BIT_AND_EXPR with swap */
    int min_lt = arr_i[0];
    int count_lt = 0;
    int i = 0;
    /* Use while loop variant */
    while (i < 64) {
        /* Conditional min reduction with < */
        if (arr_i[i] < min_lt) {
            min_lt = arr_i[i];
        }
        
        /* Another conditional with < */
        if (arr_i[i] < (thresh_i + 20)) {
            count_lt++;
        }
        
        /* Combined condition with logical AND */
        if (arr_i[i] < thresh_i && arr_us[i] < thresh_us) {
            cond_sum_f += arr_f[i];
        }
        i++;
    }
    
    /* ===== Loop 4: LE_EXPR (less-than-or-equal) ===== */
    /* This should trigger the LE_EXPR -> BIT_NOT_EXPR, BIT_IOR_EXPR with swap */
    float min_le_f = arr_f[0];
    double min_le_d = arr_d[0];
    for (int i = 0; i < 64; i++) {
        /* Multiple reductions with <= in one loop */
        if (arr_f[i] <= min_le_f) {
            min_le_f = arr_f[i];
        }
        
        if (arr_d[i] <= min_le_d) {
            min_le_d = arr_d[i];
        }
        
        /* Conditional sum with <= */
        if (arr_f[i] <= thresh_f) {
            cond_sum_f += arr_f[i];
        }
        
        if (arr_d[i] <= thresh_d) {
            cond_sum_d += arr_d[i];
        }
        
        /* Count with <= */
        if (arr_i[i] <= thresh_i) {
            count_le++;
        }
    }
    
    /* ===== Loop 5: Mixed comparisons in single loop ===== */
    /* Tests multiple conditional reductions simultaneously */
    int final_max = arr_i[0];
    int final_min = arr_i[0];
    int sum_mixed = 0;
    int count_mixed = 0;
    
    for (int i = 0; i < 64; i++) {
        /* All four comparison types in one loop */
        if (arr_i[i] > final_max) final_max = arr_i[i];           /* GT_EXPR */
        if (arr_i[i] < final_min) final_min = arr_i[i];           /* LT_EXPR */
        if (arr_i[i] >= thresh_i) sum_mixed += arr_i[i];          /* GE_EXPR */
        if (arr_i[i] <= thresh_i) count_mixed++;                  /* LE_EXPR */
        
        /* Additional floating-point conditional */
        if (arr_f[i] > thresh_f) {
            cond_sum_d += (double)arr_f[i];
        }
    }
    
    /* Aggregate results into checksum */
    uint32_t checksum = 0;
    checksum += (uint32_t)max_val_i;
    checksum += (uint32_t)min_val_i;
    checksum += (uint32_t)max_val_us;
    checksum += (uint32_t)count_gt;
    checksum += (uint32_t)count_le;
    checksum += (uint32_t)sum_ge;
    checksum += (uint32_t)max_ge;
    checksum += (uint32_t)min_lt;
    checksum += (uint32_t)count_lt;
    checksum += (uint32_t)(cond_sum_f * 100.0f);  /* Scale float to get integer part */
    checksum += (uint32_t)(cond_sum_d * 100.0);
    checksum += (uint32_t)(min_le_f * 100.0f);
    checksum += (uint32_t)(min_le_d * 100.0);
    checksum += (uint32_t)final_max;
    checksum += (uint32_t)final_min;
    checksum += (uint32_t)sum_mixed;
    checksum += (uint32_t)count_mixed;
    checksum += (uint32_t)mixed_sum;
    
    /* Store to volatile to prevent dead code elimination */
    g_result_int = (int)checksum;
    g_result_float = (float)checksum;
    
    printf("Checksum: %u\n", checksum);
    
    return 0;
}
