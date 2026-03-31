#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile globals to prevent dead code elimination */
volatile int g_result_int = 0;
volatile float g_result_float = 0.0f;

/* Function to create loop-invariant thresholds */
int get_threshold(int seed) {
    volatile int v = seed;
    return v % 100 + 50;  /* Loop-invariant but not compile-time constant */
}

float get_float_threshold(int seed) {
    volatile float v = (float)(seed % 100);
    return v * 0.5f + 25.0f;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    
    /* Arrays with deterministic but runtime-dependent values */
    int arr_i[64];
    unsigned short arr_us[64];
    float arr_f[64];
    double arr_d[64];
    
    /* Initialize arrays */
    for (int i = 0; i < 64; i++) {
        arr_i[i] = (i * 3 + seed) % 1000;
        arr_us[i] = (unsigned short)((i * 5 + seed * 2) % 65535);
        arr_f[i] = (float)((i * 7 + seed * 3) % 1000) * 0.1f;
        arr_d[i] = (double)((i * 11 + seed * 5) % 1000) * 0.05;
    }
    
    /* Loop-invariant thresholds */
    int thresh_i = get_threshold(seed);
    float thresh_f = get_float_threshold(seed);
    unsigned short thresh_us = (unsigned short)(seed % 100 + 100);
    double thresh_d = (double)(seed % 200) * 0.25;
    
    /* Reduction variables */
    int max_val = arr_i[0];
    int min_val = arr_i[0];
    int cond_sum_gt = 0;
    int cond_sum_ge = 0;
    float float_max = arr_f[0];
    float float_min = arr_f[0];
    unsigned short us_min = arr_us[0];
    double double_sum_lt = 0.0;
    int count_le = 0;
    
    /* Loop 1: GT_EXPR (> operator) with nested conditional */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (arr_i[i] != 0) {
            /* GT_EXPR: if (arr_i[i] > max_val) max_val = arr_i[i]; */
            if (arr_i[i] > max_val) {
                max_val = arr_i[i];
            }
            
            /* Combined with logical AND */
            if (arr_i[i] > thresh_i && arr_i[i] < 1000) {
                cond_sum_gt += arr_i[i];
            }
        }
    }
    
    /* Loop 2: GE_EXPR (>= operator) with while loop */
    int j = 0;
    while (j < 64) {
        /* GE_EXPR: sum values >= threshold */
        if (arr_i[j] >= thresh_i) {
            cond_sum_ge += arr_i[j];
        }
        
        /* Multiple reductions in one loop */
        if (arr_i[j] >= arr_i[0] || j % 2 == 0) {
            if (arr_i[j] < min_val) {  /* LT_EXPR inside */
                min_val = arr_i[j];
            }
        }
        j++;
    }
    
    /* Loop 3: LT_EXPR (< operator) with float array */
    for (int i = 0; i < 64; i++) {
        /* LT_EXPR: if (arr_f[i] < float_min) float_min = arr_f[i]; */
        if (arr_f[i] < float_min) {
            float_min = arr_f[i];
        }
        
        /* Multiple conditions with logical OR */
        if (arr_f[i] < thresh_f || i % 3 == 0) {
            if (arr_f[i] > float_max) {  /* GT_EXPR inside */
                float_max = arr_f[i];
            }
        }
    }
    
    /* Loop 4: LE_EXPR (<= operator) with unsigned short */
    for (int i = 0; i < 64; i++) {
        /* LE_EXPR: if (arr_us[i] <= us_min) us_min = arr_us[i]; */
        if (arr_us[i] <= us_min) {
            us_min = arr_us[i];
        }
        
        /* Count values <= threshold */
        if (arr_us[i] <= thresh_us) {
            count_le++;
        }
    }
    
    /* Loop 5: Mixed types and multiple reductions with double */
    double double_max = arr_d[0];
    double double_min = arr_d[0];
    for (int i = 0; i < 64; i++) {
        /* LT_EXPR for double */
        if (arr_d[i] < double_min) {
            double_min = arr_d[i];
        }
        
        /* GT_EXPR for double */
        if (arr_d[i] > double_max) {
            double_max = arr_d[i];
        }
        
        /* LE_EXPR: sum values <= threshold */
        if (arr_d[i] <= thresh_d) {
            double_sum_lt += arr_d[i];
        }
    }
    
    /* Loop 6: Complex nested conditionals with all operators */
    int complex_sum = 0;
    int complex_max = arr_i[0];
    for (int i = 0; i < 64; i++) {
        /* Outer if with AND */
        if (i > 0 && i < 63) {
            /* GT_EXPR */
            if (arr_i[i] > complex_max) {
                complex_max = arr_i[i];
            }
            
            /* GE_EXPR with logical OR */
            if (arr_i[i] >= thresh_i || arr_i[i] % 2 == 0) {
                complex_sum += arr_i[i];
            }
            
            /* Nested LT_EXPR */
            if (arr_i[i] < thresh_i + 100) {
                if (arr_i[i] <= arr_i[i-1] + 10) {  /* LE_EXPR */
                    complex_sum += arr_i[i] * 2;
                }
            }
        }
    }
    
    /* Aggregate results into checksum */
    uint64_t checksum = 0;
    checksum += max_val;
    checksum += min_val;
    checksum += cond_sum_gt;
    checksum += cond_sum_ge;
    checksum += (int)(float_max * 100);
    checksum += (int)(float_min * 100);
    checksum += us_min;
    checksum += count_le;
    checksum += (uint64_t)(double_max * 1000);
    checksum += (uint64_t)(double_min * 1000);
    checksum += (uint64_t)(double_sum_lt * 100);
    checksum += complex_sum;
    checksum += complex_max;
    
    /* Store to volatile to prevent elimination */
    g_result_int = (int)checksum;
    g_result_float = (float)checksum;
    
    printf("Checksum: %lu\n", checksum);
    
    return 0;
}
