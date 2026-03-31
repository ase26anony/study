#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_result = 0;
volatile float g_float_result = 0.0f;

/* Function to generate deterministic data */
int generate_value(int i, int seed) {
    return (i * 3 + seed) % 100;
}

float generate_float(int i, int seed) {
    return (float)((i * 7 + seed * 3) % 100) / 2.0f;
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
    
    /* Initialize arrays */
    for (int i = 0; i < 64; i++) {
        arr_i[i] = generate_value(i, seed);
        arr_us[i] = (unsigned short)(generate_value(i, seed + 1) & 0xFFFF);
        arr_f[i] = generate_float(i, seed);
        arr_d[i] = (double)generate_float(i, seed + 2);
    }
    
    /* Loop-invariant thresholds from volatile sources */
    volatile int vi_thresh = seed + 10;
    volatile float vf_thresh = (float)(seed % 50) / 2.0f + 10.0f;
    volatile double vd_thresh = (double)(seed % 40) / 3.0 + 15.0;
    
    int thresh_i = vi_thresh;
    float thresh_f = vf_thresh;
    double thresh_d = vd_thresh;
    
    /* Reduction variables */
    int max_val_i = arr_i[0];
    int min_val_i = arr_i[0];
    unsigned short max_val_us = arr_us[0];
    float cond_sum_f = 0.0f;
    double cond_sum_d = 0.0;
    int count_gt = 0;
    int count_le = 0;
    
    /* Loop 1: GT_EXPR (>) conditional reduction with nested if */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (arr_i[i] > 0) {
            /* GT_EXPR pattern: if (arr_i[i] > max_val_i) max_val_i = arr_i[i]; */
            if (arr_i[i] > max_val_i) {
                max_val_i = arr_i[i];
            }
            
            /* Combined with another reduction using logical AND */
            if (arr_i[i] > thresh_i && arr_i[i] < 100) {
                count_gt++;
            }
        }
    }
    
    /* Loop 2: GE_EXPR (>=) conditional reduction with while loop */
    int j = 0;
    int limit = 64;
    while (j < limit) {
        /* GE_EXPR pattern: sum values >= threshold */
        if (arr_f[j] >= thresh_f) {
            cond_sum_f += arr_f[j];
        }
        
        /* Multiple reductions in one loop */
        if (arr_us[j] >= (unsigned short)thresh_i) {
            /* Find max with >= comparison */
            if (arr_us[j] >= max_val_us) {
                max_val_us = arr_us[j];
            }
        }
        j++;
    }
    
    /* Loop 3: LT_EXPR (<) and LE_EXPR (<=) combined reductions */
    int min_val_lt = arr_i[0];
    int sum_le = 0;
    
    for (int i = 0; i < 64; i++) {
        /* LT_EXPR pattern: if (arr_i[i] < min_val_lt) min_val_lt = arr_i[i]; */
        if (arr_i[i] < min_val_lt) {
            min_val_lt = arr_i[i];
        }
        
        /* LE_EXPR pattern: sum values <= threshold */
        if (arr_i[i] <= thresh_i) {
            sum_le += arr_i[i];
            count_le++;
        }
        
        /* Additional conditional with logical OR */
        if (arr_i[i] < thresh_i || arr_i[i] > thresh_i + 20) {
            /* Do something to complicate the pattern */
            min_val_i = (arr_i[i] < min_val_i) ? arr_i[i] : min_val_i;
        }
    }
    
    /* Loop 4: Mixed comparisons on double array */
    double max_d = arr_d[0];
    double min_d = arr_d[0];
    
    for (int i = 0; i < 64; i++) {
        /* GT_EXPR on doubles */
        if (arr_d[i] > max_d) {
            max_d = arr_d[i];
        }
        
        /* LT_EXPR on doubles */
        if (arr_d[i] < min_d) {
            min_d = arr_d[i];
        }
        
        /* GE_EXPR conditional sum */
        if (arr_d[i] >= thresh_d) {
            cond_sum_d += arr_d[i];
        }
    }
    
    /* Loop 5: Complex nested conditionals with multiple reductions */
    int complex_sum = 0;
    int complex_max = arr_i[0];
    int complex_min = arr_i[0];
    
    for (int i = 0; i < 64; i++) {
        /* Outer if with multiple conditions */
        if (arr_i[i] > 10 && arr_i[i] < 90) {
            /* Inner comparisons for different operators */
            if (arr_i[i] > complex_max) {
                complex_max = arr_i[i];
            }
            
            if (arr_i[i] < complex_min) {
                complex_min = arr_i[i];
            }
            
            /* Conditional sum with >= */
            if (arr_i[i] >= thresh_i) {
                complex_sum += arr_i[i];
            }
        }
    }
    
    /* Aggregate results into checksum */
    int checksum = 0;
    checksum += max_val_i;
    checksum += min_val_i;
    checksum += max_val_us;
    checksum += (int)cond_sum_f;
    checksum += (int)cond_sum_d;
    checksum += count_gt;
    checksum += count_le;
    checksum += min_val_lt;
    checksum += sum_le;
    checksum += (int)max_d;
    checksum += (int)min_d;
    checksum += complex_sum;
    checksum += complex_max;
    checksum += complex_min;
    
    /* Store to volatile to prevent dead code elimination */
    g_result = checksum;
    g_float_result = cond_sum_f;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
