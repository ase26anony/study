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
    return v % 100 + 50; /* Make it non-constant */
}

float get_float_threshold(int seed) {
    volatile float v = seed * 0.7f;
    return v + 25.0f;
}

double get_double_threshold(int seed) {
    volatile double v = seed * 0.3;
    return v + 15.0;
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
        arr_i[i] = (i * 3 + seed) % 200;
        arr_us[i] = (unsigned short)((i * 5 + seed * 2) % 65535);
        arr_f[i] = (float)((i * 7 + seed * 3) % 100) * 0.5f;
        arr_d[i] = (double)((i * 11 + seed * 5) % 150) * 0.3;
    }
    
    /* Get loop-invariant thresholds */
    int thresh_i = get_threshold(seed);
    float thresh_f = get_float_threshold(seed);
    double thresh_d = get_double_threshold(seed);
    
    /* Reduction variables */
    int max_val = arr_i[0];
    int min_val = arr_i[0];
    int cond_sum_gt = 0;
    int cond_sum_ge = 0;
    float f_max = arr_f[0];
    float f_min = arr_f[0];
    double d_sum_lt = 0.0;
    unsigned short us_max = arr_us[0];
    int count_le = 0;
    
    /* Pattern 1: GT_EXPR (>)
       Conditional max reduction with outer if */
    for (int i = 0; i < 64; i++) {
        /* Outer conditional to complicate control flow */
        if (arr_i[i] > 0) {
            if (arr_i[i] > max_val) {
                max_val = arr_i[i];
            }
        }
    }
    
    /* Pattern 2: GE_EXPR (>=)
       Conditional sum with logical AND */
    for (int i = 0; i < 64; i++) {
        if (arr_i[i] >= thresh_i && arr_i[i] < 180) {
            cond_sum_ge += arr_i[i];
        }
    }
    
    /* Pattern 3: LT_EXPR (<)
       Multiple reductions in one loop */
    int i = 0;
    while (i < 64) {
        /* Find min and sum values less than threshold */
        if (arr_i[i] < thresh_i) {
            if (arr_i[i] < min_val) {
                min_val = arr_i[i];
            }
            cond_sum_gt += arr_i[i]; /* Reusing for different condition */
        }
        i++;
    }
    
    /* Pattern 4: LE_EXPR (<=)
       With floating point and type mixing */
    for (int i = 0; i < 64; i++) {
        /* Combined condition with OR */
        if (arr_f[i] <= thresh_f || i % 3 == 0) {
            if (arr_f[i] > f_max) {
                f_max = arr_f[i];
            }
        }
        
        /* Another condition in same loop */
        if (arr_f[i] <= thresh_f * 0.8f) {
            if (arr_f[i] < f_min) {
                f_min = arr_f[i];
            }
        }
    }
    
    /* Pattern 5: Mixed comparisons with unsigned short */
    for (int i = 0; i < 64; i++) {
        /* GT with unsigned */
        if (arr_us[i] > (unsigned short)thresh_i) {
            if (arr_us[i] > us_max) {
                us_max = arr_us[i];
            }
        }
        
        /* LE with unsigned */
        if (arr_us[i] <= (unsigned short)(thresh_i * 2)) {
            count_le++;
        }
    }
    
    /* Pattern 6: Double precision with LT and GE */
    for (int i = 0; i < 64; i++) {
        /* LT with double */
        if (arr_d[i] < thresh_d) {
            d_sum_lt += arr_d[i];
        }
        
        /* GE with double in same loop */
        if (arr_d[i] >= thresh_d * 0.5) {
            g_result_double += arr_d[i];
        }
    }
    
    /* Pattern 7: Nested complex condition with multiple operators */
    int complex_sum = 0;
    int complex_max = arr_i[0];
    for (int i = 0; i < 64; i++) {
        /* Complex condition: (x > thresh/2) && (x <= thresh*1.5) */
        if (arr_i[i] > thresh_i / 2) {
            if (arr_i[i] <= thresh_i + thresh_i / 2) {
                complex_sum += arr_i[i];
                if (arr_i[i] > complex_max) {
                    complex_max = arr_i[i];
                }
            }
        }
    }
    
    /* Compute checksum from all results */
    int checksum = max_val + min_val + cond_sum_gt + cond_sum_ge;
    checksum += (int)f_max + (int)f_min + (int)d_sum_lt;
    checksum += us_max + count_le + complex_sum + complex_max;
    checksum += (int)g_result_double;
    
    /* Store to volatile to prevent elimination */
    g_result_int = checksum;
    g_result_float = f_max;
    
    printf("Checksum: %d\n", checksum);
    printf("Results: max=%d, min=%d, sum_ge=%d, sum_lt=%d\n", 
           max_val, min_val, cond_sum_ge, (int)d_sum_lt);
    printf("Float: max=%.2f, min=%.2f\n", f_max, f_min);
    printf("UShort: max=%u, count_le=%d\n", us_max, count_le);
    
    return 0;
}
