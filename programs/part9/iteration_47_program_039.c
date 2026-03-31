#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile globals to prevent dead code elimination */
volatile int g_result_int = 0;
volatile float g_result_float = 0.0f;

/* Function to create loop-invariant thresholds */
int get_threshold(int seed) {
    volatile int v = seed;
    return v % 100 + 50;  /* Returns 50-149 depending on seed */
}

float get_float_threshold(int seed) {
    volatile float v = seed * 0.7f;
    return v + 25.0f;  /* Make it loop invariant but not constant */
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
        arr_us[i] = (unsigned short)((i * 5 + seed * 2) % 65535);
        arr_f[i] = (float)((i * 7 + seed * 3) % 1000) * 0.1f;
        arr_d[i] = (double)((i * 11 + seed * 5) % 2000) * 0.05;
    }
    
    /* Loop-invariant thresholds from volatile sources */
    int thresh_int = get_threshold(seed);
    float thresh_float = get_float_threshold(seed);
    unsigned short thresh_ushort = (unsigned short)(seed % 30000 + 10000);
    double thresh_double = (double)(seed % 500) * 0.3 + 10.0;
    
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
    
    /* ===== TEST CASE 1: GT_EXPR (greater-than) ===== */
    /* Find maximum with conditional: if (arr_i[i] > current_max) */
    for (int i = 0; i < 64; i++) {
        /* Nested conditional to complicate control flow */
        if (i % 2 == 0) {
            if (arr_i[i] > max_val_int) {
                max_val_int = arr_i[i];
            }
        } else {
            /* Alternative path that doesn't affect the reduction */
            volatile int dummy = arr_i[i];
        }
    }
    
    /* ===== TEST CASE 2: GE_EXPR (greater-than-or-equal) ===== */
    /* Sum values >= threshold and count them */
    for (int i = 0; i < 64; i++) {
        /* Combined with logical OR in outer condition */
        if (i < 60 || arr_f[i] >= thresh_float) {
            if (arr_f[i] >= thresh_float) {
                cond_sum_float += arr_f[i];
                count_ge++;
            }
        }
    }
    
    /* ===== TEST CASE 3: LT_EXPR (less-than) ===== */
    /* Find minimum with conditional: if (arr_i[i] < current_min) */
    /* Using while loop instead of for loop */
    int w = 0;
    while (w < 64) {
        if (arr_i[w] < min_val_int) {
            min_val_int = arr_i[w];
        }
        w++;
    }
    
    /* ===== TEST CASE 4: LE_EXPR (less-than-or-equal) ===== */
    /* Multiple reductions in one loop with different conditions */
    for (int i = 0; i < 64; i++) {
        /* First reduction: sum if <= threshold */
        if (arr_d[i] <= thresh_double) {
            cond_sum_double += arr_d[i];
            count_le++;
        }
        
        /* Second reduction: find min float with additional condition */
        if (i > 10 && i < 50) {
            if (arr_f[i] < min_val_float) {
                min_val_float = arr_f[i];
            }
        }
    }
    
    /* ===== TEST CASE 5: Mixed types and operators ===== */
    /* Multiple conditional reductions in one loop */
    int temp_max = -1000;
    int temp_min = 1000;
    int sum_gt = 0;
    int sum_lt = 0;
    
    for (int i = 0; i < 64; i++) {
        /* GT_EXPR reduction */
        if (arr_i[i] > temp_max) {
            temp_max = arr_i[i];
        }
        
        /* LT_EXPR reduction */
        if (arr_i[i] < temp_min) {
            temp_min = arr_i[i];
        }
        
        /* GE_EXPR conditional sum */
        if (arr_i[i] >= thresh_int) {
            sum_gt += arr_i[i];
        }
        
        /* LE_EXPR conditional sum with unsigned short */
        if (arr_us[i] <= thresh_ushort) {
            sum_lt += (int)arr_us[i];
        }
    }
    
    /* ===== TEST CASE 6: Nested conditionals with logical operators ===== */
    float complex_max = -1000.0f;
    for (int i = 0; i < 64; i++) {
        /* Complex condition using logical AND */
        if (i % 3 == 0 && arr_f[i] > complex_max) {
            complex_max = arr_f[i];
        }
        
        /* Another condition with logical OR */
        if (i < 10 || i > 50) {
            if (arr_f[i] < min_val_float) {
                min_val_float = arr_f[i];
            }
        }
    }
    
    /* Aggregate results into checksum */
    int checksum = 0;
    checksum += max_val_int;
    checksum += min_val_int;
    checksum += (int)max_val_float;
    checksum += (int)min_val_float;
    checksum += (int)cond_sum_double;
    checksum += count_ge;
    checksum += count_le;
    checksum += (int)cond_sum_float;
    checksum += temp_max;
    checksum += temp_min;
    checksum += sum_gt;
    checksum += sum_lt;
    checksum += (int)complex_max;
    
    /* Store to volatile to prevent elimination */
    g_result_int = checksum;
    g_result_float = cond_sum_float;
    
    printf("Checksum: %d\n", checksum);
    printf("Max int: %d, Min int: %d\n", max_val_int, min_val_int);
    printf("Float sum >= threshold: %.2f, Count: %u\n", cond_sum_float, count_ge);
    printf("Double sum <= threshold: %.2f, Count: %u\n", cond_sum_double, count_le);
    
    return 0;
}
