#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile globals to prevent dead code elimination */
volatile int g_result_int = 0;
volatile float g_result_float = 0.0f;

/* Function to generate deterministic data based on seed */
static void init_arrays(int seed, int arr_i[], unsigned short arr_us[], 
                       float arr_f[], double arr_d[], int size) {
    for (int i = 0; i < size; i++) {
        arr_i[i] = (i * 3 + seed) % 100;
        arr_us[i] = (unsigned short)((i * 5 + seed) % 65535);
        arr_f[i] = (float)((i * 7 + seed) % 100) * 1.5f;
        arr_d[i] = (double)((i * 11 + seed) % 100) * 0.75;
    }
}

int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    const int SIZE = 64;
    
    /* Declare arrays */
    int arr_i[SIZE];
    unsigned short arr_us[SIZE];
    float arr_f[SIZE];
    double arr_d[SIZE];
    
    /* Initialize arrays with deterministic pattern */
    init_arrays(seed, arr_i, arr_us, arr_f, arr_d, SIZE);
    
    /* Loop-invariant thresholds from volatile sources */
    volatile int vi_thresh = 30;
    volatile float vf_thresh = 45.0f;
    volatile double vd_thresh = 35.0;
    volatile unsigned short vus_thresh = 20000;
    
    /* Extract thresholds to prevent constant propagation */
    int int_thresh = vi_thresh;
    float float_thresh = vf_thresh;
    double double_thresh = vd_thresh;
    unsigned short us_thresh = vus_thresh;
    
    /* Reduction variables - initialized with volatile reads */
    volatile int v_init_max = -1000;
    volatile int v_init_min = 1000;
    volatile float v_init_fmax = -1000.0f;
    volatile double v_init_dmin = 1000.0;
    
    int max_val = v_init_max;
    int min_val = v_init_min;
    float fmax_val = v_init_fmax;
    double dmin_val = v_init_dmin;
    int cond_sum_gt = 0;
    int cond_sum_ge = 0;
    unsigned short cond_sum_lt = 0;
    float cond_sum_le = 0.0f;
    int count_gt = 0;
    int count_lt = 0;
    
    /* ===== TEST 1: Greater-than (GT_EXPR) conditional reduction ===== */
    /* This should trigger BIT_NOT_EXPR + BIT_AND_EXPR transformation */
    for (int i = 0; i < SIZE; i++) {
        /* Outer if to complicate control flow */
        if (i % 2 == 0) {
            /* Conditional max reduction with > */
            if (arr_i[i] > max_val) {
                max_val = arr_i[i];
            }
            
            /* Conditional sum with > */
            if (arr_i[i] > int_thresh) {
                cond_sum_gt += arr_i[i];
            }
        }
    }
    
    /* ===== TEST 2: Greater-than-or-equal (GE_EXPR) with multiple reductions ===== */
    /* This should trigger BIT_NOT_EXPR + BIT_IOR_EXPR transformation */
    /* Multiple reductions in one loop */
    for (int i = 0; i < SIZE; i++) {
        /* Combined condition with logical AND */
        if (i < SIZE - 1 && arr_i[i] >= int_thresh) {
            /* Count values >= threshold */
            count_gt++;
            
            /* Nested conditional for additional complexity */
            if (arr_us[i % SIZE] >= us_thresh) {
                cond_sum_ge += arr_i[i];
            }
        }
        
        /* Simultaneous float max with >= */
        if (arr_f[i] >= fmax_val) {
            fmax_val = arr_f[i];
        }
    }
    
    /* ===== TEST 3: Less-than (LT_EXPR) with while loop ===== */
    /* This should trigger BIT_NOT_EXPR + BIT_AND_EXPR with swapped operands */
    int j = 0;
    while (j < SIZE) {
        /* Conditional min reduction with < */
        if (arr_i[j] < min_val) {
            min_val = arr_i[j];
        }
        
        /* Conditional sum with < and logical OR */
        if (j % 3 == 0 || arr_i[j] < int_thresh) {
            cond_sum_lt += arr_us[j];
        }
        
        j++;
    }
    
    /* ===== TEST 4: Less-than-or-equal (LE_EXPR) with mixed types ===== */
    /* This should trigger BIT_NOT_EXPR + BIT_IOR_EXPR with swapped operands */
    for (int i = 0; i < SIZE; i++) {
        /* Double min reduction with <= */
        if (arr_d[i] <= dmin_val) {
            dmin_val = arr_d[i];
        }
        
        /* Conditional float sum with <= */
        if (arr_f[i] <= float_thresh) {
            cond_sum_le += arr_f[i];
            count_lt++;
        }
        
        /* Additional reduction with combined condition */
        if (arr_i[i] <= int_thresh && arr_f[i] <= float_thresh) {
            /* Complex nested conditional */
            if (i % 4 == 0) {
                cond_sum_gt += arr_i[i];
            }
        }
    }
    
    /* ===== TEST 5: All comparisons in one complex loop ===== */
    int complex_max = v_init_max;
    int complex_min = v_init_min;
    float complex_fsum = 0.0f;
    int complex_count = 0;
    
    for (int i = 0; i < SIZE; i++) {
        /* GT comparison */
        if (arr_i[i] > complex_max) {
            complex_max = arr_i[i];
        }
        
        /* GE comparison */
        if (arr_i[i] >= int_thresh && i % 2 == 0) {
            complex_count++;
        }
        
        /* LT comparison */
        if (arr_f[i] < float_thresh) {
            complex_fsum += arr_f[i];
        }
        
        /* LE comparison with logical OR */
        if (arr_d[i] <= double_thresh || i % 3 == 0) {
            complex_min = (arr_i[i] < complex_min) ? arr_i[i] : complex_min;
        }
    }
    
    /* Aggregate results into checksum to prevent optimization */
    int checksum = 0;
    checksum += max_val;
    checksum += min_val;
    checksum += (int)fmax_val;
    checksum += (int)dmin_val;
    checksum += cond_sum_gt;
    checksum += cond_sum_ge;
    checksum += cond_sum_lt;
    checksum += (int)cond_sum_le;
    checksum += count_gt;
    checksum += count_lt;
    checksum += complex_max;
    checksum += complex_min;
    checksum += (int)complex_fsum;
    checksum += complex_count;
    
    /* Store to volatile to ensure computation happens */
    g_result_int = checksum;
    g_result_float = complex_fsum;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
