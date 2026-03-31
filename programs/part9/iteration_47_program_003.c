#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile globals to prevent dead code elimination */
volatile int g_result_int = 0;
volatile float g_result_float = 0.0f;

/* Function to create loop-invariant thresholds */
int get_threshold(int seed) {
    volatile int v = seed;
    return v % 100 + 50;  /* Returns 50-149 based on seed */
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
    
    /* Arrays with different types */
    int arr_i[64];
    unsigned short arr_us[64];
    float arr_f[64];
    double arr_d[64];
    
    /* Initialize arrays with deterministic but non-constant values */
    for (int i = 0; i < 64; i++) {
        arr_i[i] = (i * 3 + seed) % 200;
        arr_us[i] = (unsigned short)((i * 5 + seed * 2) % 300);
        arr_f[i] = (float)((i * 7 + seed * 3) % 400) * 0.5f;
        arr_d[i] = (double)((i * 11 + seed * 5) % 500) * 0.3;
    }
    
    /* Loop-invariant thresholds from volatile sources */
    int int_thresh = get_threshold(seed);
    float float_thresh = get_float_threshold(seed);
    unsigned short us_thresh = (unsigned short)(seed % 150 + 50);
    double double_thresh = (double)(seed % 200 + 100) * 0.4;
    
    /* Reduction variables */
    int max_val = -1000;
    int min_val = 1000;
    float cond_sum_float = 0.0f;
    double cond_sum_double = 0.0;
    unsigned short max_ushort = 0;
    int count_gt = 0;
    int count_le = 0;
    
    /* TEST 1: GT_EXPR (greater-than) conditional reduction */
    /* Pattern: if (arr_i[i] > current_max) current_max = arr_i[i] */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (i % 3 == 0) {
            if (arr_i[i] > max_val) {
                max_val = arr_i[i];
            }
        }
    }
    
    /* TEST 2: GE_EXPR (greater-than-or-equal) conditional reduction */
    /* Pattern: sum += (arr_f[i] >= threshold) ? arr_f[i] : 0 */
    for (int i = 0; i < 64; i++) {
        /* Combined with logical OR in condition */
        if (i < 60 || arr_f[i] >= float_thresh) {
            cond_sum_float += (arr_f[i] >= float_thresh) ? arr_f[i] : 0.0f;
        }
    }
    
    /* TEST 3: LT_EXPR (less-than) conditional reduction */
    /* Pattern: if (arr_us[i] < current_min) current_min = arr_us[i] */
    /* Using while loop variant */
    int idx = 0;
    while (idx < 64) {
        /* Nested conditional with logical AND */
        if (idx > 0 && idx < 63) {
            if (arr_us[idx] < min_val) {
                min_val = (int)arr_us[idx];
            }
        }
        idx++;
    }
    
    /* TEST 4: LE_EXPR (less-than-or-equal) conditional reduction */
    /* Pattern: sum += (arr_d[i] <= threshold) ? arr_d[i] : 0 */
    for (int i = 0; i < 64; i++) {
        /* Multiple conditions combined */
        if (i % 4 == 0 || i % 5 == 0) {
            if (arr_d[i] <= double_thresh) {
                cond_sum_double += arr_d[i];
            }
        }
    }
    
    /* TEST 5: Multiple reductions in one loop with different comparison operators */
    /* This tests handling of multiple concurrent conditional reductions */
    int local_max = -1000;
    int local_min = 1000;
    int sum_gt = 0;
    int sum_le = 0;
    
    for (int i = 0; i < 64; i++) {
        /* GT_EXPR reduction for max */
        if (arr_i[i] > local_max) {
            local_max = arr_i[i];
        }
        
        /* LT_EXPR reduction for min */
        if (arr_i[i] < local_min) {
            local_min = arr_i[i];
        }
        
        /* GE_EXPR conditional sum */
        if (arr_i[i] >= int_thresh) {
            sum_gt += arr_i[i];
            count_gt++;
        }
        
        /* LE_EXPR conditional sum with different threshold */
        if (arr_i[i] <= (int_thresh + 20)) {
            sum_le += arr_i[i];
            count_le++;
        }
    }
    
    /* TEST 6: Mixed types with GT_EXPR and GE_EXPR */
    /* Using unsigned short array */
    for (int i = 0; i < 64; i++) {
        /* GT_EXPR for unsigned short */
        if (arr_us[i] > max_ushort) {
            max_ushort = arr_us[i];
        }
        
        /* GE_EXPR conditional count */
        if (arr_us[i] >= us_thresh) {
            /* Additional operation to make body non-trivial */
            max_ushort = (max_ushort > arr_us[i]) ? max_ushort : arr_us[i];
        }
    }
    
    /* Aggregate results into checksum */
    int checksum = 0;
    checksum += max_val;
    checksum += min_val;
    checksum += (int)cond_sum_float;
    checksum += (int)cond_sum_double;
    checksum += local_max;
    checksum += local_min;
    checksum += sum_gt;
    checksum += sum_le;
    checksum += (int)max_ushort;
    checksum += count_gt;
    checksum += count_le;
    
    /* Store to volatile to prevent elimination */
    g_result_int = checksum;
    g_result_float = cond_sum_float;
    
    printf("Checksum: %d\n", checksum);
    printf("Max: %d, Min: %d\n", max_val, min_val);
    printf("Float sum: %.2f, Double sum: %.2f\n", cond_sum_float, cond_sum_double);
    printf("Local max/min: %d/%d\n", local_max, local_min);
    printf("Counts: GT=%d, LE=%d\n", count_gt, count_le);
    
    return 0;
}
