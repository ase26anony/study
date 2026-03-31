#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile globals to prevent dead code elimination */
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
    
    /* Loop-invariant thresholds from volatile sources */
    volatile int vi = 25;
    volatile unsigned short vus = 30000;
    volatile float vf = 150.0f;
    volatile double vd = 250.0;
    
    int threshold_i = vi;
    unsigned short threshold_us = vus;
    float threshold_f = vf;
    double threshold_d = vd;
    
    /* Reduction variables */
    int max_i = arr_i[0];
    int min_i = arr_i[0];
    unsigned short max_us = arr_us[0];
    float sum_f_ge = 0.0f;
    double sum_d_le = 0.0;
    int count_gt = 0;
    int count_lt = 0;
    int mixed_max = arr_i[0];
    int mixed_min = arr_i[0];
    int mixed_sum = 0;
    
    /* 1. GT_EXPR pattern - find max with > comparison */
    for (int i = 0; i < SIZE; i++) {
        /* Outer if to complicate control flow */
        if (i % 2 == 0) {
            /* Conditional reduction with > */
            if (arr_i[i] > max_i) {
                max_i = arr_i[i];
            }
        }
    }
    
    /* 2. GE_EXPR pattern - sum values >= threshold */
    for (int i = 0; i < SIZE; i++) {
        /* Combined condition with logical AND */
        if (arr_f[i] >= threshold_f && i < SIZE - 1) {
            sum_f_ge += arr_f[i];
        }
    }
    
    /* 3. LT_EXPR pattern - find min with < comparison */
    int j = 0;
    while (j < SIZE) {
        /* Nested conditional */
        if (j > 0) {
            if (arr_i[j] < min_i) {
                min_i = arr_i[j];
            }
        }
        j++;
    }
    
    /* 4. LE_EXPR pattern - sum values <= threshold */
    for (int i = 0; i < SIZE; i++) {
        if (arr_d[i] <= threshold_d) {
            sum_d_le += arr_d[i];
        }
    }
    
    /* 5. Multiple reductions in one loop with different comparisons */
    for (int i = 0; i < SIZE; i++) {
        /* Count values > threshold */
        if (arr_i[i] > threshold_i) {
            count_gt++;
        }
        
        /* Count values < threshold with different threshold */
        if (arr_i[i] < (threshold_i + 10)) {
            count_lt++;
        }
        
        /* Update max and min simultaneously */
        if (arr_i[i] > mixed_max) {
            mixed_max = arr_i[i];
        }
        if (arr_i[i] < mixed_min) {
            mixed_min = arr_i[i];
        }
    }
    
    /* 6. Complex mixed-type reductions with logical OR */
    unsigned short complex_max = arr_us[0];
    int complex_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        /* Combined condition with OR */
        if (arr_us[i] >= threshold_us || arr_i[i % 16] > 50) {
            if (arr_us[i] > complex_max) {
                complex_max = arr_us[i];
            }
        }
        
        /* Another conditional sum with <= */
        if (arr_i[i] <= (threshold_i * 2)) {
            complex_sum += arr_i[i];
        }
    }
    
    /* 7. Float comparisons with mixed operators */
    float float_max = arr_f[0];
    float float_min = arr_f[0];
    for (int i = 0; i < SIZE; i++) {
        /* Outer if with modulus */
        if (i % 3 != 0) {
            /* GT comparison for max */
            if (arr_f[i] > float_max) {
                float_max = arr_f[i];
            }
            
            /* LT comparison for min */
            if (arr_f[i] < float_min) {
                float_min = arr_f[i];
            }
        }
    }
    
    /* Prevent optimization */
    global_sink = max_i + min_i + count_gt + count_lt + complex_sum;
    
    /* Compute checksum */
    int checksum = max_i + min_i + max_us + (int)sum_f_ge + (int)sum_d_le +
                   count_gt + count_lt + mixed_max + mixed_min + mixed_sum +
                   complex_max + complex_sum + (int)float_max + (int)float_min;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
