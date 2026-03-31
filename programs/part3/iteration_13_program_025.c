#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define SEED 42

/* Simple deterministic pseudo-random generator */
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
    
    /* Initialize arrays with deterministic values */
    for (int i = 0; i < N; i++) {
        arr_int[i] = rand_int(-1000, 1000);
        arr_uint[i] = (unsigned int)rand_int(0, 2000);
        arr_float[i] = rand_float(-500.0f, 500.0f);
        arr_double[i] = (double)rand_float(-500.0f, 500.0f);
        arr_char[i] = (signed char)rand_int(-128, 127);
    }
    
    /* Loop 1: GT_EXPR (>) with integer type */
    const int threshold_gt = 100;
    for (int i = 0; i < N; i++) {
        /* Create mask using > comparison */
        out1[i] = (arr_int[i] > threshold_gt) ? arr_int[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with unsigned integer type */
    const unsigned int limit_ge = 500;
    for (int i = 0; i < N; i++) {
        /* Create mask using >= comparison */
        out2[i] = (arr_uint[i] >= limit_ge) ? (int)arr_uint[i] : -1;
    }
    
    /* Loop 3: LT_EXPR (<) with floating-point type */
    const float bound_lt = 0.0f;
    for (int i = 0; i < N; i++) {
        /* Create mask using < comparison */
        fout1[i] = (arr_float[i] < bound_lt) ? arr_float[i] : bound_lt;
    }
    
    /* Loop 4: LE_EXPR (<=) with double type */
    const double cap_le = 250.0;
    for (int i = 0; i < N; i++) {
        /* Create mask using <= comparison */
        dout[i] = (arr_double[i] <= cap_le) ? arr_double[i] : cap_le;
    }
    
    /* Loop 5: Mixed comparisons with signed char */
    const signed char low = -50;
    const signed char high = 50;
    for (int i = 0; i < N; i++) {
        /* Nested condition with both < and > */
        if (arr_char[i] > low && arr_char[i] < high) {
            out3[i] = arr_char[i] * 2;
        } else {
            out3[i] = arr_char[i];
        }
    }
    
    /* Loop 6: Complex predicate with >= and <= */
    const float x = -100.0f;
    const float y = 100.0f;
    for (int i = 0; i < N; i++) {
        /* Logical OR with both >= and <= */
        if (arr_float[i] <= x || arr_float[i] >= y) {
            fout2[i] = arr_float[i] * 0.5f;
        } else {
            fout2[i] = arr_float[i];
        }
    }
    
    /* Loop 7: While loop with > comparison */
    int j = 0;
    while (j < N) {
        /* Using > in while condition and ternary */
        out4[j] = (arr_int[j] > 0) ? arr_int[j] : arr_int[j] * (-1);
        j++;
    }
    
    /* Loop 8: Accumulation with >= mask */
    int sum_ge = 0;
    const int limit_acc = 300;
    for (int i = 0; i < N; i++) {
        /* Accumulate based on >= comparison */
        sum_ge += (arr_int[i] >= limit_acc) ? arr_int[i] : 0;
    }
    
    /* Loop 9: Nested loops with < comparison */
    const int block_size = 16;
    for (int block = 0; block < N/block_size; block++) {
        for (int k = 0; k < block_size; k++) {
            int idx = block * block_size + k;
            /* Using < comparison for mask */
            int temp = (arr_int[idx] < -threshold_gt) ? -arr_int[idx] : arr_int[idx];
            out1[idx] += temp;  /* Reuse output array */
        }
    }
    
    /* Compute checksums to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i];
        checksum += (long long)fout1[i];
        checksum += (long long)fout2[i];
        checksum += (long long)dout[i];
    }
    checksum += sum_ge;
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
