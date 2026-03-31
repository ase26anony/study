#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile globals to prevent dead code elimination */
volatile int g_result_int = 0;
volatile float g_result_float = 0.0f;
volatile double g_result_double = 0.0;

/* Function to create loop-invariant thresholds */
int get_threshold(int seed) {
    volatile int v = seed;
    return v % 100 + 50;  /* Returns 50-149 */
}

float get_float_threshold(int seed) {
    volatile float v = seed * 0.7f;
    return v;
}

double get_double_threshold(int seed) {
    volatile double v = seed * 1.3;
    return v;
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
        arr_us[i] = (unsigned short)((i * 5 + seed * 2) % 65535);
        arr_f[i] = (float)((i * 7 + seed * 3) % 1000) * 0.1f;
        arr_d[i] = (double)((i * 11 + seed * 5) % 2000) * 0.05;
    }
    
    /* Loop-invariant thresholds from volatile sources */
    int int_thresh = get_threshold(seed);
    float float_thresh = get_float_threshold(seed);
    double double_thresh = get_double_threshold(seed);
    
    /* Reduction variables */
    int max_int = arr_i[0];
    int min_int = arr_i[0];
    unsigned short max_ushort = arr_us[0];
    unsigned short min_ushort = arr_us[0];
    float max_float = arr_f[0];
    float min_float = arr_f[0];
    double max_double = arr_d[0];
    double min_double = arr_d[0];
    int cond_sum_int = 0;
    float cond_sum_float = 0.0f;
    double cond_sum_double = 0.0;
    int count_above = 0;
    int count_below = 0;
    
    /* Test 1: GT_EXPR (> operator) with mixed types */
    printf("Test 1: GT_EXPR patterns\n");
    for (int i = 0; i < 64; i++) {
        /* Integer max reduction with > */
        if (arr_i[i] > max_int) {
            max_int = arr_i[i];
        }
        
        /* Float conditional sum with > */
        if (arr_f[i] > float_thresh) {
            cond_sum_float += arr_f[i];
        }
        
        /* Nested conditional with logical AND */
        if (i > 10 && arr_i[i] > int_thresh) {
            count_above++;
        }
    }
    
    /* Test 2: GE_EXPR (>= operator) with while loop */
    printf("Test 2: GE_EXPR patterns\n");
    int j = 0;
    while (j < 64) {
        /* Double min reduction with >= */
        if (arr_d[j] >= min_double) {
            /* Keep min_double as is */
        } else {
            min_double = arr_d[j];
        }
        
        /* Conditional sum with >= and ternary */
        cond_sum_int += (arr_i[j] >= int_thresh) ? arr_i[j] : 0;
        
        /* Multiple reductions in one loop */
        if (arr_us[j] >= max_ushort) {
            max_ushort = arr_us[j];
        }
        
        j++;
    }
    
    /* Test 3: LT_EXPR (< operator) with multiple reductions */
    printf("Test 3: LT_EXPR patterns\n");
    for (int i = 0; i < 64; i++) {
        /* Integer min reduction with < */
        if (arr_i[i] < min_int) {
            min_int = arr_i[i];
        }
        
        /* Float conditional sum with < */
        if (arr_f[i] < float_thresh) {
            cond_sum_float -= arr_f[i];  /* Different operation */
        }
        
        /* Count values below threshold with logical OR in condition */
        if (i < 50 || arr_i[i] < int_thresh) {
            count_below++;
        }
    }
    
    /* Test 4: LE_EXPR (<= operator) with complex conditions */
    printf("Test 4: LE_EXPR patterns\n");
    for (int i = 0; i < 64; i++) {
        /* Double max reduction with <= in nested if */
        if (i % 2 == 0) {
            if (arr_d[i] <= max_double) {
                /* Do nothing for <= case */
            } else {
                max_double = arr_d[i];
            }
        }
        
        /* Float min reduction with <= */
        if (arr_f[i] <= min_float) {
            min_float = arr_f[i];
        }
        
        /* Conditional double sum with <= */
        cond_sum_double += (arr_d[i] <= double_thresh) ? arr_d[i] : 0.0;
    }
    
    /* Test 5: Mixed operators in single loop */
    printf("Test 5: Mixed conditional reductions\n");
    int mixed_max = arr_i[0];
    int mixed_min = arr_i[0];
    int mixed_sum_gt = 0;
    int mixed_count_le = 0;
    
    for (int i = 0; i < 64; i++) {
        /* GT for max */
        if (arr_i[i] > mixed_max) {
            mixed_max = arr_i[i];
        }
        
        /* LT for min */
        if (arr_i[i] < mixed_min) {
            mixed_min = arr_i[i];
        }
        
        /* GE for conditional sum */
        if (arr_i[i] >= int_thresh) {
            mixed_sum_gt += arr_i[i];
        }
        
        /* LE for count */
        if (arr_i[i] <= int_thresh + 10) {
            mixed_count_le++;
        }
    }
    
    /* Aggregate results into checksum */
    int checksum = 0;
    checksum += max_int;
    checksum += min_int;
    checksum += (int)max_ushort;
    checksum += (int)min_ushort;
    checksum += (int)max_float;
    checksum += (int)min_float;
    checksum += (int)max_double;
    checksum += (int)min_double;
    checksum += (int)cond_sum_int;
    checksum += (int)cond_sum_float;
    checksum += (int)cond_sum_double;
    checksum += count_above;
    checksum += count_below;
    checksum += mixed_max;
    checksum += mixed_min;
    checksum += mixed_sum_gt;
    checksum += mixed_count_le;
    
    /* Store to volatile to prevent optimization */
    g_result_int = checksum;
    g_result_float = cond_sum_float;
    g_result_double = cond_sum_double;
    
    printf("Checksum: %d\n", checksum);
    printf("Float sum: %f\n", cond_sum_float);
    printf("Double sum: %lf\n", cond_sum_double);
    
    return 0;
}
