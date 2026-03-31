#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_sink;
volatile float g_volatile_float_sink;

/* Function to create loop-invariant thresholds */
int get_threshold(int seed) {
    volatile int v = seed;
    return v % 100 + 50;  /* Returns 50-149 */
}

float get_float_threshold(int seed) {
    volatile float v = seed * 0.7f;
    return v + 25.0f;  /* Loop-invariant float threshold */
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
        arr_i[i] = (i * 3 + seed) % 200;
        arr_us[i] = (unsigned short)((i * 5 + seed) % 300);
        arr_f[i] = (float)((i * 7 + seed) % 400) * 0.5f;
        arr_d[i] = (double)((i * 11 + seed) % 500) * 0.3;
    }
    
    /* Loop-invariant thresholds from volatile sources */
    int int_thresh = get_threshold(seed);
    float float_thresh = get_float_threshold(seed);
    unsigned short us_thresh = (unsigned short)(seed % 150 + 50);
    double double_thresh = (double)(seed % 200 + 100) * 0.4;
    
    /* Reduction variables */
    int max_val = -1000;
    int min_val = 1000;
    float cond_sum_f = 0.0f;
    double cond_sum_d = 0.0;
    unsigned short max_us = 0;
    int count_gt = 0;
    int count_le = 0;
    
    /* Prevent initialization optimization */
    volatile int init_max = max_val;
    volatile float init_sum = cond_sum_f;
    max_val = init_max;
    cond_sum_f = init_sum;
    
    /* ====== Loop 1: GT_EXPR (greater-than) conditional reductions ====== */
    /* Mixed reductions with GT_EXPR */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (i % 3 != 0) {
            /* GT_EXPR: if (arr_i[i] > max_val) max_val = arr_i[i] */
            if (arr_i[i] > max_val) {
                max_val = arr_i[i];
            }
            
            /* Another GT_EXPR with different type and logical AND */
            if (arr_i[i] > int_thresh && i % 2 == 0) {
                count_gt++;
            }
        }
    }
    
    /* ====== Loop 2: GE_EXPR (greater-than-or-equal) reductions ====== */
    /* While loop variant with GE_EXPR */
    int j = 0;
    while (j < 64) {
        /* GE_EXPR: sum values >= threshold */
        if (arr_f[j] >= float_thresh) {
            cond_sum_f += arr_f[j];
        }
        
        /* Nested conditional with GE_EXPR */
        if (j > 10) {
            if (arr_us[j] >= us_thresh) {
                if (arr_us[j] > max_us) {  /* Inner GT_EXPR */
                    max_us = arr_us[j];
                }
            }
        }
        j++;
    }
    
    /* ====== Loop 3: LT_EXPR (less-than) and LE_EXPR (less-than-or-equal) ====== */
    /* Multiple reductions in one loop with LT_EXPR and LE_EXPR */
    for (int i = 0; i < 64; i++) {
        /* LT_EXPR: if (arr_i[i] < min_val) min_val = arr_i[i] */
        if (arr_i[i] < min_val) {
            min_val = arr_i[i];
        }
        
        /* LE_EXPR: sum values <= threshold with logical OR */
        if (arr_d[i] <= double_thresh || i % 4 == 0) {
            cond_sum_d += arr_d[i];
        }
        
        /* Another LE_EXPR for counting */
        if (arr_i[i] <= int_thresh) {
            count_le++;
        }
    }
    
    /* ====== Loop 4: Mixed comparison operators in nested conditionals ====== */
    /* Complex loop with multiple conditions */
    float complex_sum = 0.0f;
    int complex_max = -1000;
    
    for (int i = 0; i < 64; i++) {
        /* Outer if with compound condition */
        if (i > 5 && i < 58) {
            /* GT_EXPR and LT_EXPR combined */
            if (arr_i[i] > (int_thresh / 2) && arr_i[i] < (int_thresh * 2)) {
                complex_sum += arr_f[i];
                
                /* GE_EXPR inside */
                if (arr_f[i] >= (float_thresh * 0.5f)) {
                    if (arr_i[i] > complex_max) {  /* Another GT_EXPR */
                        complex_max = arr_i[i];
                    }
                }
            }
            
            /* LE_EXPR with logical OR */
            if (arr_us[i] <= us_thresh || arr_i[i] % 3 == 0) {
                count_gt++;  /* Reuse counter */
            }
        }
    }
    
    /* ====== Loop 5: Float/double comparisons with all operators ====== */
    float float_max = -1e9f;
    float float_min = 1e9f;
    double double_max = -1e9;
    
    for (int i = 0; i < 64; i++) {
        /* GT_EXPR for floats */
        if (arr_f[i] > float_max) {
            float_max = arr_f[i];
        }
        
        /* LT_EXPR for floats */
        if (arr_f[i] < float_min) {
            float_min = arr_f[i];
        }
        
        /* GE_EXPR for doubles */
        if (arr_d[i] >= double_thresh) {
            if (arr_d[i] > double_max) {  /* Nested GT_EXPR */
                double_max = arr_d[i];
            }
        }
        
        /* LE_EXPR with logical AND */
        if (arr_d[i] <= (double_thresh * 1.5) && i % 3 != 0) {
            cond_sum_d += arr_d[i] * 0.5;
        }
    }
    
    /* Prevent dead code elimination */
    g_volatile_sink = max_val;
    g_volatile_float_sink = cond_sum_f;
    
    /* Compute checksum from all results */
    int checksum = 0;
    checksum += max_val;
    checksum += min_val;
    checksum += (int)cond_sum_f;
    checksum += (int)cond_sum_d;
    checksum += max_us;
    checksum += count_gt;
    checksum += count_le;
    checksum += (int)complex_sum;
    checksum += complex_max;
    checksum += (int)float_max;
    checksum += (int)float_min;
    checksum += (int)double_max;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
