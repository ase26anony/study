#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_seed;
volatile int g_result_store[8];

/* Function to generate deterministic but non-constant data */
static inline int gen_value(int i, int seed) {
    return (i * 3 + seed) ^ (seed >> 2);
}

static inline float gen_float(int i, int seed) {
    return (float)((i * 7 + seed * 3) % 100) / 10.0f;
}

static inline double gen_double(int i, int seed) {
    return (double)((i * 11 + seed * 5) % 200) / 20.0;
}

/* Test function with multiple conditional reduction patterns */
void test_conditional_reductions(int seed, int threshold_int, 
                                 float threshold_float, 
                                 double threshold_double,
                                 unsigned short threshold_ushort) {
    int arr_int[64];
    float arr_float[64];
    double arr_double[64];
    unsigned short arr_ushort[64];
    
    /* Initialize arrays with deterministic but non-constant values */
    for (int i = 0; i < 64; i++) {
        arr_int[i] = gen_value(i, seed);
        arr_float[i] = gen_float(i, seed);
        arr_double[i] = gen_double(i, seed);
        arr_ushort[i] = (unsigned short)(gen_value(i, seed) & 0xFFFF);
    }
    
    /* Reduction variables - initialized with volatile read */
    int max_int = g_volatile_seed;
    int min_int = g_volatile_seed;
    float max_float = (float)g_volatile_seed;
    double min_double = (double)g_volatile_seed;
    unsigned short max_ushort = (unsigned short)g_volatile_seed;
    int cond_sum_int = 0;
    int cond_count = 0;
    float cond_sum_float = 0.0f;
    double cond_sum_double = 0.0;
    
    /* Loop 1: GT_EXPR pattern - find maximum with > comparison */
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (arr_int[i] > 0) {
            /* Conditional reduction with > */
            if (arr_int[i] > max_int) {
                max_int = arr_int[i];
            }
            
            /* Additional reduction with different condition */
            if (arr_int[i] < min_int) {
                min_int = arr_int[i];
            }
        }
    }
    
    /* Loop 2: GE_EXPR pattern - conditional sum with >= */
    /* Use while loop variant */
    int j = 0;
    while (j < 64) {
        /* Combine conditions with logical AND */
        if (j % 2 == 0 && arr_float[j] >= threshold_float) {
            cond_sum_float += arr_float[j];
        }
        
        /* Nested conditional for GE_EXPR on different type */
        if (arr_int[j] >= threshold_int) {
            cond_sum_int += arr_int[j];
            /* Count elements satisfying condition */
            cond_count++;
        }
        j++;
    }
    
    /* Loop 3: LT_EXPR pattern - find minimum with < comparison */
    /* Multiple reductions in one loop */
    double local_min_double = min_double;
    unsigned short local_max_ushort = max_ushort;
    
    for (int i = 0; i < 64; i++) {
        /* LT_EXPR conditional reduction */
        if (arr_double[i] < local_min_double) {
            local_min_double = arr_double[i];
        }
        
        /* Additional reduction with different comparison */
        if (arr_ushort[i] > local_max_ushort) {
            local_max_ushort = arr_ushort[i];
        }
        
        /* Conditional sum with LT_EXPR */
        if (arr_int[i] < threshold_int) {
            cond_sum_double += (double)arr_int[i];
        }
    }
    min_double = local_min_double;
    max_ushort = local_max_ushort;
    
    /* Loop 4: LE_EXPR pattern - mixed comparisons */
    int sum_le_int = 0;
    float sum_le_float = 0.0f;
    
    for (int i = 0; i < 64; i++) {
        /* LE_EXPR pattern with logical OR combination */
        if (i < 32 || arr_int[i] <= threshold_int) {
            sum_le_int += arr_int[i];
        }
        
        /* Nested conditionals with LE_EXPR */
        if (arr_float[i] > 0.0f) {
            if (arr_float[i] <= threshold_float * 2.0f) {
                sum_le_float += arr_float[i];
            }
        }
    }
    
    /* Loop 5: Mixed comparison operators in one loop */
    int mixed_max = max_int;
    int mixed_min = min_int;
    int mixed_sum = 0;
    
    for (int i = 0; i < 64; i++) {
        /* GT_EXPR */
        if (arr_int[i] > mixed_max) {
            mixed_max = arr_int[i];
        }
        
        /* LT_EXPR */
        if (arr_int[i] < mixed_min) {
            mixed_min = arr_int[i];
        }
        
        /* GE_EXPR */
        if (arr_int[i] >= threshold_int - 10) {
            mixed_sum += arr_int[i];
        }
        
        /* LE_EXPR with additional condition */
        if (i % 3 == 0 && arr_int[i] <= threshold_int + 10) {
            mixed_sum -= arr_int[i] / 2;
        }
    }
    
    /* Store results to volatile array to prevent elimination */
    g_result_store[0] = max_int;
    g_result_store[1] = min_int;
    g_result_store[2] = cond_sum_int;
    g_result_store[3] = cond_count;
    g_result_store[4] = (int)cond_sum_float;
    g_result_store[5] = (int)cond_sum_double;
    g_result_store[6] = sum_le_int;
    g_result_store[7] = (int)sum_le_float;
    
    /* Also update the volatile seed */
    g_volatile_seed = mixed_max + mixed_min + mixed_sum;
}

int main(int argc, char *argv[]) {
    int seed = 42;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Make thresholds loop-invariant but not compile-time constants */
    volatile int vol_threshold_int = seed * 2;
    volatile float vol_threshold_float = (float)seed / 3.0f;
    volatile double vol_threshold_double = (double)seed / 4.0;
    volatile unsigned short vol_threshold_ushort = (unsigned short)(seed * 5);
    
    int threshold_int = vol_threshold_int;
    float threshold_float = vol_threshold_float;
    double threshold_double = vol_threshold_double;
    unsigned short threshold_ushort = vol_threshold_ushort;
    
    /* Initialize volatile seed */
    g_volatile_seed = seed;
    
    /* Run the test */
    test_conditional_reductions(seed, threshold_int, threshold_float,
                               threshold_double, threshold_ushort);
    
    /* Compute checksum from results */
    int checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += g_result_store[i];
    }
    checksum += g_volatile_seed;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
