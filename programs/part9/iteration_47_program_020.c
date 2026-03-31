#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile globals to prevent dead code elimination */
volatile int g_result_int = 0;
volatile float g_result_float = 0.0f;
volatile double g_result_double = 0.0;

/* Function to generate deterministic data */
static inline int gen_data(int i, int seed) {
    return (i * 3 + seed) % 1000;
}

static inline float gen_float_data(int i, int seed) {
    return (float)((i * 7 + seed * 3) % 1000) / 10.0f;
}

static inline double gen_double_data(int i, int seed) {
    return (double)((i * 11 + seed * 5) % 1000) / 20.0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    
    /* Declare arrays with different types */
    int arr_int[64];
    unsigned short arr_ushort[64];
    float arr_float[64];
    double arr_double[64];
    
    /* Initialize arrays deterministically */
    for (int i = 0; i < 64; i++) {
        arr_int[i] = gen_data(i, seed);
        arr_ushort[i] = (unsigned short)(gen_data(i, seed + 1) % 65535);
        arr_float[i] = gen_float_data(i, seed);
        arr_double[i] = gen_double_data(i, seed);
    }
    
    /* Loop-invariant thresholds from volatile sources */
    volatile int thresh_int = seed + 500;
    volatile unsigned short thresh_ushort = (seed * 3) % 65535;
    volatile float thresh_float = (float)(seed % 100) + 25.5f;
    volatile double thresh_double = (double)(seed % 200) + 50.5;
    
    int threshold_int = thresh_int;
    unsigned short threshold_ushort = thresh_ushort;
    float threshold_float = thresh_float;
    double threshold_double = thresh_double;
    
    /* TEST 1: GT_EXPR (greater-than) conditional reduction */
    {
        int max_val = arr_int[0];
        float max_float = arr_float[0];
        
        /* Loop with GT_EXPR conditional max reduction */
        for (int i = 0; i < 64; i++) {
            /* Outer if to complicate control flow */
            if (i % 2 == 0) {
                /* GT_EXPR pattern for integers */
                if (arr_int[i] > max_val) {
                    max_val = arr_int[i];
                }
                
                /* GT_EXPR pattern for floats */
                if (arr_float[i] > max_float) {
                    max_float = arr_float[i];
                }
            }
        }
        
        g_result_int = max_val;
        g_result_float = max_float;
    }
    
    /* TEST 2: GE_EXPR (greater-than-or-equal) conditional sum */
    {
        int sum_ge = 0;
        double sum_double_ge = 0.0;
        
        /* Loop with GE_EXPR conditional sum reduction */
        int i = 0;
        while (i < 64) {
            /* Combined condition with logical AND */
            if (i < 60 && arr_int[i] >= threshold_int) {
                sum_ge += arr_int[i];
            }
            
            /* GE_EXPR for doubles */
            if (arr_double[i] >= threshold_double) {
                sum_double_ge += arr_double[i];
            }
            i++;
        }
        
        g_result_int += sum_ge;
        g_result_double = sum_double_ge;
    }
    
    /* TEST 3: LT_EXPR (less-than) conditional min reduction */
    {
        int min_val = arr_int[0];
        float min_float = arr_float[0];
        
        /* Loop with LT_EXPR conditional min reduction */
        for (int i = 0; i < 64; i++) {
            /* LT_EXPR pattern for integers */
            if (arr_int[i] < min_val) {
                min_val = arr_int[i];
            }
            
            /* Nested if with logical OR */
            if (i % 3 == 0 || i % 5 == 0) {
                if (arr_float[i] < min_float) {
                    min_float = arr_float[i];
                }
            }
        }
        
        g_result_int += min_val;
        g_result_float += min_float;
    }
    
    /* TEST 4: LE_EXPR (less-than-or-equal) conditional reduction */
    {
        unsigned short min_ushort = arr_ushort[0];
        int count_le = 0;
        
        /* Loop with LE_EXPR conditional reduction */
        for (int i = 0; i < 64; i++) {
            /* LE_EXPR pattern for unsigned short */
            if (arr_ushort[i] <= min_ushort) {
                min_ushort = arr_ushort[i];
            }
            
            /* LE_EXPR for counting elements */
            if (arr_int[i] <= threshold_int) {
                count_le++;
            }
        }
        
        g_result_int += min_ushort + count_le;
    }
    
    /* TEST 5: Multiple reductions in one loop with different comparison operators */
    {
        int multi_max = arr_int[0];      /* GT_EXPR */
        int multi_min = arr_int[0];      /* LT_EXPR */
        int multi_sum_ge = 0;            /* GE_EXPR */
        int multi_count_le = 0;          /* LE_EXPR */
        
        /* Single loop with multiple conditional reductions */
        for (int i = 0; i < 64; i++) {
            /* GT_EXPR for max */
            if (arr_int[i] > multi_max) {
                multi_max = arr_int[i];
            }
            
            /* LT_EXPR for min */
            if (arr_int[i] < multi_min) {
                multi_min = arr_int[i];
            }
            
            /* GE_EXPR for conditional sum */
            if (arr_int[i] >= threshold_int) {
                multi_sum_ge += arr_int[i];
            }
            
            /* LE_EXPR for conditional count */
            if (arr_int[i] <= threshold_int) {
                multi_count_le++;
            }
        }
        
        g_result_int += multi_max + multi_min + multi_sum_ge + multi_count_le;
    }
    
    /* TEST 6: Mixed types with different comparison operators */
    {
        float float_max = arr_float[0];          /* GT_EXPR */
        double double_min = arr_double[0];       /* LT_EXPR */
        int int_sum_ge = 0;                      /* GE_EXPR */
        int ushort_count_le = 0;                 /* LE_EXPR */
        
        /* While loop variant */
        int i = 0;
        while (i < 64) {
            /* GT_EXPR for float max */
            if (arr_float[i] > float_max) {
                float_max = arr_float[i];
            }
            
            /* LT_EXPR for double min */
            if (arr_double[i] < double_min) {
                double_min = arr_double[i];
            }
            
            /* GE_EXPR for integer sum */
            if (arr_int[i] >= threshold_int) {
                int_sum_ge += arr_int[i];
            }
            
            /* LE_EXPR for unsigned short count */
            if (arr_ushort[i] <= threshold_ushort) {
                ushort_count_le++;
            }
            
            i++;
        }
        
        g_result_float += float_max;
        g_result_double += double_min;
        g_result_int += int_sum_ge + ushort_count_le;
    }
    
    /* Compute final checksum */
    int checksum = g_result_int;
    checksum += (int)g_result_float;
    checksum += (int)g_result_double;
    
    /* Add array elements to checksum for verification */
    for (int i = 0; i < 64; i++) {
        checksum += arr_int[i] + (int)arr_float[i] + (int)arr_double[i] + arr_ushort[i];
    }
    
    printf("%d\n", checksum);
    return 0;
}
