#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile globals to prevent dead code elimination */
volatile int g_result_int = 0;
volatile float g_result_float = 0.0f;

/* Function to generate deterministic but non-constant data */
static inline int gen_data(int i, int seed) {
    return (i * 3 + seed) % 100;
}

static inline float gen_float_data(int i, int seed) {
    return (float)((i * 7 + seed * 13) % 1000) / 10.0f;
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
    
    /* Initialize arrays with deterministic but non-constant data */
    for (int i = 0; i < 64; i++) {
        arr_i[i] = gen_data(i, seed);
        arr_us[i] = (unsigned short)(gen_data(i, seed + 1) & 0xFFFF);
        arr_f[i] = gen_float_data(i, seed);
        arr_d[i] = (double)gen_float_data(i, seed + 3);
    }
    
    /* Loop-invariant thresholds from volatile sources */
    volatile int thresh_int = seed + 50;
    volatile float thresh_float = (float)(seed % 30) + 25.5f;
    volatile double thresh_double = (double)(seed % 40) + 35.7;
    
    int threshold_i = thresh_int;
    float threshold_f = thresh_float;
    double threshold_d = thresh_double;
    
    /* Reduction variables */
    int max_val_int = -1000;
    int min_val_int = 1000;
    float max_val_float = -1000.0f;
    float min_val_float = 1000.0f;
    double cond_sum_double = 0.0;
    unsigned int count_ge = 0;
    unsigned int count_le = 0;
    int cond_sum_int = 0;
    float cond_sum_float = 0.0f;
    
    /* ===== TEST 1: GT_EXPR (greater-than) with mixed reductions ===== */
    /* This should trigger the GT_EXPR -> BIT_NOT_EXPR, BIT_AND_EXPR transformation */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (i % 3 != 0) {
            /* Conditional max reduction with > */
            if (arr_i[i] > max_val_int) {
                max_val_int = arr_i[i];
            }
            
            /* Conditional min reduction with > in a different form */
            int temp = arr_i[i];
            if (temp > threshold_i) {
                cond_sum_int += temp;  /* Sum values greater than threshold */
            }
        }
    }
    
    /* ===== TEST 2: GE_EXPR (greater-than-or-equal) ===== */
    /* This should trigger the GE_EXPR -> BIT_NOT_EXPR, BIT_IOR_EXPR transformation */
    int limit = 64;
    int j = 0;
    while (j < limit) {
        /* Combined condition with logical AND */
        if (j < 60 && arr_us[j] >= (unsigned short)threshold_i) {
            count_ge++;
            if (arr_us[j] > max_val_int) {  /* Nested comparison */
                max_val_int = arr_us[j];
            }
        }
        j++;
    }
    
    /* ===== TEST 3: LT_EXPR (less-than) with multiple data types ===== */
    /* This should trigger the LT_EXPR -> BIT_NOT_EXPR, BIT_AND_EXPR with swap */
    for (int i = 0; i < 64; i++) {
        /* Conditional min with < */
        if (arr_f[i] < min_val_float) {
            min_val_float = arr_f[i];
        }
        
        /* Another reduction with < in same loop */
        if (arr_i[i] < threshold_i) {
            count_le++;
        }
        
        /* Nested conditional with logical OR */
        if (i % 4 == 0 || arr_f[i] < threshold_f) {
            cond_sum_float += arr_f[i];
        }
    }
    
    /* ===== TEST 4: LE_EXPR (less-than-or-equal) with double type ===== */
    /* This should trigger the LE_EXPR -> BIT_NOT_EXPR, BIT_IOR_EXPR with swap */
    for (int i = 0; i < 64; i++) {
        /* Outer if with compound condition */
        if (i > 10 && i < 50) {
            /* Conditional reduction with <= */
            if (arr_d[i] <= threshold_d) {
                cond_sum_double += arr_d[i];
            }
            
            /* Another conditional with <= for max */
            if (arr_d[i] <= max_val_float) {  /* Mixed type comparison */
                max_val_float = (float)arr_d[i];
            }
        }
    }
    
    /* ===== TEST 5: All four comparisons in one complex loop ===== */
    int complex_max = -1000;
    int complex_min = 1000;
    int complex_sum_gt = 0;
    int complex_count_le = 0;
    
    for (int i = 0; i < 64; i++) {
        /* Multiple conditional reductions in one loop */
        if (arr_i[i] > complex_max) {           /* GT_EXPR */
            complex_max = arr_i[i];
        }
        
        if (arr_i[i] >= threshold_i) {          /* GE_EXPR */
            complex_sum_gt += arr_i[i];
        }
        
        if (arr_i[i] < complex_min) {           /* LT_EXPR */
            complex_min = arr_i[i];
        }
        
        if (arr_i[i] <= threshold_i + 10) {     /* LE_EXPR */
            complex_count_le++;
        }
    }
    
    /* Aggregate results into checksum to prevent optimization */
    int checksum = 0;
    checksum += max_val_int;
    checksum += min_val_int;
    checksum += (int)max_val_float;
    checksum += (int)min_val_float;
    checksum += (int)cond_sum_double;
    checksum += count_ge;
    checksum += count_le;
    checksum += cond_sum_int;
    checksum += (int)cond_sum_float;
    checksum += complex_max;
    checksum += complex_min;
    checksum += complex_sum_gt;
    checksum += complex_count_le;
    
    /* Store to volatile to ensure computation happens */
    g_result_int = checksum;
    g_result_float = cond_sum_float;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
