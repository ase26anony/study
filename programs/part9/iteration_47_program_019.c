#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_result = 0;
volatile float g_float_result = 0.0f;

/* Function to create loop-invariant thresholds */
int get_threshold(int seed) {
    volatile int v = seed;
    return v % 100 + 50;  /* Make it loop-invariant but not constant */
}

float get_float_threshold(int seed) {
    volatile float v = (float)(seed % 100);
    return v + 25.5f;
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
        arr_i[i] = (i * 3 + seed) % 256;
        arr_us[i] = (unsigned short)((i * 5 + seed) % 65535);
        arr_f[i] = (float)((i * 7 + seed) % 1000) / 3.0f;
        arr_d[i] = (double)((i * 11 + seed) % 1000) / 5.0;
    }
    
    /* Loop-invariant thresholds */
    int thresh_i = get_threshold(seed);
    float thresh_f = get_float_threshold(seed);
    unsigned short thresh_us = (unsigned short)(seed % 200 + 100);
    double thresh_d = (double)(seed % 300 + 150) / 2.0;
    
    /* Reduction variables */
    int max_val = arr_i[0];
    int min_val = arr_i[0];
    unsigned short max_us = arr_us[0];
    float cond_sum_f = 0.0f;
    double cond_sum_d = 0.0;
    int count_gt = 0;
    int count_le = 0;
    
    /* ===== Loop 1: GT_EXPR (greater-than) conditional reductions ===== */
    /* Multiple reductions in one loop with outer conditional */
    if (seed > 0) {  /* Outer if to complicate control flow */
        for (int i = 0; i < 64; i++) {
            /* GT_EXPR: if (arr_i[i] > max_val) max_val = arr_i[i]; */
            if (arr_i[i] > max_val) {
                max_val = arr_i[i];
            }
            
            /* Another GT_EXPR with different type */
            if (arr_us[i] > max_us && i % 2 == 0) {  /* Combined with logical AND */
                max_us = arr_us[i];
            }
            
            /* Conditional sum with GT_EXPR */
            if (arr_f[i] > thresh_f) {
                cond_sum_f += arr_f[i];
                count_gt++;
            }
        }
    }
    
    /* ===== Loop 2: GE_EXPR (greater-than-or-equal) ===== */
    /* Using while loop instead of for */
    int j = 0;
    int sum_ge = 0;
    float min_f = arr_f[0];
    
    while (j < 64) {
        /* GE_EXPR: if (arr_f[j] >= min_f) min_f = arr_f[j]; */
        /* Actually for min we need opposite, so we'll do max with >= */
        if (arr_f[j] >= min_f || j == 0) {  /* Using logical OR */
            min_f = arr_f[j];  /* This finds max actually, but tests GE_EXPR */
        }
        
        /* Conditional count with GE_EXPR */
        if (arr_i[j] >= thresh_i && arr_i[j] < thresh_i * 2) {
            sum_ge += arr_i[j];
        }
        
        j++;
    }
    
    /* ===== Loop 3: LT_EXPR (less-than) ===== */
    /* Mixed data types in same loop */
    double min_d = arr_d[0];
    int sum_lt = 0;
    
    for (int i = 0; i < 64; i++) {
        /* LT_EXPR: if (arr_d[i] < min_d) min_d = arr_d[i]; */
        if (arr_d[i] < min_d) {
            min_d = arr_d[i];
        }
        
        /* Another LT_EXPR with integer */
        if (arr_i[i] < thresh_i) {
            sum_lt += arr_i[i];
        }
        
        /* Nested conditional with LT_EXPR */
        if (i > 10 && i < 50) {
            if (arr_us[i] < thresh_us) {
                count_le++;
            }
        }
    }
    
    /* ===== Loop 4: LE_EXPR (less-than-or-equal) ===== */
    /* Complex loop with multiple LE_EXPR conditions */
    int last_val = arr_i[0];
    double sum_le_d = 0.0;
    
    for (int i = 0; i < 64; i++) {
        /* LE_EXPR: if (arr_i[i] <= min_val) min_val = arr_i[i]; */
        /* Actually min_val already holds min, so update it */
        if (arr_i[i] <= min_val) {
            min_val = arr_i[i];
        }
        
        /* LE_EXPR with float and logical combination */
        if (arr_f[i] <= thresh_f && arr_f[i] > 0) {
            sum_le_d += (double)arr_f[i];
        }
        
        /* Another LE_EXPR in same loop */
        if (arr_i[i] <= last_val || i % 3 == 0) {
            last_val = arr_i[i];
        }
    }
    
    /* ===== Loop 5: Mixed comparisons in single loop ===== */
    /* Tests vectorizer's ability to handle multiple different comparisons */
    int mixed_sum = 0;
    float mixed_max_f = arr_f[0];
    double mixed_min_d = arr_d[0];
    
    for (int i = 0; i < 64; i++) {
        /* GT_EXPR */
        if (arr_i[i] > thresh_i) {
            mixed_sum += 1;
        }
        
        /* GE_EXPR */
        if (arr_f[i] >= thresh_f) {
            if (arr_f[i] > mixed_max_f) {  /* Nested GT_EXPR */
                mixed_max_f = arr_f[i];
            }
        }
        
        /* LT_EXPR */
        if (arr_d[i] < thresh_d) {
            mixed_sum += 2;
        }
        
        /* LE_EXPR */
        if (arr_us[i] <= thresh_us) {
            mixed_sum += 3;
        }
    }
    
    /* Aggregate results into checksum */
    int checksum = 0;
    checksum += max_val;
    checksum += min_val;
    checksum += max_us;
    checksum += (int)cond_sum_f;
    checksum += sum_ge;
    checksum += (int)min_f;
    checksum += sum_lt;
    checksum += (int)min_d;
    checksum += (int)sum_le_d;
    checksum += mixed_sum;
    checksum += (int)mixed_max_f;
    checksum += (int)mixed_min_d;
    checksum += count_gt;
    checksum += count_le;
    
    /* Store to volatile to prevent dead code elimination */
    g_result = checksum;
    g_float_result = cond_sum_f;
    
    printf("Checksum: %d\n", checksum);
    printf("Max int: %d, Min int: %d\n", max_val, min_val);
    printf("Max ushort: %u\n", max_us);
    printf("Float sum above threshold: %.2f\n", cond_sum_f);
    
    return 0;
}
