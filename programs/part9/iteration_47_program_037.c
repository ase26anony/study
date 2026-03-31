#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile globals to prevent dead code elimination */
volatile int g_result_int = 0;
volatile float g_result_float = 0.0f;

/* Function to create loop-invariant thresholds */
int get_threshold(int seed) {
    volatile int v = seed;
    return v % 100 + 50;  /* Returns 50-149 */
}

float get_fthreshold(int seed) {
    volatile float v = seed * 0.7f;
    return v + 25.0f;  /* Returns 25+ */
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    
    /* Initialize arrays with deterministic but non-constant patterns */
    int arr_i[64];
    unsigned short arr_us[64];
    float arr_f[64];
    double arr_d[64];
    
    for (int i = 0; i < 64; i++) {
        arr_i[i] = (i * 3 + seed) % 200;
        arr_us[i] = (unsigned short)((i * 5 + seed * 2) % 300);
        arr_f[i] = (float)((i * 7 + seed * 3) % 150) * 0.5f;
        arr_d[i] = (double)((i * 11 + seed * 5) % 180) * 0.3;
    }
    
    /* Loop-invariant thresholds */
    int thresh_i = get_threshold(seed);
    float thresh_f = get_fthreshold(seed);
    unsigned short thresh_us = (unsigned short)(seed % 150 + 50);
    double thresh_d = (double)(seed % 100 + 75) * 0.4;
    
    /* Reduction variables */
    int max_val = -1000;
    int min_val = 1000;
    unsigned short max_us = 0;
    float cond_sum_f = 0.0f;
    double cond_sum_d = 0.0;
    int count_gt = 0;
    int count_le = 0;
    
    /* ====== Loop 1: GT_EXPR pattern with nested conditionals ====== */
    /* This should trigger: case GT_EXPR: bitop1 = BIT_NOT_EXPR; bitop2 = BIT_AND_EXPR; */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (i % 3 != 0) {
            /* Multiple reductions with GT_EXPR */
            if (arr_i[i] > max_val) {
                max_val = arr_i[i];
            }
            
            /* Combined with logical AND */
            if (arr_i[i] > thresh_i && i % 2 == 0) {
                count_gt++;
            }
        }
    }
    
    /* ====== Loop 2: GE_EXPR pattern with multiple reductions ====== */
    /* This should trigger: case GE_EXPR: bitop1 = BIT_NOT_EXPR; bitop2 = BIT_IOR_EXPR; */
    for (int i = 0; i < 64; i++) {
        /* Conditional sum with GE_EXPR */
        if (arr_f[i] >= thresh_f) {
            cond_sum_f += arr_f[i];
        }
        
        /* Another reduction in same loop */
        if (arr_us[i] >= thresh_us) {
            if (arr_us[i] > max_us) {
                max_us = arr_us[i];
            }
        }
    }
    
    /* ====== Loop 3: LT_EXPR pattern in while loop ====== */
    /* This should trigger: case LT_EXPR with swap */
    int j = 0;
    while (j < 64) {
        /* LT_EXPR for min finding */
        if (arr_i[j] < min_val) {
            min_val = arr_i[j];
        }
        
        /* Combined with logical OR */
        if (arr_f[j] < thresh_f || j % 4 == 0) {
            cond_sum_f += arr_f[j] * 0.5f;
        }
        j++;
    }
    
    /* ====== Loop 4: LE_EXPR pattern with mixed types ====== */
    /* This should trigger: case LE_EXPR with swap */
    for (int i = 0; i < 64; i++) {
        /* LE_EXPR conditional sum */
        if (arr_d[i] <= thresh_d) {
            cond_sum_d += arr_d[i];
            count_le++;
        }
        
        /* Multiple conditions in same loop */
        if (arr_i[i] <= thresh_i && arr_f[i] <= thresh_f) {
            max_val = (arr_i[i] > max_val) ? arr_i[i] : max_val;
        }
    }
    
    /* ====== Loop 5: All four operators in one complex loop ====== */
    int sum_all = 0;
    float max_f = -1e9f;
    double min_d = 1e9;
    
    for (int i = 0; i < 64; i++) {
        /* GT_EXPR */
        if (arr_i[i] > thresh_i) {
            sum_all += arr_i[i];
        }
        
        /* GE_EXPR */
        if (arr_f[i] >= thresh_f) {
            if (arr_f[i] > max_f) {
                max_f = arr_f[i];
            }
        }
        
        /* LT_EXPR */
        if (arr_d[i] < thresh_d) {
            if (arr_d[i] < min_d) {
                min_d = arr_d[i];
            }
        }
        
        /* LE_EXPR */
        if (arr_us[i] <= thresh_us) {
            sum_all += arr_us[i];
        }
    }
    
    /* Aggregate results into checksum */
    int checksum = max_val + min_val + max_us + (int)cond_sum_f + 
                   (int)cond_sum_d + count_gt + count_le + sum_all + 
                   (int)max_f + (int)min_d;
    
    /* Store to volatile to prevent elimination */
    g_result_int = checksum;
    g_result_float = cond_sum_f + max_f;
    
    printf("Checksum: %d\n", checksum);
    printf("Results: max_val=%d, min_val=%d, max_us=%u, max_f=%.2f\n",
           max_val, min_val, max_us, max_f);
    
    return 0;
}
