#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_seed = 0;
volatile int g_result_store[8] = {0};

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

/* Conditional reduction with GT_EXPR (>) */
int test_gt_reduction(int* arr, int n, int threshold) {
    int max_val = arr[0];
    for (int i = 0; i < n; i++) {
        /* Outer if to complicate control flow */
        if (arr[i] != 0) {
            /* GT_EXPR pattern: if (arr[i] > max_val) max_val = arr[i]; */
            if (arr[i] > max_val) {
                max_val = arr[i];
            }
        }
    }
    return max_val;
}

/* Conditional reduction with GE_EXPR (>=) */
float test_ge_reduction(float* arr, int n, float threshold) {
    float sum = 0.0f;
    /* While loop variant */
    int i = 0;
    while (i < n) {
        /* GE_EXPR pattern with logical AND */
        if (arr[i] >= threshold && i % 2 == 0) {
            sum += arr[i];
        }
        i++;
    }
    return sum;
}

/* Conditional reduction with LT_EXPR (<) */
double test_lt_reduction(double* arr, int n, double threshold) {
    double min_val = arr[0];
    int count = 0;
    
    /* Multiple reductions in one loop */
    for (int i = 0; i < n; i++) {
        /* LT_EXPR pattern: if (arr[i] < min_val) min_val = arr[i]; */
        if (arr[i] < min_val) {
            min_val = arr[i];
        }
        
        /* Additional conditional reduction with different type */
        if (arr[i] < threshold) {
            count++;
        }
    }
    
    /* Store to volatile to prevent elimination */
    g_result_store[0] = count;
    return min_val;
}

/* Conditional reduction with LE_EXPR (<=) */
unsigned short test_le_reduction(unsigned short* arr, int n, unsigned short threshold) {
    unsigned short max_val = 0;
    unsigned short min_val = 65535;
    
    /* Complex nested conditionals */
    for (int i = 0; i < n; i++) {
        if (arr[i] > 100) {  /* Outer condition */
            /* LE_EXPR pattern with logical OR */
            if (arr[i] <= threshold || (i % 3 == 0)) {
                if (arr[i] > max_val) {
                    max_val = arr[i];
                }
            }
            
            /* Another LE_EXPR in same loop */
            if (arr[i] <= max_val) {
                if (arr[i] < min_val) {
                    min_val = arr[i];
                }
            }
        }
    }
    
    return max_val + min_val;
}

/* Mixed type reductions in single loop */
void test_mixed_reductions(int* arr_int, float* arr_float, int n, 
                           int int_thresh, float float_thresh) {
    int int_sum = 0;
    float float_sum = 0.0f;
    int int_max = arr_int[0];
    float float_min = arr_float[0];
    
    for (int i = 0; i < n; i++) {
        /* GT_EXPR for integers */
        if (arr_int[i] > int_thresh) {
            int_sum += arr_int[i];
        }
        
        /* LT_EXPR for floats */
        if (arr_float[i] < float_thresh) {
            float_sum += arr_float[i];
        }
        
        /* GE_EXPR for integer max */
        if (arr_int[i] >= int_max) {
            int_max = arr_int[i];
        }
        
        /* LE_EXPR for float min */
        if (arr_float[i] <= float_min) {
            float_min = arr_float[i];
        }
    }
    
    g_result_store[1] = int_sum;
    g_result_store[2] = (int)float_sum;
    g_result_store[3] = int_max;
    g_result_store[4] = (int)(float_min * 100);
}

int main(int argc, char** argv) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    const int N = 64;
    int arr_int[N];
    float arr_float[N];
    double arr_double[N];
    unsigned short arr_ushort[N];
    
    /* Initialize arrays with deterministic but non-constant data */
    for (int i = 0; i < N; i++) {
        arr_int[i] = gen_data(i, seed);
        arr_float[i] = gen_float_data(i, seed);
        arr_double[i] = gen_double_data(i, seed);
        arr_ushort[i] = (unsigned short)(gen_data(i, seed) % 65535);
    }
    
    /* Make thresholds loop-invariant but not compile-time constants */
    volatile int vol_threshold_int = seed + 50;
    volatile float vol_threshold_float = (float)(seed % 100) / 2.0f;
    volatile double vol_threshold_double = (double)(seed % 200) / 4.0;
    volatile unsigned short vol_threshold_ushort = (unsigned short)(seed % 1000);
    
    int threshold_int = vol_threshold_int;
    float threshold_float = vol_threshold_float;
    double threshold_double = vol_threshold_double;
    unsigned short threshold_ushort = vol_threshold_ushort;
    
    /* Test each comparison operator */
    int result1 = test_gt_reduction(arr_int, N, threshold_int);
    float result2 = test_ge_reduction(arr_float, N, threshold_float);
    double result3 = test_lt_reduction(arr_double, N, threshold_double);
    unsigned short result4 = test_le_reduction(arr_ushort, N, threshold_ushort);
    
    /* Test mixed reductions */
    test_mixed_reductions(arr_int, arr_float, N, threshold_int, threshold_float);
    
    /* Compute checksum from all results */
    int checksum = result1;
    checksum += (int)result2;
    checksum += (int)result3;
    checksum += result4;
    
    /* Add volatile stored results */
    for (int i = 0; i < 5; i++) {
        checksum += g_result_store[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
