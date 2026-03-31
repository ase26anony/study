#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_result = 0;
volatile float g_float_result = 0.0f;

/* Function to create loop-invariant thresholds */
int get_threshold(int seed) {
    volatile int v = seed;
    return v % 100 + 50; /* Make it non-constant but predictable */
}

float get_float_threshold(int seed) {
    volatile float v = (float)(seed % 100);
    return v * 0.5f + 25.0f;
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
        arr_i[i] = (i * 3 + seed) % 1000;
        arr_us[i] = (unsigned short)((i * 5 + seed * 2) % 65535);
        arr_f[i] = (float)((i * 7 + seed * 3) % 1000) * 0.1f;
        arr_d[i] = (double)((i * 11 + seed * 5) % 1000) * 0.05;
    }
    
    /* Loop-invariant thresholds from volatile sources */
    int thresh_i = get_threshold(seed);
    unsigned short thresh_us = (unsigned short)get_threshold(seed * 2);
    float thresh_f = get_float_threshold(seed);
    double thresh_d = get_float_threshold(seed * 3) * 2.0;
    
    /* Reduction variables */
    int max_val_i = arr_i[0];  /* For GT_EXPR pattern */
    int min_val_i = arr_i[0];  /* For LT_EXPR pattern */
    float sum_ge_f = 0.0f;     /* For GE_EXPR pattern */
    double sum_le_d = 0.0;     /* For LE_EXPR pattern */
    unsigned short max_us = arr_us[0]; /* Mixed with other conditions */
    int count_gt = 0;          /* Counter for > comparisons */
    
    /* ===== Loop 1: GT_EXPR pattern (greater-than) ===== */
    /* Finding maximum with conditional update: if (arr_i[i] > max_val_i) */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (seed > 0) {
            /* The key GT_EXPR comparison for conditional reduction */
            if (arr_i[i] > max_val_i) {
                max_val_i = arr_i[i];
            }
            
            /* Additional operation to prevent trivial loop body */
            count_gt += (arr_i[i] > thresh_i) ? 1 : 0;
        }
    }
    
    /* ===== Loop 2: GE_EXPR pattern (greater-than-or-equal) ===== */
    /* Conditional sum: sum += (arr_f[i] >= threshold) ? arr_f[i] : 0 */
    /* Using while loop variant */
    int idx = 0;
    while (idx < 64) {
        /* Combine with logical OR to test pattern matching */
        if (idx % 2 == 0 || arr_f[idx] >= thresh_f) {
            sum_ge_f += (arr_f[idx] >= thresh_f) ? arr_f[idx] : 0.0f;
        }
        idx++;
    }
    
    /* ===== Loop 3: LT_EXPR pattern (less-than) ===== */
    /* Finding minimum with conditional update: if (arr_i[i] < min_val_i) */
    /* Multiple reductions in one loop */
    int temp_min = min_val_i;
    int temp_max = max_val_i; /* Reuse previous max as starting point */
    for (int i = 0; i < 64; i++) {
        /* Nested conditional structure */
        if (arr_i[i] % 2 == 0) {
            /* LT_EXPR for minimum finding */
            if (arr_i[i] < temp_min) {
                temp_min = arr_i[i];
            }
        } else {
            /* Additional GT_EXPR in same loop */
            if (arr_i[i] > temp_max) {
                temp_max = arr_i[i];
            }
        }
        
        /* Another conditional reduction with different type */
        if (arr_us[i] < thresh_us && arr_us[i] > max_us) {
            max_us = arr_us[i];
        }
    }
    min_val_i = temp_min;
    max_val_i = temp_max; /* Update outer variable */
    
    /* ===== Loop 4: LE_EXPR pattern (less-than-or-equal) ===== */
    /* Conditional sum with <= comparison */
    /* Multiple conditions combined with logical AND */
    for (int i = 0; i < 64; i++) {
        if (i > 0 && arr_d[i] <= thresh_d) {
            sum_le_d += arr_d[i];
        }
        
        /* Additional reduction with different comparison in same loop */
        if (arr_d[i] >= thresh_d * 0.5) {
            sum_ge_f += (float)arr_d[i]; /* Mix float and double */
        }
    }
    
    /* ===== Loop 5: Mixed reductions with all comparison types ===== */
    /* This loop tests handling multiple concurrent conditional reductions */
    int mixed_max = arr_i[0];
    int mixed_min = arr_i[0];
    float mixed_sum_ge = 0.0f;
    double mixed_sum_le = 0.0;
    
    for (int i = 0; i < 64; i++) {
        /* GT_EXPR */
        if (arr_i[i] > mixed_max) {
            mixed_max = arr_i[i];
        }
        
        /* LT_EXPR */
        if (arr_i[i] < mixed_min) {
            mixed_min = arr_i[i];
        }
        
        /* GE_EXPR */
        if (arr_f[i % 32] >= thresh_f) {
            mixed_sum_ge += arr_f[i % 32];
        }
        
        /* LE_EXPR - using different array */
        if (arr_d[i] <= thresh_d) {
            mixed_sum_le += arr_d[i];
        }
    }
    
    /* Aggregate results into checksum */
    int checksum = 0;
    checksum += max_val_i;
    checksum += min_val_i;
    checksum += (int)sum_ge_f;
    checksum += (int)sum_le_d;
    checksum += max_us;
    checksum += count_gt;
    checksum += mixed_max;
    checksum += mixed_min;
    checksum += (int)mixed_sum_ge;
    checksum += (int)mixed_sum_le;
    
    /* Store to volatile to prevent dead code elimination */
    g_result = checksum;
    g_float_result = sum_ge_f + (float)sum_le_d;
    
    printf("Checksum: %d\n", checksum);
    printf("Float result: %f\n", g_float_result);
    
    return 0;
}
