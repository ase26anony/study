#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_sink;

/* Function to initialize arrays with deterministic but non-constant values */
void init_arrays(int seed, int* arr_i, unsigned short* arr_us, 
                 float* arr_f, double* arr_d, int size) {
    for (int i = 0; i < size; i++) {
        arr_i[i] = (i * 3 + seed) % 100;
        arr_us[i] = (unsigned short)((i * 5 + seed) % 65535);
        arr_f[i] = (float)((i * 7 + seed) % 100) * 1.5f;
        arr_d[i] = (double)((i * 11 + seed) % 100) * 0.75;
    }
}

int main(int argc, char** argv) {
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    const int SIZE = 64;
    
    /* Declare arrays with different types */
    int arr_i[SIZE];
    unsigned short arr_us[SIZE];
    float arr_f[SIZE];
    double arr_d[SIZE];
    
    /* Initialize arrays */
    init_arrays(seed, arr_i, arr_us, arr_f, arr_d, SIZE);
    
    /* Loop-invariant thresholds from volatile sources */
    volatile int vi = 25;
    volatile unsigned short vu = 30000;
    volatile float vf = 50.0f;
    volatile double vd = 30.0;
    
    int threshold_i = vi;
    unsigned short threshold_us = vu;
    float threshold_f = vf;
    double threshold_d = vd;
    
    /* Reduction variables */
    int max_i = arr_i[0];  /* For GT_EXPR pattern */
    int min_i = arr_i[0];  /* For LT_EXPR pattern */
    unsigned short max_us = arr_us[0];  /* For GE_EXPR pattern */
    float sum_f_cond = 0.0f;  /* For LE_EXPR pattern */
    double sum_d_cond = 0.0;  /* For mixed conditions */
    int count_gt = 0;  /* Counter for > comparisons */
    int count_lt = 0;  /* Counter for < comparisons */
    
    /* ===== Loop 1: GT_EXPR pattern (greater-than) ===== */
    /* This should trigger the GT_EXPR -> BIT_NOT_EXPR, BIT_AND_EXPR transformation */
    for (int i = 0; i < SIZE; i++) {
        /* Outer if to complicate control flow */
        if (i % 2 == 0) {
            /* Conditional reduction with > comparison */
            if (arr_i[i] > max_i) {
                max_i = arr_i[i];
            }
            
            /* Additional reduction in same loop */
            if (arr_i[i] > threshold_i) {
                count_gt++;
            }
        }
    }
    
    /* ===== Loop 2: GE_EXPR pattern (greater-than-or-equal) ===== */
    /* This should trigger the GE_EXPR -> BIT_NOT_EXPR, BIT_IOR_EXPR transformation */
    int i = 0;
    while (i < SIZE) {
        /* Using logical AND to combine conditions */
        if (i < SIZE - 1 && arr_us[i] >= max_us) {
            max_us = arr_us[i];
        }
        
        /* Nested conditional with GE comparison */
        if (arr_us[i] >= threshold_us) {
            if (i % 3 == 0) {
                /* Multiple reductions in nested context */
                sum_d_cond += (double)arr_us[i];
            }
        }
        i++;
    }
    
    /* ===== Loop 3: LT_EXPR pattern (less-than) ===== */
    /* This should trigger the LT_EXPR -> BIT_NOT_EXPR, BIT_AND_EXPR with swap */
    /* Multiple reductions with different conditions in one loop */
    for (int j = 0; j < SIZE; j++) {
        /* Conditional reduction with < comparison */
        if (arr_f[j] < threshold_f) {
            sum_f_cond += arr_f[j];
        }
        
        /* Another reduction with < comparison */
        if (arr_i[j] < min_i) {
            min_i = arr_i[j];
        }
        
        /* Counter for < comparisons */
        if (arr_i[j] < threshold_i) {
            count_lt++;
        }
    }
    
    /* ===== Loop 4: LE_EXPR pattern (less-than-or-equal) ===== */
    /* This should trigger the LE_EXPR -> BIT_NOT_EXPR, BIT_IOR_EXPR with swap */
    /* Complex conditional with logical OR */
    for (int k = 0; k < SIZE; k++) {
        /* Using logical OR in condition */
        if (k == 0 || arr_d[k] <= threshold_d) {
            sum_d_cond += arr_d[k];
        }
        
        /* Additional LE comparison in same loop */
        if (arr_i[k] <= threshold_i && k % 4 == 0) {
            /* Nested operation */
            max_i = (arr_i[k] > max_i) ? arr_i[k] : max_i;
        }
    }
    
    /* ===== Loop 5: Mixed comparisons in single loop ===== */
    /* Tests handling multiple conditional reductions */
    int mixed_max = arr_i[0];
    int mixed_min = arr_i[0];
    float mixed_sum = 0.0f;
    int mixed_count = 0;
    
    for (int m = 0; m < SIZE; m++) {
        /* GT comparison */
        if (arr_i[m] > mixed_max) {
            mixed_max = arr_i[m];
        }
        
        /* LT comparison */
        if (arr_i[m] < mixed_min) {
            mixed_min = arr_i[m];
        }
        
        /* GE comparison with ternary operator */
        mixed_sum += (arr_f[m] >= threshold_f) ? arr_f[m] : 0.0f;
        
        /* LE comparison */
        if (arr_us[m] <= threshold_us) {
            mixed_count++;
        }
    }
    
    /* Prevent dead code elimination */
    g_volatile_sink = max_i + min_i + max_us + count_gt + count_lt;
    
    /* Compute checksum from all reduction results */
    int checksum = 0;
    checksum += max_i;
    checksum += min_i;
    checksum += (int)max_us;
    checksum += (int)sum_f_cond;
    checksum += (int)sum_d_cond;
    checksum += count_gt;
    checksum += count_lt;
    checksum += mixed_max;
    checksum += mixed_min;
    checksum += (int)mixed_sum;
    checksum += mixed_count;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
