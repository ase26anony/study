#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent dead code elimination */
volatile int global_sink;

/* Function to initialize arrays deterministically */
void init_arrays(int seed, int* arr_i, unsigned short* arr_us, 
                 float* arr_f, double* arr_d, int size) {
    for (int i = 0; i < size; i++) {
        arr_i[i] = (i * 3 + seed) % 100;
        arr_us[i] = (unsigned short)((i * 5 + seed) % 65535);
        arr_f[i] = (float)((i * 7 + seed) % 1000) / 3.0f;
        arr_d[i] = (double)((i * 11 + seed) % 2000) / 7.0;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    const int SIZE = 64;
    
    /* Declare and initialize arrays */
    int arr_i[SIZE];
    unsigned short arr_us[SIZE];
    float arr_f[SIZE];
    double arr_d[SIZE];
    
    init_arrays(seed, arr_i, arr_us, arr_f, arr_d, SIZE);
    
    /* Make thresholds loop-invariant but not compile-time constants */
    volatile int vi = seed + 10;
    volatile float vf = (float)(seed % 50) + 15.5f;
    volatile double vd = (double)(seed % 100) + 25.7;
    
    int threshold_i = vi;
    unsigned short threshold_us = (unsigned short)(vi % 200);
    float threshold_f = vf;
    double threshold_d = vd;
    
    /* Reduction variables */
    int max_val_i = arr_i[0];
    int min_val_i = arr_i[0];
    unsigned short max_val_us = arr_us[0];
    float cond_sum_f = 0.0f;
    double cond_sum_d = 0.0;
    int count_gt = 0;
    int count_le = 0;
    int mixed_max = arr_i[0];
    float mixed_min_f = arr_f[0];
    
    /* ===== Loop 1: GT_EXPR (greater-than) conditional reduction ===== */
    /* Find maximum value where arr_i[i] > threshold_i */
    for (int i = 0; i < SIZE; i++) {
        /* Outer if to complicate control flow */
        if (i % 4 != 0) {
            /* GT_EXPR pattern */
            if (arr_i[i] > threshold_i) {
                if (arr_i[i] > max_val_i) {
                    max_val_i = arr_i[i];
                }
            }
        }
    }
    
    /* ===== Loop 2: GE_EXPR (greater-than-or-equal) conditional reduction ===== */
    /* Sum values where arr_f[i] >= threshold_f */
    for (int i = 0; i < SIZE; i++) {
        /* GE_EXPR pattern with logical AND */
        if (i < SIZE - 1 && arr_f[i] >= threshold_f) {
            cond_sum_f += arr_f[i];
        }
    }
    
    /* ===== Loop 3: LT_EXPR (less-than) conditional reduction ===== */
    /* Find minimum value where arr_i[i] < threshold_i */
    /* Using while loop variant */
    int j = 0;
    while (j < SIZE) {
        /* LT_EXPR pattern with outer condition */
        if (j % 3 == 0) {
            if (arr_i[j] < threshold_i) {
                if (arr_i[j] < min_val_i) {
                    min_val_i = arr_i[j];
                }
            }
        }
        j++;
    }
    
    /* ===== Loop 4: LE_EXPR (less-than-or-equal) conditional reduction ===== */
    /* Sum values where arr_d[i] <= threshold_d */
    for (int i = 0; i < SIZE; i++) {
        /* LE_EXPR pattern with logical OR in condition */
        if (i == 0 || arr_d[i] <= threshold_d) {
            cond_sum_d += arr_d[i];
        }
    }
    
    /* ===== Loop 5: Multiple reductions with different comparisons ===== */
    /* This loop performs multiple conditional reductions simultaneously */
    for (int i = 0; i < SIZE; i++) {
        /* Multiple reduction variables in one loop */
        
        /* GT_EXPR: Count elements greater than threshold */
        if (arr_i[i] > threshold_i) {
            count_gt++;
        }
        
        /* LE_EXPR: Count elements less than or equal to threshold */
        if (arr_i[i] <= threshold_i) {
            count_le++;
        }
        
        /* Mixed type comparisons */
        if (arr_i[i] > threshold_i && arr_f[i] < threshold_f) {
            if (arr_i[i] > mixed_max) {
                mixed_max = arr_i[i];
            }
        }
        
        /* Nested conditionals */
        if (arr_us[i] > threshold_us) {
            if (arr_f[i] < mixed_min_f) {
                mixed_min_f = arr_f[i];
            }
        }
    }
    
    /* ===== Loop 6: Complex nested conditions ===== */
    /* Using unsigned short array with multiple conditions */
    unsigned short complex_max = arr_us[0];
    int complex_sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        /* Complex condition combining multiple comparisons */
        if ((arr_us[i] > threshold_us || i % 2 == 0) && 
            (arr_i[i] < threshold_i + 5)) {
            /* GE_EXPR pattern */
            if (arr_us[i] >= threshold_us) {
                if (arr_us[i] > complex_max) {
                    complex_max = arr_us[i];
                }
            }
            
            /* LT_EXPR pattern */
            if (arr_i[i] < threshold_i) {
                complex_sum += arr_i[i];
            }
        }
    }
    
    /* Aggregate results into checksum */
    int checksum = 0;
    checksum += max_val_i;
    checksum += min_val_i;
    checksum += max_val_us;
    checksum += (int)cond_sum_f;
    checksum += (int)cond_sum_d;
    checksum += count_gt;
    checksum += count_le;
    checksum += mixed_max;
    checksum += (int)mixed_min_f;
    checksum += complex_max;
    checksum += complex_sum;
    
    /* Prevent dead code elimination */
    global_sink = checksum;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
