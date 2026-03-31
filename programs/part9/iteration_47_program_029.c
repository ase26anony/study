#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent dead code elimination */
volatile int global_sink;

/* Function to generate deterministic but non-constant data */
static int gen_value(int i, int seed) {
    return (i * 3 + seed) ^ (seed >> 2);
}

static float gen_float(int i, int seed) {
    return (float)((i * 7 + seed * 11) % 100) / 10.0f;
}

static double gen_double(int i, int seed) {
    return (double)((i * 13 + seed * 17) % 200) / 20.0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <seed>\n", argv[0]);
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
        arr_us[i] = (unsigned short)(gen_value(i, seed) & 0xFFFF);
        arr_f[i] = gen_float(i, seed);
        arr_d[i] = gen_double(i, seed);
    }
    
    /* Loop-invariant thresholds from volatile sources */
    volatile int vi_threshold = seed + 50;
    volatile float vf_threshold = (float)(seed % 30) + 15.0f;
    volatile double vd_threshold = (double)(seed % 40) + 20.0;
    
    int int_threshold = vi_threshold;
    float float_threshold = vf_threshold;
    double double_threshold = vd_threshold;
    
    /* Reduction variables */
    int max_val_i = -1000000;
    int min_val_i = 1000000;
    unsigned short max_val_us = 0;
    unsigned short min_val_us = 0xFFFF;
    float cond_sum_f = 0.0f;
    double cond_sum_d = 0.0;
    int count_gt = 0;
    int count_le = 0;
    
    /* ===== Loop 1: GT_EXPR (greater-than) conditional reduction ===== */
    /* Find maximum with > comparison */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (arr_i[i] > 0) {
            /* Conditional reduction with > */
            if (arr_i[i] > max_val_i) {
                max_val_i = arr_i[i];
            }
        }
    }
    
    /* ===== Loop 2: GE_EXPR (greater-than-or-equal) conditional reduction ===== */
    /* Sum values >= threshold */
    for (int i = 0; i < 64; i++) {
        /* Combine with logical OR in condition */
        if (arr_f[i] >= float_threshold || i % 2 == 0) {
            cond_sum_f += arr_f[i];
        }
    }
    
    /* ===== Loop 3: LT_EXPR (less-than) conditional reduction ===== */
    /* Find minimum with < comparison */
    for (int i = 0; i < 64; i++) {
        /* Nested conditionals */
        if (arr_i[i] < 1000) {
            if (arr_i[i] < min_val_i) {
                min_val_i = arr_i[i];
            }
        }
    }
    
    /* ===== Loop 4: LE_EXPR (less-than-or-equal) conditional reduction ===== */
    /* Multiple reductions in one loop with <= comparison */
    {
        int i = 0;
        /* while loop variant */
        while (i < 64) {
            /* Multiple conditional reductions with different conditions */
            if (arr_d[i] <= double_threshold) {
                cond_sum_d += arr_d[i];
                count_le++;
            }
            
            /* Combined condition with logical AND */
            if (arr_i[i] > int_threshold && arr_i[i] <= max_val_i) {
                count_gt++;
            }
            i++;
        }
    }
    
    /* ===== Loop 5: Mixed reductions with all comparison types ===== */
    /* Process unsigned short array with multiple reductions */
    for (int i = 0; i < 64; i++) {
        /* GT_EXPR for unsigned short */
        if (arr_us[i] > max_val_us) {
            max_val_us = arr_us[i];
        }
        
        /* LT_EXPR for unsigned short */
        if (arr_us[i] < min_val_us) {
            min_val_us = arr_us[i];
        }
        
        /* GE_EXPR conditional sum */
        if (arr_us[i] >= (unsigned short)(int_threshold & 0xFFFF)) {
            cond_sum_f += (float)arr_us[i];
        }
        
        /* LE_EXPR conditional count */
        if (arr_us[i] <= (unsigned short)((int_threshold * 2) & 0xFFFF)) {
            count_le++;
        }
    }
    
    /* ===== Loop 6: Complex nested conditions ===== */
    /* Float array with deeply nested conditions */
    float complex_max_f = -1e30f;
    float complex_min_f = 1e30f;
    
    for (int i = 0; i < 64; i++) {
        /* Outer condition */
        if (i % 3 != 0) {
            /* Middle condition */
            if (arr_f[i] > 0.0f) {
                /* Inner GT_EXPR */
                if (arr_f[i] > complex_max_f) {
                    complex_max_f = arr_f[i];
                }
                
                /* Inner LT_EXPR in same nested block */
                if (arr_f[i] < complex_min_f) {
                    complex_min_f = arr_f[i];
                }
            }
        }
    }
    
    /* Prevent optimization */
    global_sink = max_val_i + min_val_i + max_val_us + min_val_us + 
                  (int)cond_sum_f + (int)cond_sum_d + count_gt + count_le +
                  (int)complex_max_f + (int)complex_min_f;
    
    /* Calculate checksum */
    int checksum = max_val_i;
    checksum += min_val_i;
    checksum += max_val_us;
    checksum += min_val_us;
    checksum += (int)cond_sum_f;
    checksum += (int)cond_sum_d;
    checksum += count_gt;
    checksum += count_le;
    checksum += (int)complex_max_f;
    checksum += (int)complex_min_f;
    
    printf("%d\n", checksum);
    
    return 0;
}
