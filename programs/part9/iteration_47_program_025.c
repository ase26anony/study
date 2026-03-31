#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile globals to prevent dead code elimination */
volatile int g_result_int = 0;
volatile float g_result_float = 0.0f;

/* Function to generate deterministic data */
static inline int gen_val(int i, int seed) {
    return (i * 3 + seed) & 0xFF;  /* Keep values in range */
}

static inline float gen_fval(int i, int seed) {
    return (float)((i * 7 + seed) & 0xFF) * 0.5f;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    
    /* Arrays with different types */
    int arr_i[64];
    unsigned short arr_us[64];
    float arr_f[64];
    double arr_d[64];
    
    /* Initialize arrays deterministically */
    for (int i = 0; i < 64; i++) {
        arr_i[i] = gen_val(i, seed);
        arr_us[i] = (unsigned short)gen_val(i, seed + 1);
        arr_f[i] = gen_fval(i, seed + 2);
        arr_d[i] = (double)gen_fval(i, seed + 3);
    }
    
    /* Loop-invariant thresholds from volatile sources */
    volatile int thresh_int = seed + 50;
    volatile float thresh_float = (float)(seed + 25) * 0.8f;
    volatile unsigned short thresh_ushort = (seed + 30) & 0xFFFF;
    volatile double thresh_double = (double)(seed + 40) * 0.6;
    
    int max_val, min_val, cond_sum_int, count_above;
    float max_f, min_f, cond_sum_f;
    unsigned short max_us, min_us;
    double max_d, min_d;
    
    /* Initialize reduction variables with volatile reads */
    volatile int init_max = arr_i[0];
    volatile int init_min = arr_i[0];
    max_val = init_max;
    min_val = init_min;
    cond_sum_int = 0;
    count_above = 0;
    
    /* ====== LOOP 1: GT_EXPR (greater-than) conditional reductions ====== */
    /* Multiple reductions in one loop with GT_EXPR */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (arr_i[i] > 0) {
            /* GT_EXPR for max reduction */
            if (arr_i[i] > max_val) {
                max_val = arr_i[i];
            }
            
            /* GT_EXPR with different threshold */
            if (arr_i[i] > thresh_int) {
                cond_sum_int += arr_i[i];
                count_above++;
            }
            
            /* Combined condition with logical AND */
            if (arr_i[i] > (thresh_int - 10) && arr_i[i] < (thresh_int + 10)) {
                /* Additional operation to prevent trivial loop */
                min_val = (arr_i[i] < min_val) ? arr_i[i] : min_val;
            }
        }
    }
    
    /* ====== LOOP 2: GE_EXPR (greater-or-equal) with float ====== */
    /* Initialize float reductions */
    volatile float init_max_f = arr_f[0];
    volatile float init_min_f = arr_f[0];
    max_f = init_max_f;
    min_f = init_min_f;
    cond_sum_f = 0.0f;
    
    /* While loop variant */
    int j = 0;
    while (j < 64) {
        /* GE_EXPR for float max reduction */
        if (arr_f[j] >= max_f) {
            max_f = arr_f[j];
        }
        
        /* GE_EXPR conditional sum */
        if (arr_f[j] >= thresh_float) {
            cond_sum_f += arr_f[j];
        }
        
        /* Nested conditional */
        if (j % 2 == 0) {
            if (arr_f[j] >= (thresh_float - 5.0f)) {
                /* Update min with LT_EXPR (will be swapped in transformation) */
                min_f = (arr_f[j] < min_f) ? arr_f[j] : min_f;
            }
        }
        j++;
    }
    
    /* ====== LOOP 3: LT_EXPR (less-than) with unsigned short ====== */
    /* Initialize unsigned short reductions */
    volatile unsigned short init_max_us = arr_us[0];
    volatile unsigned short init_min_us = arr_us[0];
    max_us = init_max_us;
    min_us = init_min_us;
    
    for (int i = 0; i < 64; i++) {
        /* LT_EXPR for min reduction */
        if (arr_us[i] < min_us) {
            min_us = arr_us[i];
        }
        
        /* LT_EXPR with threshold */
        if (arr_us[i] < thresh_ushort) {
            /* Count values below threshold */
            count_above--;  /* Reusing variable for different purpose */
        }
        
        /* Logical OR combined condition */
        if (arr_us[i] < (thresh_ushort >> 1) || i % 3 == 0) {
            if (arr_us[i] < max_us) {
                /* This won't change max_us often but tests the pattern */
                max_us = (arr_us[i] > max_us) ? arr_us[i] : max_us;
            }
        }
    }
    
    /* ====== LOOP 4: LE_EXPR (less-or-equal) with double ====== */
    /* Initialize double reductions */
    volatile double init_max_d = arr_d[0];
    volatile double init_min_d = arr_d[0];
    max_d = init_max_d;
    min_d = init_min_d;
    
    for (int i = 0; i < 64; i++) {
        /* LE_EXPR for min reduction */
        if (arr_d[i] <= min_d) {
            min_d = arr_d[i];
        }
        
        /* LE_EXPR conditional operation */
        if (arr_d[i] <= thresh_double) {
            /* Update max with GE_EXPR */
            max_d = (arr_d[i] >= max_d) ? arr_d[i] : max_d;
        }
        
        /* Complex nested condition */
        if (i > 16 && i < 48) {
            if (arr_d[i] <= (thresh_double * 1.5)) {
                /* Additional operation */
                cond_sum_f += (float)arr_d[i];
            }
        }
    }
    
    /* ====== LOOP 5: Mixed comparisons in single loop ====== */
    int mixed_max = arr_i[0];
    int mixed_min = arr_i[0];
    int mixed_sum_gt = 0;
    int mixed_sum_lt = 0;
    
    for (int i = 0; i < 64; i++) {
        /* GT_EXPR */
        if (arr_i[i] > mixed_max) {
            mixed_max = arr_i[i];
        }
        
        /* LT_EXPR */
        if (arr_i[i] < mixed_min) {
            mixed_min = arr_i[i];
        }
        
        /* GE_EXPR */
        if (arr_i[i] >= (thresh_int - 20)) {
            mixed_sum_gt += arr_i[i];
        }
        
        /* LE_EXPR */
        if (arr_i[i] <= (thresh_int + 20)) {
            mixed_sum_lt += arr_i[i];
        }
    }
    
    /* Aggregate results into checksum */
    uint64_t checksum = 0;
    checksum += (uint64_t)max_val;
    checksum += (uint64_t)min_val;
    checksum += (uint64_t)cond_sum_int;
    checksum += (uint64_t)count_above;
    checksum += (uint64_t)max_us;
    checksum += (uint64_t)min_us;
    checksum += (uint64_t)(max_f * 100.0f);
    checksum += (uint64_t)(min_f * 100.0f);
    checksum += (uint64_t)(cond_sum_f * 50.0f);
    checksum += (uint64_t)(max_d * 75.0);
    checksum += (uint64_t)(min_d * 75.0);
    checksum += (uint64_t)mixed_max;
    checksum += (uint64_t)mixed_min;
    checksum += (uint64_t)mixed_sum_gt;
    checksum += (uint64_t)mixed_sum_lt;
    
    /* Store to volatile to prevent elimination */
    g_result_int = (int)checksum;
    g_result_float = (float)checksum;
    
    printf("Checksum: %lu\n", checksum);
    
    return 0;
}
