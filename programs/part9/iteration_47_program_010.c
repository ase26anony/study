#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile globals to prevent dead code elimination */
volatile int g_result_int;
volatile float g_result_float;
volatile double g_result_double;

/* Function to create loop-invariant thresholds */
int get_threshold(int seed) {
    volatile int v = seed;
    return v % 100 + 50;  /* Returns 50-149 */
}

float get_float_threshold(int seed) {
    volatile float v = seed * 0.7f;
    return v + 25.0f;  /* Loop-invariant float threshold */
}

/* Main test function with multiple conditional reduction patterns */
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
        arr_i[i] = (i * 3 + seed) % 200;
        arr_us[i] = (unsigned short)((i * 5 + seed) % 300);
        arr_f[i] = (float)((i * 7 + seed) % 150) * 0.5f;
        arr_d[i] = (double)((i * 11 + seed) % 180) * 0.3;
    }
    
    /* Get loop-invariant thresholds */
    int thresh_int = get_threshold(seed);
    float thresh_float = get_float_threshold(seed);
    double thresh_double = thresh_float * 1.5;
    
    /* Reduction variables */
    int max_val = -1000;      /* For GT_EXPR pattern */
    int min_val = 1000;       /* For LT_EXPR pattern */
    float sum_ge = 0.0f;      /* For GE_EXPR pattern */
    double sum_le = 0.0;      /* For LE_EXPR pattern */
    unsigned short max_ushort = 0;  /* For mixed types */
    int count_gt = 0;         /* For counting pattern */
    
    /* ===== TEST 1: GT_EXPR (greater-than) pattern ===== */
    /* Pattern: if (arr_i[i] > current_max) current_max = arr_i[i] */
    for (int i = 0; i < 64; i++) {
        /* Nested conditional to complicate control flow */
        if (seed > 0) {
            if (arr_i[i] > max_val) {
                max_val = arr_i[i];
            }
        }
    }
    
    /* ===== TEST 2: GE_EXPR (greater-or-equal) pattern ===== */
    /* Pattern: sum += (arr_f[i] >= threshold) ? arr_f[i] : 0 */
    for (int i = 0; i < 64; i++) {
        /* Combined with logical OR in outer condition */
        if (i % 2 == 0 || seed < 1000) {
            if (arr_f[i] >= thresh_float) {
                sum_ge += arr_f[i];
            }
        }
    }
    
    /* ===== TEST 3: LT_EXPR (less-than) pattern ===== */
    /* Pattern: if (arr_i[i] < current_min) current_min = arr_i[i] */
    int i = 0;
    while (i < 64) {
        /* Multiple reductions in same loop */
        if (arr_i[i] < min_val) {
            min_val = arr_i[i];
        }
        
        /* Additional reduction with different condition */
        if (arr_i[i] > thresh_int) {
            count_gt++;
        }
        
        i++;
    }
    
    /* ===== TEST 4: LE_EXPR (less-or-equal) pattern ===== */
    /* Pattern: sum += (arr_d[i] <= threshold) ? arr_d[i] : 0 */
    for (int i = 0; i < 64; i++) {
        /* Complex conditional with AND */
        if (i > 0 && i < 63) {
            if (arr_d[i] <= thresh_double) {
                sum_le += arr_d[i];
            }
        }
    }
    
    /* ===== TEST 5: Multiple reductions with different comparisons ===== */
    /* Single loop with multiple conditional reductions */
    int multi_max = -1000;
    int multi_min = 1000;
    float multi_sum = 0.0f;
    
    for (int i = 0; i < 64; i++) {
        /* GT_EXPR pattern */
        if (arr_i[i] > multi_max) {
            multi_max = arr_i[i];
        }
        
        /* LT_EXPR pattern */
        if (arr_i[i] < multi_min) {
            multi_min = arr_i[i];
        }
        
        /* GE_EXPR pattern with different array */
        if (arr_f[i] >= thresh_float * 0.5f) {
            multi_sum += arr_f[i];
        }
    }
    
    /* ===== TEST 6: Unsigned short with LE_EXPR ===== */
    /* Pattern: max = (arr_us[i] <= current) ? current : arr_us[i] */
    /* Note: This will trigger std::swap(cond_expr0, cond_expr1) for LE_EXPR */
    unsigned short current_max = arr_us[0];
    for (int i = 1; i < 64; i++) {
        if (current_max <= arr_us[i]) {
            current_max = arr_us[i];
        }
    }
    max_ushort = current_max;
    
    /* ===== TEST 7: Mixed types and nested loops ===== */
    float nested_max = -1e6f;
    float nested_min = 1e6f;
    
    for (int outer = 0; outer < 2; outer++) {
        for (int i = 0; i < 32; i++) {
            int idx = outer * 32 + i;
            /* GT_EXPR and LT_EXPR in nested loop */
            if (arr_f[idx] > nested_max) {
                nested_max = arr_f[idx];
            }
            if (arr_f[idx] < nested_min) {
                nested_min = arr_f[idx];
            }
        }
    }
    
    /* Prevent optimization */
    g_result_int = max_val;
    g_result_float = sum_ge;
    g_result_double = sum_le;
    
    /* Compute checksum */
    int checksum = max_val + min_val + (int)sum_ge + (int)sum_le + 
                   max_ushort + count_gt + multi_max + multi_min + 
                   (int)multi_sum + (int)nested_max + (int)nested_min;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
