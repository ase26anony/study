#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile globals to prevent dead code elimination */
volatile int g_result_int;
volatile float g_result_float;
volatile double g_result_double;

/* Function to generate deterministic data */
int init_value(int i, int seed) {
    return (i * 3 + seed) % 1000;
}

float init_float(int i, int seed) {
    return (float)((i * 7 + seed * 3) % 1000) / 10.0f;
}

double init_double(int i, int seed) {
    return (double)((i * 11 + seed * 5) % 1000) / 5.0;
}

unsigned short init_ushort(int i, int seed) {
    return (unsigned short)((i * 13 + seed * 7) % 65535);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    
    /* Declare arrays with different types */
    int arr_int[64];
    float arr_float[64];
    double arr_double[64];
    unsigned short arr_ushort[64];
    
    /* Initialize arrays deterministically */
    for (int i = 0; i < 64; i++) {
        arr_int[i] = init_value(i, seed);
        arr_float[i] = init_float(i, seed);
        arr_double[i] = init_double(i, seed);
        arr_ushort[i] = init_ushort(i, seed);
    }
    
    /* Loop-invariant thresholds from volatile sources */
    volatile int thresh_int = seed + 500;
    volatile float thresh_float = (float)seed / 2.0f + 25.0f;
    volatile double thresh_double = (double)seed / 3.0 + 50.0;
    volatile unsigned short thresh_ushort = (unsigned short)(seed + 30000);
    
    int final_checksum = 0;
    
    /* ===== TEST 1: GT_EXPR (greater-than) conditional reduction ===== */
    {
        /* Multiple reduction variables in one loop */
        int max_val_gt = arr_int[0];
        int sum_gt = 0;
        int count_gt = 0;
        
        /* Outer if to complicate control flow */
        if (seed > 0) {
            for (int i = 0; i < 64; i++) {
                /* Conditional max reduction with > */
                if (arr_int[i] > max_val_gt) {
                    max_val_gt = arr_int[i];
                }
                
                /* Conditional sum with > using ternary */
                sum_gt += (arr_int[i] > thresh_int) ? arr_int[i] : 0;
                
                /* Conditional count with > */
                count_gt += (arr_int[i] > thresh_int) ? 1 : 0;
            }
        }
        
        final_checksum += max_val_gt + sum_gt + count_gt;
        g_result_int = max_val_gt;
    }
    
    /* ===== TEST 2: GE_EXPR (greater-than-or-equal) conditional reduction ===== */
    {
        float max_val_ge = arr_float[0];
        float sum_ge = 0.0f;
        
        /* While loop variant */
        int i = 0;
        while (i < 64) {
            /* Combined condition with logical AND */
            if (i % 2 == 0 && arr_float[i] >= max_val_ge) {
                max_val_ge = arr_float[i];
            }
            
            /* Conditional sum with >= */
            sum_ge += (arr_float[i] >= thresh_float) ? arr_float[i] : 0.0f;
            
            i++;
        }
        
        final_checksum += (int)max_val_ge + (int)sum_ge;
        g_result_float = max_val_ge;
    }
    
    /* ===== TEST 3: LT_EXPR (less-than) conditional reduction ===== */
    {
        double min_val_lt = arr_double[0];
        double sum_lt = 0.0;
        int count_lt = 0;
        
        /* Loop with multiple conditions */
        for (int i = 0; i < 64; i++) {
            /* Conditional min reduction with < */
            if (arr_double[i] < min_val_lt) {
                min_val_lt = arr_double[i];
            }
            
            /* Conditional sum with < */
            sum_lt += (arr_double[i] < thresh_double) ? arr_double[i] : 0.0;
            
            /* Nested conditional */
            if (i < 32) {
                count_lt += (arr_double[i] < thresh_double) ? 1 : 0;
            }
        }
        
        final_checksum += (int)min_val_lt + (int)sum_lt + count_lt;
        g_result_double = min_val_lt;
    }
    
    /* ===== TEST 4: LE_EXPR (less-than-or-equal) conditional reduction ===== */
    {
        unsigned short min_val_le = arr_ushort[0];
        unsigned int sum_le = 0;
        
        /* Loop with combined logical OR */
        for (int i = 0; i < 64; i++) {
            /* Conditional min with <= */
            if (arr_ushort[i] <= min_val_le || i == 0) {
                min_val_le = arr_ushort[i];
            }
            
            /* Conditional sum with <= using if statement */
            if (arr_ushort[i] <= thresh_ushort) {
                sum_le += arr_ushort[i];
            }
        }
        
        final_checksum += (int)min_val_le + (int)sum_le;
    }
    
    /* ===== TEST 5: Mixed reductions in single loop ===== */
    {
        /* Multiple reduction variables with different conditions */
        int mixed_max = arr_int[0];
        int mixed_min = arr_int[0];
        int mixed_sum_gt = 0;
        int mixed_sum_lt = 0;
        
        for (int i = 0; i < 64; i++) {
            /* GT and LT reductions together */
            if (arr_int[i] > mixed_max) {
                mixed_max = arr_int[i];
            }
            
            if (arr_int[i] < mixed_min) {
                mixed_min = arr_int[i];
            }
            
            /* Multiple conditional sums */
            mixed_sum_gt += (arr_int[i] > thresh_int) ? arr_int[i] : 0;
            mixed_sum_lt += (arr_int[i] < thresh_int) ? arr_int[i] : 0;
        }
        
        final_checksum += mixed_max + mixed_min + mixed_sum_gt + mixed_sum_lt;
    }
    
    /* ===== TEST 6: Floating-point with GE and LE ===== */
    {
        float fp_max_ge = arr_float[0];
        float fp_min_le = arr_float[0];
        
        /* Complicated control flow */
        for (int i = 0; i < 64; i++) {
            if (i % 3 == 0) {
                /* GE comparison */
                if (arr_float[i] >= fp_max_ge) {
                    fp_max_ge = arr_float[i];
                }
            } else if (i % 3 == 1) {
                /* LE comparison */
                if (arr_float[i] <= fp_min_le) {
                    fp_min_le = arr_float[i];
                }
            }
        }
        
        final_checksum += (int)fp_max_ge + (int)fp_min_le;
    }
    
    printf("Checksum: %d\n", final_checksum);
    return 0;
}
