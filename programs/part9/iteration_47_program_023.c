#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_seed = 0;
volatile int g_volatile_result = 0;

/* Function to generate deterministic but non-constant data */
static inline int gen_data(int i, int seed) {
    return (i * 3 + seed) ^ (seed >> 2);
}

static inline float gen_float_data(int i, int seed) {
    return (float)((i * 7 + seed * 3) % 100) / 10.0f;
}

static inline double gen_double_data(int i, int seed) {
    return (double)((i * 11 + seed * 5) % 200) / 20.0;
}

/* Test function with various conditional reduction patterns */
void test_conditional_reductions(int seed) {
    int arr_int[64];
    unsigned short arr_ushort[64];
    float arr_float[64];
    double arr_double[64];
    
    /* Initialize arrays with deterministic but non-constant data */
    for (int i = 0; i < 64; i++) {
        arr_int[i] = gen_data(i, seed);
        arr_ushort[i] = (unsigned short)(gen_data(i, seed) & 0xFFFF);
        arr_float[i] = gen_float_data(i, seed);
        arr_double[i] = gen_double_data(i, seed);
    }
    
    /* Loop-invariant thresholds from volatile source */
    volatile int volatile_threshold = g_volatile_seed + 50;
    int threshold_int = volatile_threshold;
    float threshold_float = (float)volatile_threshold / 2.0f;
    double threshold_double = (double)volatile_threshold / 3.0;
    
    /* Reduction variables - initialize with volatile to prevent constant propagation */
    volatile int volatile_init = g_volatile_seed;
    
    /* TEST 1: GT_EXPR (greater-than) conditional reduction */
    int max_gt = volatile_init;  /* Start with volatile value */
    int count_gt = 0;
    for (int i = 0; i < 64; i++) {
        /* Outer if to complicate control flow */
        if (arr_int[i] > 0) {
            /* Conditional reduction with GT_EXPR */
            if (arr_int[i] > max_gt) {
                max_gt = arr_int[i];
            }
            /* Additional reduction with different condition */
            if (arr_int[i] > threshold_int && arr_int[i] < 1000) {
                count_gt++;
            }
        }
    }
    
    /* TEST 2: GE_EXPR (greater-than-or-equal) conditional reduction */
    float sum_ge = (float)volatile_init;
    float max_ge = (float)volatile_init;
    for (int i = 0; i < 64; i++) {
        /* Combined condition with logical OR */
        if (arr_float[i] >= threshold_float || i % 3 == 0) {
            /* Conditional sum with GE_EXPR */
            if (arr_float[i] >= 0.0f) {
                sum_ge += arr_float[i];
            }
            /* Nested conditional max with GE_EXPR */
            if (arr_float[i] >= max_ge && arr_float[i] < 100.0f) {
                max_ge = arr_float[i];
            }
        }
    }
    
    /* TEST 3: LT_EXPR (less-than) conditional reduction - using while loop */
    double min_lt = (double)volatile_init;
    int idx = 0;
    while (idx < 64) {
        /* Multiple reductions in same loop */
        if (arr_double[idx] < threshold_double) {
            /* Conditional min with LT_EXPR */
            if (arr_double[idx] < min_lt) {
                min_lt = arr_double[idx];
            }
        }
        
        /* Additional reduction with different data type */
        if (arr_ushort[idx] < (unsigned short)threshold_int) {
            /* This should also trigger LT_EXPR transformation */
            if (arr_ushort[idx] < (unsigned short)min_lt) {
                /* Type conversion in comparison */
            }
        }
        idx++;
    }
    
    /* TEST 4: LE_EXPR (less-than-or-equal) conditional reduction */
    int min_le = volatile_init;
    int sum_le = 0;
    for (int i = 0; i < 64; i++) {
        /* Complex condition with logical AND */
        if (arr_int[i] <= threshold_int && arr_int[i] > -1000) {
            /* Conditional min with LE_EXPR */
            if (arr_int[i] <= min_le) {
                min_le = arr_int[i];
            }
            /* Conditional sum in same loop */
            sum_le += arr_int[i];
        }
        
        /* Additional floating-point reduction with LE_EXPR */
        if (i % 2 == 0 && arr_float[i] <= threshold_float) {
            if (arr_float[i] <= max_ge) {
                /* Another conditional update */
            }
        }
    }
    
    /* TEST 5: Mixed reductions in single loop with all comparison types */
    int mixed_max = volatile_init;
    int mixed_min = volatile_init;
    float mixed_sum = 0.0f;
    int mixed_count = 0;
    
    for (int i = 0; i < 64; i++) {
        /* GT_EXPR reduction */
        if (arr_int[i] > mixed_max && arr_int[i] < 500) {
            mixed_max = arr_int[i];
        }
        
        /* LT_EXPR reduction */
        if (arr_int[i] < mixed_min || i % 4 == 0) {
            mixed_min = arr_int[i];
        }
        
        /* GE_EXPR conditional sum */
        if (arr_float[i] >= 2.0f) {
            mixed_sum += arr_float[i];
        }
        
        /* LE_EXPR conditional count */
        if (arr_ushort[i] <= (unsigned short)(threshold_int & 0xFF)) {
            mixed_count++;
        }
    }
    
    /* Aggregate results into checksum */
    int checksum = 0;
    checksum += max_gt;
    checksum += count_gt;
    checksum += (int)sum_ge;
    checksum += (int)(max_ge * 100);
    checksum += (int)(min_lt * 1000);
    checksum += min_le;
    checksum += sum_le;
    checksum += mixed_max;
    checksum += mixed_min;
    checksum += (int)mixed_sum;
    checksum += mixed_count;
    
    /* Store to volatile to prevent dead code elimination */
    g_volatile_result = checksum;
}

int main(int argc, char *argv[]) {
    int seed = 42;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    g_volatile_seed = seed;
    
    /* Run the test multiple times with different seeds */
    for (int run = 0; run < 3; run++) {
        test_conditional_reductions(seed + run * 100);
    }
    
    printf("Checksum: %d\n", g_volatile_result);
    
    return 0;
}
