#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile globals to prevent dead code elimination */
volatile int g_result_int = 0;
volatile float g_result_float = 0.0f;

/* Function to generate deterministic data */
static inline int gen_val(int i, int seed) {
    return (i * 3 + seed) % 1000;
}

static inline float gen_float(int i, int seed) {
    return (float)((i * 7 + seed * 3) % 1000) * 0.1f;
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
    
    /* Initialize arrays deterministically */
    for (int i = 0; i < 64; i++) {
        arr_i[i] = gen_val(i, seed);
        arr_us[i] = (unsigned short)(gen_val(i, seed + 1) % 65535);
        arr_f[i] = gen_float(i, seed);
        arr_d[i] = (double)gen_float(i, seed + 2);
    }
    
    /* Loop-invariant thresholds from volatile sources */
    volatile int thresh_int = seed + 500;
    volatile float thresh_float = (float)(seed % 100) * 0.5f + 25.0f;
    volatile unsigned short thresh_us = (unsigned short)((seed * 13) % 30000 + 10000);
    volatile double thresh_double = (double)(seed % 200) * 0.25 + 50.0;
    
    /* Reduction variables */
    int max_val_i = -10000;  /* GT_EXPR pattern */
    int min_val_i = 10000;   /* LT_EXPR pattern */
    unsigned short max_val_us = 0;  /* GE_EXPR pattern */
    float cond_sum_f = 0.0f; /* LE_EXPR pattern */
    double max_val_d = -1e9; /* Mixed comparisons */
    int count_gt = 0, count_lt = 0;
    
    /* ===== Loop 1: GT_EXPR and LT_EXPR with multiple reductions ===== */
    /* This loop tests GT_EXPR (>) and LT_EXPR (<) patterns */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (arr_i[i] != 0) {
            /* GT_EXPR pattern for maximum */
            if (arr_i[i] > max_val_i) {
                max_val_i = arr_i[i];
            }
            
            /* LT_EXPR pattern for minimum with swapped operands */
            if (min_val_i < arr_i[i]) {  /* Equivalent to arr_i[i] > min_val_i */
                /* This will be transformed to use BIT_NOT_EXPR and BIT_AND_EXPR */
                min_val_i = arr_i[i];
            }
            
            /* Combined condition with logical AND */
            if (arr_i[i] > (int)thresh_int && i % 2 == 0) {
                count_gt++;
            }
        }
    }
    
    /* ===== Loop 2: GE_EXPR pattern with unsigned short ===== */
    /* This loop tests GE_EXPR (>=) pattern */
    int j = 0;
    while (j < 64) {
        /* GE_EXPR pattern - will use BIT_NOT_EXPR and BIT_IOR_EXPR */
        if (arr_us[j] >= max_val_us) {
            max_val_us = arr_us[j];
        }
        
        /* Additional condition with logical OR */
        if (arr_us[j] >= thresh_us || j < 32) {
            count_lt++;  /* Reusing count_lt for different purpose */
        }
        j++;
    }
    
    /* ===== Loop 3: LE_EXPR pattern with float ===== */
    /* This loop tests LE_EXPR (<=) pattern */
    for (int i = 0; i < 64; i++) {
        /* Nested conditionals */
        if (i > 10) {
            if (i < 50) {
                /* LE_EXPR pattern - will swap operands and use BIT_NOT_EXPR and BIT_IOR_EXPR */
                if (arr_f[i] <= thresh_float) {
                    cond_sum_f += arr_f[i];
                } else {
                    /* Alternative path to ensure both branches exist */
                    cond_sum_f += 1.0f;
                }
            }
        }
    }
    
    /* ===== Loop 4: Mixed comparisons with double ===== */
    /* This loop tests all comparison types together */
    double sum_d = 0.0;
    int limit = 64;
    for (int k = 0; k < limit; k++) {
        /* GT_EXPR */
        if (arr_d[k] > max_val_d) {
            max_val_d = arr_d[k];
        }
        
        /* GE_EXPR with different threshold */
        if (arr_d[k] >= (thresh_double - 10.0)) {
            sum_d += arr_d[k];
        }
        
        /* LT_EXPR with swapped pattern */
        if ((thresh_double + 20.0) < arr_d[k]) {  /* Equivalent to arr_d[k] > thresh_double + 20.0 */
            count_gt += 2;
        }
        
        /* LE_EXPR */
        if (arr_d[k] <= (thresh_double + 40.0)) {
            count_lt += 3;
        }
    }
    
    /* ===== Loop 5: Complex nested conditionals ===== */
    /* Tests pattern recognition in complex control flow */
    int complex_sum = 0;
    int complex_max = -10000;
    for (int i = 0; i < 64; i++) {
        /* Outer condition */
        if (arr_i[i] % 3 == seed % 3) {
            /* Inner condition with GT_EXPR */
            if (arr_i[i] > complex_max) {
                complex_max = arr_i[i];
            }
            
            /* Another condition with LE_EXPR */
            if (arr_i[i] <= (thresh_int + 100)) {
                complex_sum += arr_i[i];
            }
        } else if (arr_i[i] % 5 == seed % 5) {
            /* Alternative branch with LT_EXPR */
            if (complex_max < arr_i[i]) {  /* Swapped operands */
                complex_max = arr_i[i];
            }
        }
    }
    
    /* Aggregate results into checksum */
    int checksum = 0;
    checksum += max_val_i;
    checksum += min_val_i;
    checksum += (int)max_val_us;
    checksum += (int)cond_sum_f;
    checksum += (int)max_val_d;
    checksum += (int)sum_d;
    checksum += count_gt;
    checksum += count_lt;
    checksum += complex_sum;
    checksum += complex_max;
    
    /* Store to volatile to prevent optimization */
    g_result_int = checksum;
    g_result_float = cond_sum_f;
    
    printf("Checksum: %d\n", checksum);
    printf("Float sum: %f\n", cond_sum_f);
    printf("Max int: %d, Min int: %d\n", max_val_i, min_val_i);
    printf("Max ushort: %u\n", max_val_us);
    printf("Max double: %f\n", max_val_d);
    
    return 0;
}
