#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_seed;
volatile int g_volatile_result;

/* Function to create loop-invariant thresholds */
int get_threshold(int base) {
    volatile int v = base;
    return v + (g_volatile_seed % 5);
}

float get_fthreshold(float base) {
    volatile float v = base;
    return v + (g_volatile_seed % 5) * 0.5f;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    g_volatile_seed = seed;
    
    /* Arrays with deterministic but non-constant values */
    int arr_i[64];
    unsigned short arr_us[64];
    float arr_f[64];
    double arr_d[64];
    
    /* Initialize arrays */
    for (int i = 0; i < 64; i++) {
        arr_i[i] = (i * 3 + seed) % 100;
        arr_us[i] = (i * 5 + seed) % 256;
        arr_f[i] = (i * 7 + seed) % 100 * 0.7f;
        arr_d[i] = (i * 11 + seed) % 100 * 0.9;
    }
    
    /* Loop-invariant thresholds from volatile sources */
    int thresh_i = get_threshold(50);
    unsigned short thresh_us = get_threshold(100) % 256;
    float thresh_f = get_fthreshold(35.0f);
    double thresh_d = get_fthreshold(45.0);
    
    /* Reduction variables */
    int max_val_i = arr_i[0];
    int min_val_i = arr_i[0];
    unsigned short max_val_us = arr_us[0];
    float cond_sum_f = 0.0f;
    double cond_sum_d = 0.0;
    int count_gt = 0;
    int count_le = 0;
    
    /* Example 1: GT_EXPR (> operator) with nested conditional */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (i % 3 == 0) {
            /* Conditional reduction: find max with > comparison */
            if (arr_i[i] > max_val_i) {
                max_val_i = arr_i[i];
            }
        } else if (i % 3 == 1) {
            /* Multiple reductions in same loop */
            if (arr_i[i] > thresh_i) {
                count_gt++;
            }
        }
    }
    
    /* Example 2: GE_EXPR (>= operator) with logical AND */
    for (int i = 0; i < 64; i++) {
        /* Combined condition with logical AND */
        if (arr_i[i] >= thresh_i && arr_i[i] < 90) {
            /* Conditional sum with >= comparison */
            cond_sum_f += arr_f[i];
        }
    }
    
    /* Example 3: LT_EXPR (< operator) in while loop */
    int j = 0;
    while (j < 64) {
        /* Conditional reduction: find min with < comparison */
        if (arr_i[j] < min_val_i) {
            min_val_i = arr_i[j];
        }
        j++;
    }
    
    /* Example 4: LE_EXPR (<= operator) with multiple reductions */
    int sum_short = 0;
    int count_short = 0;
    for (int i = 0; i < 64; i++) {
        /* Multiple reductions with different conditions */
        if (arr_us[i] <= thresh_us) {
            sum_short += arr_us[i];
            count_short++;
        }
        
        /* Additional reduction with different comparison */
        if (arr_f[i] <= thresh_f) {
            cond_sum_d += arr_d[i];
        }
    }
    
    /* Example 5: Mixed comparisons in single loop */
    int mixed_max = arr_i[0];
    int mixed_min = arr_i[0];
    int mixed_sum = 0;
    for (int i = 0; i < 64; i++) {
        /* GT_EXPR for max */
        if (arr_i[i] > mixed_max) {
            mixed_max = arr_i[i];
        }
        
        /* LT_EXPR for min */
        if (arr_i[i] < mixed_min) {
            mixed_min = arr_i[i];
        }
        
        /* GE_EXPR for conditional sum */
        if (arr_i[i] >= thresh_i) {
            mixed_sum += arr_i[i];
        }
    }
    
    /* Example 6: LE_EXPR with floating point */
    float float_min = arr_f[0];
    for (int i = 0; i < 64; i++) {
        /* LE_EXPR for floating point min */
        if (arr_f[i] <= float_min) {
            float_min = arr_f[i];
        }
    }
    
    /* Aggregate results into checksum */
    int checksum = max_val_i + min_val_i + max_val_us + (int)cond_sum_f + 
                   (int)cond_sum_d + count_gt + count_le + sum_short + 
                   count_short + mixed_max + mixed_min + mixed_sum + 
                   (int)float_min;
    
    /* Store to volatile to prevent dead code elimination */
    g_volatile_result = checksum;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
