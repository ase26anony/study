#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile globals to prevent dead code elimination */
volatile int g_volatile_int = 0;
volatile float g_volatile_float = 0.0f;

/* Function to generate deterministic data */
static inline int gen_val(int i, int seed) {
    return (i * 3 + seed) & 0xFF;  /* Keep values in range */
}

static inline float gen_float(int i, int seed) {
    return (float)((i * 7 + seed * 3) & 0xFF) * 0.5f;
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
    
    /* Initialize arrays deterministically */
    for (int i = 0; i < 64; i++) {
        arr_i[i] = gen_val(i, seed);
        arr_us[i] = (unsigned short)gen_val(i, seed ^ 0x55);
        arr_f[i] = gen_float(i, seed);
        arr_d[i] = (double)gen_float(i, seed ^ 0xAA);
    }
    
    /* Loop-invariant thresholds from volatile sources */
    volatile int vi = seed + 100;
    volatile float vf = (float)(seed % 50) + 25.0f;
    int threshold_i = vi;  /* Not compile-time constant */
    float threshold_f = vf;
    unsigned short threshold_us = (unsigned short)(seed + 50);
    double threshold_d = (double)(seed % 30) + 15.0;
    
    /* Reduction variables */
    int max_val_i = arr_i[0];
    int min_val_i = arr_i[0];
    unsigned short max_val_us = arr_us[0];
    float cond_sum_f = 0.0f;
    double cond_sum_d = 0.0;
    int count_gt = 0;
    int count_le = 0;
    
    /* ===== Loop 1: GT_EXPR and LT_EXPR with multiple reductions ===== */
    /* This loop tests > and < operators with swapped operands */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (arr_i[i] != 0) {
            /* GT_EXPR pattern: if (arr_i[i] > max_val_i) */
            if (arr_i[i] > max_val_i) {
                max_val_i = arr_i[i];
            }
            
            /* LT_EXPR pattern: if (arr_i[i] < min_val_i) */
            /* Note: This will trigger std::swap(cond_expr0, cond_expr1) */
            if (arr_i[i] < min_val_i) {
                min_val_i = arr_i[i];
            }
            
            /* Combined with logical AND */
            if (arr_i[i] > threshold_i && arr_i[i] < (threshold_i + 20)) {
                count_gt++;
            }
        }
    }
    
    /* ===== Loop 2: GE_EXPR conditional sum ===== */
    /* Tests >= operator with BIT_NOT_EXPR and BIT_IOR_EXPR */
    for (int i = 0; i < 64; i++) {
        /* Nested conditionals */
        if (arr_us[i] > 10) {
            /* GE_EXPR pattern: if (arr_us[i] >= max_val_us) */
            if (arr_us[i] >= max_val_us) {
                max_val_us = arr_us[i];
            }
            
            /* Conditional sum with GE_EXPR */
            if (arr_us[i] >= threshold_us) {
                cond_sum_f += (float)arr_us[i];
            }
        }
    }
    
    /* ===== Loop 3: LE_EXPR with while loop ===== */
    /* Tests <= operator with swap */
    int j = 0;
    while (j < 64) {
        /* LE_EXPR pattern: if (arr_f[j] <= threshold_f) */
        /* Note: This will trigger std::swap(cond_expr0, cond_expr1) */
        if (arr_f[j] <= threshold_f) {
            cond_sum_d += (double)arr_f[j];
            count_le++;
        }
        
        /* Additional condition with logical OR */
        if (arr_f[j] <= threshold_f || arr_f[j] >= (threshold_f * 2.0f)) {
            /* Do nothing, just complicate control flow */
        }
        j++;
    }
    
    /* ===== Loop 4: Mixed comparisons in single loop ===== */
    /* Tests all four operators in one loop */
    float max_f = arr_f[0];
    float min_f = arr_f[0];
    double sum_ge_d = 0.0;
    double sum_le_d = 0.0;
    
    for (int i = 0; i < 64; i++) {
        /* GT_EXPR for floats */
        if (arr_f[i] > max_f) {
            max_f = arr_f[i];
        }
        
        /* LT_EXPR for floats (with swap) */
        if (arr_f[i] < min_f) {
            min_f = arr_f[i];
        }
        
        /* GE_EXPR conditional sum */
        if (arr_d[i] >= threshold_d) {
            sum_ge_d += arr_d[i];
        }
        
        /* LE_EXPR conditional sum (with swap) */
        if (arr_d[i] <= threshold_d * 1.5) {
            sum_le_d += arr_d[i];
        }
    }
    
    /* Prevent optimization */
    g_volatile_int = max_val_i;
    g_volatile_float = max_f;
    
    /* Compute checksum */
    int checksum = 0;
    checksum += max_val_i;
    checksum += min_val_i;
    checksum += max_val_us;
    checksum += (int)cond_sum_f;
    checksum += (int)cond_sum_d;
    checksum += count_gt;
    checksum += count_le;
    checksum += (int)max_f;
    checksum += (int)min_f;
    checksum += (int)sum_ge_d;
    checksum += (int)sum_le_d;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
