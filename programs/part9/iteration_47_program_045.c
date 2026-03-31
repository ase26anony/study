#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_result = 0;
volatile float g_float_result = 0.0f;

/* Function to create loop-invariant thresholds */
int get_threshold(int seed) {
    volatile int v = seed;
    return v % 100 + 50;  /* Loop invariant but not compile-time constant */
}

float get_float_threshold(int seed) {
    volatile float v = seed * 1.5f;
    return v + 25.0f;  /* Loop invariant */
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
        arr_i[i] = (i * 3 + seed) % 200 - 100;      /* Range: -100 to 99 */
        arr_us[i] = (i * 5 + seed) % 65535;         /* Range: 0 to 65534 */
        arr_f[i] = (i * 7 + seed * 1.3f) / 10.0f;   /* Various float values */
        arr_d[i] = (i * 11 + seed * 1.7) / 8.0;     /* Various double values */
    }
    
    /* Get loop-invariant thresholds */
    int int_thresh = get_threshold(seed);
    float float_thresh = get_float_threshold(seed);
    unsigned short us_thresh = (seed * 13) % 30000;
    double double_thresh = seed * 2.5;
    
    /* Reduction variables */
    int max_val = arr_i[0];
    int min_val = arr_i[0];
    int cond_sum_gt = 0;
    int cond_sum_ge = 0;
    float float_max = arr_f[0];
    float float_min = arr_f[0];
    unsigned short us_max = arr_us[0];
    unsigned short us_min = arr_us[0];
    double double_sum_lt = 0.0;
    double double_sum_le = 0.0;
    int count_gt = 0;
    int count_lt = 0;
    
    /* ===== Test 1: GT_EXPR (greater-than) with multiple reductions ===== */
    /* This should trigger the GT_EXPR case with BIT_NOT_EXPR and BIT_AND_EXPR */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (arr_i[i] > -1000) {  /* Always true, but prevents simple analysis */
            /* Conditional max reduction with > */
            if (arr_i[i] > max_val) {
                max_val = arr_i[i];
            }
            
            /* Conditional sum with > */
            if (arr_i[i] > int_thresh) {
                cond_sum_gt += arr_i[i];
                count_gt++;
            }
            
            /* Combined condition with logical AND */
            if (arr_i[i] > (int_thresh - 20) && arr_i[i] < (int_thresh + 20)) {
                /* Additional operation to make body non-trivial */
                min_val = (arr_i[i] < min_val) ? arr_i[i] : min_val;
            }
        }
    }
    
    /* ===== Test 2: GE_EXPR (greater-than-or-equal) ===== */
    /* This should trigger the GE_EXPR case with BIT_NOT_EXPR and BIT_IOR_EXPR */
    for (int i = 0; i < 64; i++) {
        /* Nested conditionals */
        if (arr_us[i] > 100) {
            if (arr_us[i] >= us_thresh) {
                cond_sum_ge += arr_us[i];
            }
            
            /* Multiple reductions in same loop */
            if (arr_us[i] >= us_max) {
                us_max = arr_us[i];
            }
            
            /* Logical OR in condition */
            if (arr_us[i] >= us_thresh || arr_us[i] < 1000) {
                us_min = (arr_us[i] < us_min) ? arr_us[i] : us_min;
            }
        }
    }
    
    /* ===== Test 3: LT_EXPR (less-than) with while loop ===== */
    /* This should trigger the LT_EXPR case with swap and BIT_AND_EXPR */
    int j = 0;
    while (j < 64) {
        /* Conditional min reduction with < */
        if (arr_f[j] < float_min) {
            float_min = arr_f[j];
        }
        
        /* Conditional operation with < */
        if (arr_f[j] < float_thresh) {
            float_max = (arr_f[j] > float_max) ? arr_f[j] : float_max;
        }
        
        /* Complex condition */
        if (j % 2 == 0 && arr_f[j] < (float_thresh + 10.0f)) {
            count_lt++;
        }
        
        j++;
    }
    
    /* ===== Test 4: LE_EXPR (less-than-or-equal) ===== */
    /* This should trigger the LE_EXPR case with swap and BIT_IOR_EXPR */
    for (int i = 0; i < 64; i++) {
        /* Multiple double reductions with <= */
        if (arr_d[i] <= double_thresh) {
            double_sum_le += arr_d[i];
        }
        
        /* Another reduction with different condition */
        if (arr_d[i] <= (double_thresh * 1.5)) {
            double_sum_lt += arr_d[i];
        }
        
        /* Nested conditional */
        if (i > 10) {
            if (arr_d[i] <= double_thresh && arr_d[i] > 0) {
                /* Additional operation */
                double_sum_le *= 1.0001;
            }
        }
    }
    
    /* ===== Test 5: Mixed types and operations in single loop ===== */
    int mixed_sum = 0;
    float mixed_float_max = arr_f[0];
    int mixed_count = 0;
    
    for (int i = 0; i < 64; i++) {
        /* Integer comparison with > */
        if (arr_i[i] > (int_thresh / 2)) {
            mixed_sum += arr_i[i];
        }
        
        /* Float comparison with < */
        if (arr_f[i] < (float_thresh * 1.2f)) {
            if (arr_f[i] > mixed_float_max) {
                mixed_float_max = arr_f[i];
            }
        }
        
        /* Unsigned comparison with >= */
        if (arr_us[i] >= (us_thresh / 3)) {
            mixed_count++;
        }
        
        /* Double comparison with <= in logical expression */
        if (i % 3 == 0 && arr_d[i] <= double_thresh) {
            mixed_sum += (int)arr_d[i];
        }
    }
    
    /* Aggregate results into checksum */
    int checksum = 0;
    checksum += max_val;
    checksum += min_val;
    checksum += cond_sum_gt;
    checksum += cond_sum_ge;
    checksum += (int)float_max;
    checksum += (int)(float_min * 100);
    checksum += us_max;
    checksum += us_min;
    checksum += (int)double_sum_lt;
    checksum += (int)double_sum_le;
    checksum += count_gt;
    checksum += count_lt;
    checksum += mixed_sum;
    checksum += (int)(mixed_float_max * 10);
    checksum += mixed_count;
    
    /* Store to volatile to prevent dead code elimination */
    g_result = checksum;
    g_float_result = mixed_float_max;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
