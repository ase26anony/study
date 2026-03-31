#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_result = 0;
volatile float g_float_result = 0.0f;

/* Function to create loop-invariant thresholds */
int get_threshold(int seed) {
    volatile int t = seed % 100 + 50;
    return t;
}

float get_float_threshold(int seed) {
    volatile float t = (seed % 100) * 0.5f + 25.0f;
    return t;
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
        arr_i[i] = (i * 3 + seed) % 200 - 100;  /* Range: -100 to 99 */
        arr_us[i] = (i * 5 + seed) % 65535;
        arr_f[i] = (i * 7 + seed) % 100 * 0.3f - 15.0f;
        arr_d[i] = (i * 11 + seed) % 100 * 0.7 - 35.0;
    }
    
    /* Loop-invariant thresholds from volatile sources */
    int thresh_i = get_threshold(seed);
    unsigned short thresh_us = get_threshold(seed) % 65535;
    float thresh_f = get_float_threshold(seed);
    double thresh_d = get_float_threshold(seed) * 2.0;
    
    /* Reduction variables */
    int max_val = arr_i[0];
    int min_val = arr_i[0];
    int cond_sum_gt = 0;
    int cond_sum_ge = 0;
    float float_max = arr_f[0];
    float float_min = arr_f[0];
    double double_sum_lt = 0.0;
    unsigned short us_max = arr_us[0];
    int count_le = 0;
    
    /* Test 1: GT_EXPR (greater-than) with nested if */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (i % 2 == 0) {
            /* Conditional max reduction with > */
            if (arr_i[i] > max_val) {
                max_val = arr_i[i];
            }
            
            /* Conditional sum with > */
            if (arr_i[i] > thresh_i) {
                cond_sum_gt += arr_i[i];
            }
        }
    }
    
    /* Test 2: GE_EXPR (greater-than-or-equal) with logical AND */
    for (int i = 0; i < 64; i++) {
        /* Combined condition with && */
        if (arr_i[i] >= thresh_i && i < 60) {
            cond_sum_ge += arr_i[i];
        }
        
        /* Float conditional max with >= in same loop */
        if (arr_f[i] >= thresh_f && arr_f[i] > float_max) {
            float_max = arr_f[i];
        }
    }
    
    /* Test 3: LT_EXPR (less-than) with while loop */
    int j = 0;
    while (j < 64) {
        /* Conditional min reduction with < */
        if (arr_i[j] < min_val) {
            min_val = arr_i[j];
        }
        
        /* Double conditional sum with < */
        if (arr_d[j] < thresh_d) {
            double_sum_lt += arr_d[j];
        }
        
        /* Multiple reductions in one loop */
        if (arr_us[j] < thresh_us) {
            if (arr_us[j] < us_max) {
                us_max = arr_us[j];
            }
        }
        j++;
    }
    
    /* Test 4: LE_EXPR (less-than-or-equal) with complex condition */
    for (int i = 0; i < 64; i++) {
        /* Nested conditionals with || */
        if (i > 10 || i < 50) {
            if (arr_i[i] <= thresh_i) {
                count_le++;
                
                /* Another conditional in same block */
                if (arr_f[i] <= float_min) {
                    float_min = arr_f[i];
                }
            }
        }
    }
    
    /* Test 5: Mixed reductions in single loop with all operators */
    int mixed_max = arr_i[0];
    int mixed_min = arr_i[0];
    int mixed_sum_gt = 0;
    int mixed_sum_le = 0;
    
    for (int i = 0; i < 64; i++) {
        /* GT reduction */
        if (arr_i[i] > mixed_max) {
            mixed_max = arr_i[i];
        }
        
        /* LT reduction */
        if (arr_i[i] < mixed_min) {
            mixed_min = arr_i[i];
        }
        
        /* GE conditional sum */
        if (arr_i[i] >= thresh_i) {
            mixed_sum_gt += arr_i[i];
        }
        
        /* LE conditional sum */
        if (arr_i[i] <= thresh_i) {
            mixed_sum_le += arr_i[i];
        }
    }
    
    /* Aggregate results into checksum */
    uint64_t checksum = 0;
    checksum += max_val;
    checksum += min_val;
    checksum += cond_sum_gt;
    checksum += cond_sum_ge;
    checksum += (int)float_max;
    checksum += (int)float_min;
    checksum += (int)double_sum_lt;
    checksum += us_max;
    checksum += count_le;
    checksum += mixed_max;
    checksum += mixed_min;
    checksum += mixed_sum_gt;
    checksum += mixed_sum_le;
    
    /* Store to volatile to prevent elimination */
    g_result = (int)checksum;
    g_float_result = float_max + float_min;
    
    printf("Checksum: %lu\n", checksum);
    
    return 0;
}
