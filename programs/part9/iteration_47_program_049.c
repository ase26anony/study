#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent dead code elimination */
volatile int g_result = 0;
volatile float g_float_result = 0.0f;

/* Function to initialize arrays deterministically */
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
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    const int SIZE = 64;
    int arr_i[SIZE];
    unsigned short arr_us[SIZE];
    float arr_f[SIZE];
    double arr_d[SIZE];
    
    init_arrays(seed, arr_i, arr_us, arr_f, arr_d, SIZE);
    
    /* Loop-invariant thresholds from volatile sources */
    volatile int vi_thresh = 30;
    volatile float vf_thresh = 45.0f;
    volatile double vd_thresh = 35.0;
    volatile unsigned short vus_thresh = 20000;
    
    int thresh_i = vi_thresh;
    float thresh_f = vf_thresh;
    double thresh_d = vd_thresh;
    unsigned short thresh_us = vus_thresh;
    
    /* Reduction variables */
    int max_val_i = arr_i[0];
    int min_val_i = arr_i[0];
    unsigned short max_val_us = arr_us[0];
    float cond_sum_f = 0.0f;
    double cond_sum_d = 0.0;
    int count_gt = 0;
    int count_le = 0;
    
    /* Example 1: GT_EXPR pattern - find maximum with > comparison */
    /* Mixed with outer conditional to complicate control flow */
    if (seed > 0) {
        for (int i = 0; i < SIZE; i++) {
            /* Conditional reduction with > */
            if (arr_i[i] > max_val_i) {
                max_val_i = arr_i[i];
            }
            
            /* Additional reduction with different condition */
            if (arr_us[i] > thresh_us) {
                count_gt++;
            }
        }
    }
    
    /* Example 2: GE_EXPR pattern - conditional sum with >= */
    /* Using while loop variant */
    int idx = 0;
    while (idx < SIZE) {
        /* Conditional reduction with >= */
        if (arr_f[idx] >= thresh_f) {
            cond_sum_f += arr_f[idx];
        }
        
        /* Nested conditionals with logical OR */
        if (idx % 2 == 0 || arr_f[idx] >= thresh_f * 0.5f) {
            /* Another reduction inside nested conditional */
            if (arr_i[idx] >= thresh_i) {
                g_result += arr_i[idx];
            }
        }
        idx++;
    }
    
    /* Example 3: LT_EXPR pattern - find minimum with < */
    /* Multiple reductions in one loop */
    for (int i = 0; i < SIZE; i++) {
        /* Conditional reduction with < */
        if (arr_i[i] < min_val_i) {
            min_val_i = arr_i[i];
        }
        
        /* Another reduction with different comparison */
        if (arr_d[i] < thresh_d) {
            cond_sum_d += arr_d[i];
        }
        
        /* Combined condition with logical AND */
        if (arr_i[i] < thresh_i && arr_us[i] < thresh_us) {
            count_le++;
        }
    }
    
    /* Example 4: LE_EXPR pattern - conditional operations with <= */
    /* Complex control flow with outer if */
    if (max_val_i > min_val_i) {
        int temp_max = arr_i[0];
        int temp_min = arr_i[0];
        float temp_sum = 0.0f;
        
        for (int i = 0; i < SIZE; i++) {
            /* Multiple conditional reductions with <= */
            if (arr_i[i] <= temp_max) {
                /* Do nothing, this is the inverse of finding max */
            } else {
                temp_max = arr_i[i];
            }
            
            if (arr_i[i] <= temp_min) {
                temp_min = arr_i[i];
            }
            
            /* Conditional sum with <= on float */
            if (arr_f[i] <= thresh_f * 2.0f) {
                temp_sum += arr_f[i];
            }
        }
        
        /* Use results to prevent elimination */
        g_float_result = temp_sum;
    }
    
    /* Example 5: Mixed comparisons in one loop */
    int mixed_max = arr_i[0];
    int mixed_min = arr_i[0];
    double mixed_sum = 0.0;
    int mixed_count = 0;
    
    for (int i = 0; i < SIZE; i++) {
        /* GT comparison */
        if (arr_i[i] > mixed_max) {
            mixed_max = arr_i[i];
        }
        
        /* LT comparison */
        if (arr_i[i] < mixed_min) {
            mixed_min = arr_i[i];
        }
        
        /* GE comparison */
        if (arr_f[i] >= thresh_f) {
            mixed_sum += arr_f[i];
        }
        
        /* LE comparison */
        if (arr_us[i] <= thresh_us) {
            mixed_count++;
        }
    }
    
    /* Aggregate results into checksum */
    uint32_t checksum = 0;
    checksum += max_val_i;
    checksum += min_val_i;
    checksum += max_val_us;
    checksum += (uint32_t)cond_sum_f;
    checksum += (uint32_t)cond_sum_d;
    checksum += count_gt;
    checksum += count_le;
    checksum += (uint32_t)g_float_result;
    checksum += mixed_max;
    checksum += mixed_min;
    checksum += (uint32_t)mixed_sum;
    checksum += mixed_count;
    
    printf("Checksum: %u\n", checksum);
    
    return 0;
}
