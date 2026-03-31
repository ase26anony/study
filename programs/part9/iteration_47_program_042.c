#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent dead code elimination */
volatile int global_result = 0;
volatile float global_float_result = 0.0f;

/* Function to generate deterministic data based on seed */
static void init_data(int seed, int* int_arr, unsigned short* ushort_arr, 
                     float* float_arr, double* double_arr, int size) {
    for (int i = 0; i < size; i++) {
        int val = (i * 3 + seed) % 1000;
        int_arr[i] = val;
        ushort_arr[i] = (unsigned short)(val % 65535);
        float_arr[i] = val * 0.5f;
        double_arr[i] = val * 0.25;
    }
}

/* Test function with loop-invariant threshold from argument */
static int test_conditional_reductions(int seed, int threshold_int, 
                                      float threshold_float, 
                                      double threshold_double) {
    const int SIZE = 64;
    int int_arr[SIZE];
    unsigned short ushort_arr[SIZE];
    float float_arr[SIZE];
    double double_arr[SIZE];
    
    init_data(seed, int_arr, ushort_arr, float_arr, double_arr, SIZE);
    
    /* Initialize reduction variables with volatile to prevent constant propagation */
    volatile int init_max = -1000000;
    volatile int init_min = 1000000;
    volatile float init_fmax = -1e9f;
    volatile double init_dmin = 1e9;
    
    int max_val = init_max;
    int min_val = init_min;
    float fmax_val = init_fmax;
    double dmin_val = init_dmin;
    int cond_sum_gt = 0;
    int cond_sum_ge = 0;
    unsigned short cond_count_lt = 0;
    float cond_sum_le = 0.0f;
    
    /* Loop 1: Greater-than (GT_EXPR) conditional reduction with outer if */
    for (int i = 0; i < SIZE; i++) {
        /* Outer if to complicate control flow */
        if (i % 2 == 0) {
            /* Conditional max reduction with > */
            if (int_arr[i] > max_val) {
                max_val = int_arr[i];
            }
            
            /* Conditional sum with > */
            if (int_arr[i] > threshold_int) {
                cond_sum_gt += int_arr[i];
            }
        }
    }
    
    /* Loop 2: Greater-than-or-equal (GE_EXPR) with multiple reductions */
    int temp_max = max_val;  /* Use previous result */
    int temp_min = min_val;
    for (int i = 0; i < SIZE; i++) {
        /* Combined condition with logical AND */
        if (i >= 10 && i <= 50) {
            /* Conditional max with >= */
            if (int_arr[i] >= temp_max) {
                temp_max = int_arr[i];
            }
            
            /* Conditional sum with >= */
            if (ushort_arr[i] >= (threshold_int % 256)) {
                cond_sum_ge += ushort_arr[i];
            }
        }
    }
    
    /* Loop 3: Less-than (LT_EXPR) with while loop */
    int j = 0;
    int limit = SIZE - 4;  /* Loop-invariant computed limit */
    while (j < limit) {
        /* Nested conditionals */
        if (j % 3 != 0) {
            /* Conditional min with < */
            if (float_arr[j] < fmax_val) {
                fmax_val = float_arr[j];
            }
            
            /* Conditional count with < using logical OR */
            if (j < 20 || j > 40) {
                if (ushort_arr[j] < (threshold_int % 1000)) {
                    cond_count_lt++;
                }
            }
        }
        j++;
    }
    
    /* Loop 4: Less-than-or-equal (LE_EXPR) with mixed types */
    double local_threshold = threshold_double;
    int k = 0;
    for (k = 0; k < SIZE; k += 2) {  /* Step by 2 to avoid trivial vectorization */
        /* Multiple reductions in one loop */
        
        /* Conditional min with <= on double */
        if (double_arr[k] <= dmin_val) {
            dmin_val = double_arr[k];
        }
        
        /* Conditional sum with <= on float */
        if (float_arr[k] <= threshold_float) {
            cond_sum_le += float_arr[k];
        }
        
        /* Additional reduction with <= on int */
        if (int_arr[k] <= (threshold_int + 100)) {
            if (int_arr[k] < min_val) {
                min_val = int_arr[k];
            }
        }
    }
    
    /* Loop 5: All comparison operators in one complex loop */
    int complex_sum = 0;
    float complex_max = -1e9f;
    double complex_min = 1e9;
    
    for (int i = 0; i < SIZE; i++) {
        /* GT comparison */
        if (int_arr[i] > (threshold_int - 50)) {
            complex_sum += int_arr[i];
        }
        
        /* GE comparison */
        if (float_arr[i] >= (threshold_float - 10.0f)) {
            if (float_arr[i] > complex_max) {
                complex_max = float_arr[i];
            }
        }
        
        /* LT comparison with logical operator */
        if (i > 10 && double_arr[i] < (threshold_double + 5.0)) {
            if (double_arr[i] < complex_min) {
                complex_min = double_arr[i];
            }
        }
        
        /* LE comparison */
        if (ushort_arr[i] <= (unsigned short)(threshold_int % 500)) {
            complex_sum += ushort_arr[i];
        }
    }
    
    /* Aggregate results into checksum */
    int checksum = 0;
    checksum += max_val;
    checksum += min_val;
    checksum += (int)fmax_val;
    checksum += (int)dmin_val;
    checksum += cond_sum_gt;
    checksum += cond_sum_ge;
    checksum += cond_count_lt;
    checksum += (int)cond_sum_le;
    checksum += temp_max;
    checksum += complex_sum;
    checksum += (int)complex_max;
    checksum += (int)complex_min;
    
    /* Store to volatile to prevent elimination */
    global_result = checksum;
    global_float_result = complex_max;
    
    return checksum;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    
    /* Make thresholds loop-invariant but not compile-time constants */
    volatile int threshold_int = seed * 7 + 123;
    volatile float threshold_float = seed * 0.3f + 25.5f;
    volatile double threshold_double = seed * 0.1 + 50.25;
    
    int result = test_conditional_reductions(
        seed, 
        threshold_int,
        threshold_float,
        threshold_double
    );
    
    printf("Checksum: %d\n", result);
    return 0;
}
