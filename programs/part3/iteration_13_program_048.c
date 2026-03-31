#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define SEED 42

/* Simple deterministic pseudo-random number generator */
static unsigned int lcg = SEED;
static inline int rand_int(int min, int max) {
    lcg = lcg * 1103515245 + 12345;
    return min + (lcg % (max - min + 1));
}

static inline float rand_float(float min, float max) {
    lcg = lcg * 1103515245 + 12345;
    return min + ((float)(lcg % 10001) / 10000.0f) * (max - min);
}

int main() {
    /* Declare arrays with different types */
    int arr_int[N];
    unsigned int arr_uint[N];
    float arr_float[N];
    double arr_double[N];
    signed char arr_char[N];
    
    int out1[N], out2[N], out3[N], out4[N];
    float fout1[N], fout2[N];
    double dout[N];
    long long checksum = 0;
    
    /* Initialize arrays with deterministic values */
    lcg = SEED;
    for (int i = 0; i < N; i++) {
        arr_int[i] = rand_int(-1000, 1000);
        arr_uint[i] = (unsigned int)rand_int(0, 2000);
        arr_float[i] = rand_float(-500.0f, 500.0f);
        arr_double[i] = (double)rand_float(-500.0f, 500.0f);
        arr_char[i] = (signed char)rand_int(-128, 127);
    }
    
    /* Loop 1: GT_EXPR (>) with integer array */
    const int threshold_gt = 100;
    for (int i = 0; i < N; i++) {
        /* Pattern: mask-based selection using > */
        out1[i] = (arr_int[i] > threshold_gt) ? arr_int[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with unsigned integer array */
    const unsigned int limit_ge = 500;
    for (int i = 0; i < N; i++) {
        /* Pattern: mask-based selection using >= */
        out2[i] = (arr_uint[i] >= limit_ge) ? arr_uint[i] : limit_ge;
    }
    
    /* Loop 3: LT_EXPR (<) with floating-point array */
    const float bound_lt = 0.0f;
    for (int i = 0; i < N; i++) {
        /* Pattern: mask-based selection using < */
        fout1[i] = (arr_float[i] < bound_lt) ? arr_float[i] : bound_lt;
    }
    
    /* Loop 4: LE_EXPR (<=) with double array */
    const double cap_le = 250.0;
    for (int i = 0; i < N; i++) {
        /* Pattern: mask-based selection using <= */
        dout[i] = (arr_double[i] <= cap_le) ? arr_double[i] : cap_le;
    }
    
    /* Loop 5: Mixed comparisons with logical AND (GT and LT) */
    const int low = -200;
    const int high = 200;
    for (int i = 0; i < N; i++) {
        /* Nested condition with > and < */
        if (arr_int[i] > low && arr_int[i] < high) {
            out3[i] = arr_int[i];
        } else {
            out3[i] = 0;
        }
    }
    
    /* Loop 6: Mixed comparisons with logical OR (LE and GE) */
    const float x = -100.0f;
    const float y = 100.0f;
    for (int i = 0; i < N; i++) {
        /* Condition with <= and >= */
        if (arr_float[i] <= x || arr_float[i] >= y) {
            fout2[i] = arr_float[i];
        } else {
            fout2[i] = 0.0f;
        }
    }
    
    /* Loop 7: Complex nested condition with signed char */
    const signed char min_char = -50;
    const signed char max_char = 50;
    for (int i = 0; i < N; i++) {
        /* Multiple comparisons in nested if */
        if (arr_char[i] > min_char) {
            if (arr_char[i] < max_char) {
                out4[i] = arr_char[i];
            } else {
                out4[i] = max_char;
            }
        } else {
            out4[i] = min_char;
        }
    }
    
    /* Loop 8: While loop with <= condition */
    int j = 0;
    int temp_sum = 0;
    while (j < N) {
        /* Using <= in while condition and mask operation */
        if (arr_int[j] <= threshold_gt) {
            temp_sum += arr_int[j];
        }
        j++;
    }
    
    /* Loop 9: For loop with increment and >= condition */
    int count_ge = 0;
    for (int i = 0; i < N; i++) {
        /* Using >= with accumulation */
        count_ge += (arr_uint[i] >= limit_ge) ? 1 : 0;
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i];
        checksum += (long long)fout1[i];
        checksum += (long long)fout2[i];
        checksum += (long long)dout[i];
    }
    checksum += temp_sum + count_ge;
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
