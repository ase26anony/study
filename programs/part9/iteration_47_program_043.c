#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent dead code elimination */
volatile int g_result = 0;
volatile float g_float_result = 0.0f;

/* Function to create loop-invariant thresholds */
int get_threshold(int seed) {
    volatile int v = seed;
    return v % 100 + 50;  /* Returns 50-149 */
}

float get_float_threshold(int seed) {
    volatile float v = (float)(seed % 100);
    return v + 25.5f;  /* Returns 25.5-124.5 */
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
        arr_i[i] = (i * 3 + seed) % 200 - 100;      /* Values between -100 and 99 */
        arr_us[i] = (unsigned short)((i * 5 + seed) % 65535);
        arr_f[i] = (float)((i * 7 + seed) % 200 - 100) * 1.5f;
        arr_d[i] = (double)((i * 11 + seed) % 200 - 100) * 2.5;
    }
    
    /* Loop-invariant thresholds from volatile sources */
    int int_thresh = get_threshold(seed);
    float float_thresh = get_float_threshold(seed);
    unsigned short us_thresh = (unsigned short)(seed % 1000 + 1000);
    double double_thresh = (double)(seed % 200) * 0.8;
    
    /* Reduction variables */
    int max_val = arr_i[0];
    int min_val = arr_i[0];
    int cond_sum_gt = 0;
    int cond_sum_ge = 0;
    unsigned short max_us = arr_us[0];
    float max_float = arr_f[0];
    float min_float = arr_f[0];
    double sum_double_lt = 0.0;
    int count_le = 0;
    
    /* ===== Loop 1: GT_EXPR (greater-than) with multiple reductions ===== */
    /* This should trigger: case GT_EXPR: bitop1 = BIT_NOT_EXPR; bitop2 = BIT_AND_EXPR; */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (i % 2 == 0) {
            /* Find max with > comparison */
            if (arr_i[i] > max_val) {
                max_val = arr_i[i];
            }
            
            /* Conditional sum with > comparison */
            if (arr_i[i] > int_thresh) {
                cond_sum_gt += arr_i[i];
            }
            
            /* Combined condition with logical AND */
            if (arr_i[i] > (int_thresh - 20) && arr_i[i] < (int_thresh + 20)) {
                /* Additional operation inside */
                max_val = (arr_i[i] > max_val) ? arr_i[i] : max_val;
            }
        }
    }
    
    /* ===== Loop 2: GE_EXPR (greater-than-or-equal) ===== */
    /* This should trigger: case GE_EXPR: bitop1 = BIT_NOT_EXPR; bitop2 = BIT_IOR_EXPR; */
    for (int i = 0; i < 64; i++) {
        /* Conditional sum with >= comparison */
        if (arr_i[i] >= int_thresh) {
            cond_sum_ge += arr_i[i];
        }
        
        /* Find max of unsigned short with >= in while-style loop */
        int j = i;  /* Use while for variation */
        while (j == i) {  /* Single iteration while */
            if (arr_us[i] >= us_thresh && arr_us[i] > max_us) {
                max_us = arr_us[i];
            }
            j++;
        }
    }
    
    /* ===== Loop 3: LT_EXPR (less-than) ===== */
    /* This should trigger: case LT_EXPR with std::swap */
    /* Use a while loop for variation */
    int k = 0;
    while (k < 64) {
        /* Find min with < comparison */
        if (arr_f[k] < min_float) {
            min_float = arr_f[k];
        }
        
        /* Nested conditional with logical OR */
        if (k < 32 || arr_f[k] < float_thresh) {
            if (arr_f[k] < (float_thresh - 10.0f)) {
                min_float = (arr_f[k] < min_float) ? arr_f[k] : min_float;
            }
        }
        k++;
    }
    
    /* ===== Loop 4: LE_EXPR (less-than-or-equal) ===== */
    /* This should trigger: case LE_EXPR with std::swap */
    for (int i = 0; i < 64; i++) {
        /* Count elements <= threshold */
        if (arr_d[i] <= double_thresh) {
            count_le++;
        }
        
        /* Sum values < threshold (mixed with <= in same loop) */
        if (arr_d[i] < double_thresh) {
            sum_double_lt += arr_d[i];
        }
        
        /* Find max float with <= comparison in same loop */
        if (arr_f[i] <= max_float) {
            /* This is actually checking for min, so invert logic */
            if (arr_f[i] < max_float) {
                max_float = arr_f[i];  /* Actually finding min here */
            }
        }
    }
    
    /* ===== Loop 5: Mixed comparisons in single loop ===== */
    /* Multiple reduction variables with different comparisons */
    int sum_gt_mixed = 0;
    int sum_lt_mixed = 0;
    int count_ge_mixed = 0;
    int count_le_mixed = 0;
    
    for (int i = 0; i < 64; i++) {
        /* All 4 comparison types in one loop */
        if (arr_i[i] > int_thresh) {
            sum_gt_mixed += arr_i[i];
        }
        
        if (arr_i[i] >= int_thresh - 10) {
            count_ge_mixed++;
        }
        
        if (arr_i[i] < int_thresh + 10) {
            sum_lt_mixed += arr_i[i];
        }
        
        if (arr_i[i] <= int_thresh + 20) {
            count_le_mixed++;
        }
    }
    
    /* Aggregate results into checksum */
    int checksum = 0;
    checksum += max_val;
    checksum += min_val;
    checksum += cond_sum_gt;
    checksum += cond_sum_ge;
    checksum += (int)max_us;
    checksum += (int)max_float;
    checksum += (int)min_float;
    checksum += (int)sum_double_lt;
    checksum += count_le;
    checksum += sum_gt_mixed;
    checksum += sum_lt_mixed;
    checksum += count_ge_mixed;
    checksum += count_le_mixed;
    
    /* Store to volatile to prevent elimination */
    g_result = checksum;
    g_float_result = max_float + min_float;
    
    printf("Checksum: %d\n", checksum);
    printf("Max int: %d, Min int: %d\n", max_val, min_val);
    printf("Cond sum GT: %d, Cond sum GE: %d\n", cond_sum_gt, cond_sum_ge);
    printf("Max unsigned short: %u\n", max_us);
    printf("Float range: [%f, %f]\n", min_float, max_float);
    printf("Double sum LT: %f, Count LE: %d\n", sum_double_lt, count_le);
    printf("Mixed: sum_gt=%d, sum_lt=%d, count_ge=%d, count_le=%d\n", 
           sum_gt_mixed, sum_lt_mixed, count_ge_mixed, count_le_mixed);
    
    return 0;
}
