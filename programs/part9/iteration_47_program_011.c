#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile globals to prevent dead code elimination */
volatile int global_sink_int;
volatile float global_sink_float;

/* Function to create loop-invariant thresholds */
int get_threshold(int seed, int offset) {
    volatile int v = seed + offset;
    return v;
}

float get_fthreshold(int seed, int offset) {
    volatile float v = (float)(seed + offset) * 0.5f;
    return v;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    
    /* Arrays with deterministic but non-constant values */
    int arr_i[64];
    unsigned short arr_us[64];
    float arr_f[64];
    double arr_d[64];
    
    /* Initialize arrays */
    for (int i = 0; i < 64; i++) {
        arr_i[i] = (i * 3 + seed) % 1000;
        arr_us[i] = (unsigned short)((i * 5 + seed) % 65535);
        arr_f[i] = (float)((i * 7 + seed) % 1000) * 0.1f;
        arr_d[i] = (double)((i * 11 + seed) % 1000) * 0.01;
    }
    
    /* Loop-invariant thresholds from volatile sources */
    int thresh_i = get_threshold(seed, 500);
    unsigned short thresh_us = (unsigned short)get_threshold(seed, 300);
    float thresh_f = get_fthreshold(seed, 200);
    double thresh_d = (double)get_fthreshold(seed, 100);
    
    /* Reduction variables */
    int max_val_i = arr_i[0];
    int min_val_i = arr_i[0];
    unsigned short max_val_us = arr_us[0];
    float cond_sum_f = 0.0f;
    double cond_sum_d = 0.0;
    int count_gt = 0;
    int count_le = 0;
    
    /* ====== Loop 1: GT_EXPR and LT_EXPR with multiple reductions ====== */
    /* This should trigger GT_EXPR -> BIT_NOT_EXPR, BIT_AND_EXPR
       and LT_EXPR -> BIT_NOT_EXPR, BIT_AND_EXPR with swap */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (i % 2 == 0) {
            /* GT_EXPR conditional max reduction */
            if (arr_i[i] > max_val_i) {
                max_val_i = arr_i[i];
            }
            
            /* LT_EXPR conditional min reduction */
            if (arr_i[i] < min_val_i) {
                min_val_i = arr_i[i];
            }
        }
    }
    
    /* ====== Loop 2: GE_EXPR with logical AND ====== */
    /* This should trigger GE_EXPR -> BIT_NOT_EXPR, BIT_IOR_EXPR */
    float max_val_f = arr_f[0];
    for (int i = 0; i < 64; i++) {
        /* Combined condition with logical AND */
        if (arr_f[i] >= thresh_f && i < 60) {
            if (arr_f[i] > max_val_f) {
                max_val_f = arr_f[i];
            }
            cond_sum_f += arr_f[i];
            count_gt++;
        }
    }
    
    /* ====== Loop 3: LE_EXPR with while loop ====== */
    /* This should trigger LE_EXPR -> BIT_NOT_EXPR, BIT_IOR_EXPR with swap */
    int idx = 0;
    double min_val_d = arr_d[0];
    while (idx < 64) {
        /* Nested conditional for LE_EXPR */
        if (idx > 10) {
            if (arr_d[idx] <= thresh_d) {
                if (arr_d[idx] < min_val_d) {
                    min_val_d = arr_d[idx];
                }
                cond_sum_d += arr_d[idx];
                count_le++;
            }
        }
        idx++;
    }
    
    /* ====== Loop 4: Mixed comparisons on unsigned short ====== */
    /* Multiple reductions in one loop with different comparison types */
    unsigned short sum_us_ge = 0;
    unsigned short sum_us_lt = 0;
    int count_ge = 0;
    int count_lt = 0;
    
    for (int i = 0; i < 64; i++) {
        /* GE_EXPR reduction */
        if (arr_us[i] >= thresh_us) {
            sum_us_ge += arr_us[i];
            count_ge++;
        }
        
        /* LT_EXPR reduction (with outer if) */
        if (i % 3 == 0) {
            if (arr_us[i] < thresh_us) {
                sum_us_lt += arr_us[i];
                count_lt++;
            }
        }
    }
    
    /* ====== Loop 5: Complex nested conditionals with multiple operators ====== */
    int complex_sum = 0;
    int complex_max = arr_i[0];
    for (int i = 0; i < 64; i++) {
        /* Outer condition with logical OR */
        if (i % 4 == 0 || i % 5 == 0) {
            /* Inner GT_EXPR */
            if (arr_i[i] > thresh_i) {
                complex_sum += arr_i[i];
                
                /* Nested LE_EXPR */
                if (arr_i[i] <= (thresh_i + 100)) {
                    if (arr_i[i] > complex_max) {
                        complex_max = arr_i[i];
                    }
                }
            }
        }
    }
    
    /* Prevent optimization */
    global_sink_int = max_val_i;
    global_sink_float = max_val_f;
    
    /* Compute checksum */
    int checksum = 0;
    checksum += max_val_i;
    checksum += min_val_i;
    checksum += max_val_us;
    checksum += (int)max_val_f;
    checksum += (int)min_val_d;
    checksum += (int)cond_sum_f;
    checksum += (int)cond_sum_d;
    checksum += sum_us_ge;
    checksum += sum_us_lt;
    checksum += count_gt;
    checksum += count_le;
    checksum += count_ge;
    checksum += count_lt;
    checksum += complex_sum;
    checksum += complex_max;
    
    /* Normalize checksum */
    checksum = checksum % 1000000;
    
    printf("%d\n", checksum);
    return 0;
}
