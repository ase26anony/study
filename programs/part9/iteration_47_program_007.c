#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent dead code elimination */
volatile int g_volatile_sink;

/* Function to generate deterministic data */
static int gen_value(int i, int seed) {
    return (i * 3 + seed) ^ (seed >> 2);
}

static float gen_float(int i, int seed) {
    return (float)((i * 7 + seed * 3) % 100) / 10.0f;
}

static double gen_double(int i, int seed) {
    return (double)((i * 11 + seed * 5) % 200) / 20.0;
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
        arr_us[i] = (unsigned short)(gen_value(i, seed) & 0xFFFF);
        arr_f[i] = gen_float(i, seed);
        arr_d[i] = gen_double(i, seed);
    }
    
    /* Loop-invariant thresholds from volatile source */
    volatile int vol_thresh = seed + 10;
    volatile float vol_fthresh = (float)(seed % 50) / 2.0f;
    volatile double vol_dthresh = (double)(seed % 100) / 4.0;
    
    int thresh_i = vol_thresh;
    float thresh_f = vol_fthresh;
    double thresh_d = vol_dthresh;
    
    /* Reduction variables */
    int max_val_i = arr_i[0];
    int min_val_i = arr_i[0];
    unsigned short max_val_us = arr_us[0];
    unsigned short min_val_us = arr_us[0];
    float max_val_f = arr_f[0];
    float min_val_f = arr_f[0];
    double max_val_d = arr_d[0];
    double min_val_d = arr_d[0];
    
    int cond_sum_i = 0;
    int cond_sum_us = 0;
    float cond_sum_f = 0.0f;
    double cond_sum_d = 0.0;
    int count_gt = 0, count_lt = 0;
    
    /* ===== Loop 1: GT_EXPR (greater-than) conditional reductions ===== */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (i % 3 != 0) {
            /* Multiple reductions with GT_EXPR */
            if (arr_i[i] > max_val_i) {
                max_val_i = arr_i[i];
            }
            
            /* Combined with logical AND */
            if (arr_i[i] > thresh_i && arr_i[i] % 2 == 0) {
                cond_sum_i += arr_i[i];
            }
            
            /* Floating-point GT_EXPR */
            if (arr_f[i] > max_val_f) {
                max_val_f = arr_f[i];
            }
            
            /* Count values greater than threshold */
            if (arr_f[i] > thresh_f) {
                count_gt++;
            }
        }
    }
    
    /* ===== Loop 2: GE_EXPR (greater-than-or-equal) conditional reductions ===== */
    int i = 0;
    while (i < 64) {
        /* Nested conditionals with GE_EXPR */
        if (i > 16) {
            if (arr_us[i] >= max_val_us) {
                max_val_us = arr_us[i];
            }
            
            /* Conditional sum with GE_EXPR */
            if (arr_us[i] >= (unsigned short)thresh_i) {
                cond_sum_us += arr_us[i];
            }
        }
        
        /* Double type with GE_EXPR */
        if (arr_d[i] >= max_val_d) {
            max_val_d = arr_d[i];
        }
        
        i++;
    }
    
    /* ===== Loop 3: LT_EXPR (less-than) conditional reductions ===== */
    /* Reset min values to first element */
    min_val_i = arr_i[0];
    min_val_us = arr_us[0];
    min_val_f = arr_f[0];
    min_val_d = arr_d[0];
    
    for (int i = 0; i < 64; i++) {
        /* Multiple reductions with LT_EXPR in one loop */
        if (arr_i[i] < min_val_i) {
            min_val_i = arr_i[i];
        }
        
        /* Combined with logical OR */
        if (arr_i[i] < thresh_i || arr_i[i] < 0) {
            cond_sum_i -= arr_i[i];
        }
        
        /* Floating-point LT_EXPR */
        if (arr_f[i] < min_val_f) {
            min_val_f = arr_f[i];
        }
        
        /* Count values less than threshold */
        if (arr_f[i] < thresh_f) {
            count_lt++;
        }
    }
    
    /* ===== Loop 4: LE_EXPR (less-than-or-equal) conditional reductions ===== */
    for (int i = 0; i < 64; i++) {
        /* Outer if with LE_EXPR inside */
        if (i % 4 == 0) {
            if (arr_us[i] <= min_val_us) {
                min_val_us = arr_us[i];
            }
            
            /* Conditional accumulation with LE_EXPR */
            if (arr_us[i] <= (unsigned short)(thresh_i * 2)) {
                cond_sum_us -= arr_us[i];
            }
        }
        
        /* Double type with LE_EXPR */
        if (arr_d[i] <= min_val_d) {
            min_val_d = arr_d[i];
        }
        
        /* Mixed conditional sum with LE_EXPR */
        if (arr_d[i] <= thresh_d) {
            cond_sum_d += arr_d[i];
        }
    }
    
    /* ===== Loop 5: Multiple reductions with different comparison operators ===== */
    int sum_gt = 0, sum_le = 0;
    float fsum_gt = 0.0f, fsum_lt = 0.0f;
    
    for (int i = 0; i < 64; i++) {
        /* Four different conditional reductions in one loop */
        if (arr_i[i] > thresh_i) {
            sum_gt += arr_i[i];
        }
        
        if (arr_i[i] <= thresh_i) {
            sum_le += arr_i[i];
        }
        
        if (arr_f[i] > thresh_f) {
            fsum_gt += arr_f[i];
        }
        
        if (arr_f[i] < thresh_f) {
            fsum_lt += arr_f[i];
        }
    }
    
    /* Prevent optimization */
    g_volatile_sink = max_val_i;
    
    /* Compute checksum from all results */
    int checksum = 0;
    checksum += max_val_i;
    checksum += min_val_i;
    checksum += (int)max_val_us;
    checksum += (int)min_val_us;
    checksum += (int)max_val_f;
    checksum += (int)min_val_f;
    checksum += (int)max_val_d;
    checksum += (int)min_val_d;
    checksum += cond_sum_i;
    checksum += cond_sum_us;
    checksum += (int)cond_sum_f;
    checksum += (int)cond_sum_d;
    checksum += count_gt;
    checksum += count_lt;
    checksum += sum_gt;
    checksum += sum_le;
    checksum += (int)fsum_gt;
    checksum += (int)fsum_lt;
    
    /* Print result to prevent elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
