#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent dead code elimination */
volatile int g_volatile_sink;
volatile float g_volatile_float_sink;

/* Function to generate deterministic but non-constant data */
static int gen_value(int i, int seed) {
    return (i * 3 + seed) % 100;
}

static float gen_float(int i, int seed) {
    return (float)((i * 7 + seed * 3) % 1000) / 10.0f;
}

static double gen_double(int i, int seed) {
    return (double)((i * 11 + seed * 5) % 2000) / 20.0;
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
        arr_us[i] = (unsigned short)(gen_value(i, seed + 1) & 0xFFFF);
        arr_f[i] = gen_float(i, seed);
        arr_d[i] = gen_double(i, seed);
    }
    
    /* Loop-invariant thresholds from volatile sources */
    volatile int volatile_threshold = seed + 25;
    volatile float volatile_fthreshold = (float)(seed + 50) / 2.0f;
    volatile double volatile_dthreshold = (double)(seed + 75) / 3.0;
    
    int threshold_i = volatile_threshold;
    float threshold_f = volatile_fthreshold;
    double threshold_d = volatile_dthreshold;
    
    /* Reduction variables */
    int max_val_i = arr_i[0];
    int min_val_i = arr_i[0];
    unsigned short max_val_us = arr_us[0];
    float cond_sum_f = 0.0f;
    double cond_sum_d = 0.0;
    int count_gt = 0;
    int count_le = 0;
    int sum_ge = 0;
    int sum_lt = 0;
    
    /* Test 1: GT_EXPR (greater-than) with integer array */
    /* Pattern: if (arr_i[i] > current_max) current_max = arr_i[i]; */
    for (int i = 1; i < 64; i++) {
        if (arr_i[i] > max_val_i) {
            max_val_i = arr_i[i];
        }
    }
    
    /* Test 2: GE_EXPR (greater-than-or-equal) with mixed conditions */
    /* Pattern: sum += (arr_i[i] >= threshold) ? arr_i[i] : 0; */
    for (int i = 0; i < 64; i++) {
        /* Nested conditional to complicate control flow */
        if (i % 2 == 0) {
            if (arr_i[i] >= threshold_i) {
                sum_ge += arr_i[i];
            }
        } else {
            /* Logical OR to further complicate */
            if (arr_i[i] >= threshold_i || arr_i[i] % 3 == 0) {
                sum_ge += arr_i[i] / 2;
            }
        }
    }
    
    /* Test 3: LT_EXPR (less-than) with while loop */
    /* Pattern: if (arr_us[i] < current_min) current_min = arr_us[i]; */
    int j = 0;
    while (j < 64) {
        if (arr_us[j] < min_val_i) {  /* Note: comparing different types */
            min_val_i = arr_us[j];
        }
        j++;
    }
    
    /* Test 4: LE_EXPR (less-than-or-equal) with multiple reductions */
    /* Single loop with multiple conditional reductions */
    for (int i = 0; i < 64; i++) {
        /* First reduction: count elements <= threshold */
        if (arr_i[i] <= threshold_i) {
            count_le++;
        }
        
        /* Second reduction: sum elements < threshold */
        if (arr_i[i] < threshold_i) {
            sum_lt += arr_i[i];
        }
        
        /* Third reduction: conditional sum with floating point */
        if (arr_f[i] <= threshold_f) {
            cond_sum_f += arr_f[i];
        }
    }
    
    /* Test 5: Mixed comparisons in one loop with floating point */
    float max_f = arr_f[0];
    float min_f = arr_f[0];
    for (int i = 1; i < 64; i++) {
        /* GT_EXPR for max */
        if (arr_f[i] > max_f) {
            max_f = arr_f[i];
        }
        
        /* LT_EXPR for min */
        if (arr_f[i] < min_f) {
            min_f = arr_f[i];
        }
        
        /* GE_EXPR conditional accumulation */
        if (arr_f[i] >= threshold_f) {
            count_gt++;
        }
    }
    
    /* Test 6: LE_EXPR with double precision */
    double min_d = arr_d[0];
    for (int i = 1; i < 64; i++) {
        if (arr_d[i] <= min_d) {
            min_d = arr_d[i];
        }
        
        /* Nested with logical AND */
        if (i > 10 && i < 50) {
            if (arr_d[i] <= threshold_d) {
                cond_sum_d += arr_d[i];
            }
        }
    }
    
    /* Test 7: Complex nested conditionals with GE_EXPR */
    int complex_sum = 0;
    for (int i = 0; i < 64; i++) {
        /* Outer if with logical OR */
        if (i % 3 == 0 || i % 5 == 0) {
            /* Inner if with GE comparison */
            if (arr_i[i] >= (threshold_i / 2)) {
                complex_sum += arr_i[i];
                
                /* Further nesting */
                if (arr_i[i] % 2 == 0) {
                    complex_sum += 1;
                }
            }
        }
    }
    
    /* Prevent optimization by using volatile sink */
    g_volatile_sink = max_val_i + min_val_i + sum_ge + sum_lt + count_gt + count_le + complex_sum;
    g_volatile_float_sink = max_f + min_f + cond_sum_f + (float)cond_sum_d;
    
    /* Compute checksum */
    int checksum = max_val_i;
    checksum += min_val_i;
    checksum += max_val_us;
    checksum += (int)cond_sum_f;
    checksum += (int)cond_sum_d;
    checksum += count_gt;
    checksum += count_le;
    checksum += sum_ge;
    checksum += sum_lt;
    checksum += (int)max_f;
    checksum += (int)min_f;
    checksum += (int)min_d;
    checksum += complex_sum;
    checksum += g_volatile_sink;
    checksum += (int)g_volatile_float_sink;
    
    printf("%d\n", checksum);
    return 0;
}
