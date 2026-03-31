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
    
    /* Initialize with deterministic values */
    lcg = SEED;
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
        /* Mask-based computation using > */
        out1[i] = (arr_int[i] > threshold_gt) ? arr_int[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with unsigned integer type */
    const unsigned int limit_ge = 500;
    for (int i = 0; i < N; i++) {
        /* Mask-based computation using >= */
        out2[i] = (arr_uint[i] >= limit_ge) ? (int)arr_uint[i] : -1;
    }
    
    /* Loop 3: LT_EXPR (<) with floating-point type */
    const float bound_lt = 0.0f;
    for (int i = 0; i < N; i++) {
        /* Mask-based computation using < */
        fout1[i] = (arr_float[i] < bound_lt) ? arr_float[i] : 0.0f;
    }
    
    /* Loop 4: LE_EXPR (<=) with double type */
    const double cap_le = 250.0;
    for (int i = 0; i < N; i++) {
        /* Mask-based computation using <= */
        dout[i] = (arr_double[i] <= cap_le) ? arr_double[i] : cap_le;
    }
    
    /* Loop 5: Mixed GT_EXPR with signed char */
    const signed char char_threshold = 0;
    for (int i = 0; i < N; i++) {
        /* Using > with signed char */
        out3[i] = (arr_char[i] > char_threshold) ? arr_char[i] : 0;
    }
    
    /* Loop 6: Nested conditionals with GE_EXPR and LE_EXPR */
    const int low = -200;
    const int high = 200;
    for (int i = 0; i < N; i++) {
        /* Complex predicate: arr_int[i] >= low && arr_int[i] <= high */
        if (arr_int[i] >= low && arr_int[i] <= high) {
            out4[i] = arr_int[i];
        } else {
            out4[i] = 0;
        }
    }
    
    /* Loop 7: Logical OR with LT_EXPR and GT_EXPR */
    const float f_low = -100.0f;
    const float f_high = 100.0f;
    for (int i = 0; i < N; i++) {
        /* arr_float[i] < f_low || arr_float[i] > f_high */
        fout2[i] = (arr_float[i] < f_low || arr_float[i] > f_high) ? arr_float[i] : 0.0f;
    }
    
    /* Loop 8: Combined comparisons in ternary operator */
    const int mid = 0;
    for (int i = 0; i < N; i++) {
        /* Using both < and > in same expression */
        int val = arr_int[i];
        out1[i] += (val < mid) ? -val : (val > mid) ? val : 0;
    }
    
    /* Loop 9: While loop with LE_EXPR */
    int j = 0;
    int temp_sum = 0;
    while (j < N) {
        /* Using <= in while condition and inside loop */
        if (arr_int[j] <= threshold_gt) {
            temp_sum += arr_int[j];
        }
        j++;
    }
    
    /* Loop 10: For loop with increment based on GE_EXPR */
    int count = 0;
    for (int i = 0; i < N; i++) {
        /* Assignment based on >= comparison */
        int mask = arr_uint[i] >= limit_ge;
        out2[i] = mask ? out2[i] * 2 : out2[i];
        count += mask;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i];
        checksum += (long long)fout1[i];
        checksum += (long long)dout[i];
        checksum += (long long)fout2[i];
    }
    checksum += temp_sum + count;
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
