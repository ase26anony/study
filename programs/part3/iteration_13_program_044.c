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
    
    /* Initialize arrays with deterministic values */
    lcg = SEED;
    for (int i = 0; i < N; i++) {
        arr_int[i] = rand_int(-1000, 1000);
        arr_uint[i] = (unsigned int)rand_int(0, 2000);
        arr_float[i] = rand_float(-500.0f, 500.0f);
        arr_double[i] = (double)rand_float(-500.0f, 500.0f);
        arr_char[i] = (signed char)rand_int(-128, 127);
    }
    
    /* Loop 1: GT_EXPR (>) with integer mask pattern */
    const int threshold1 = 100;
    for (int i = 0; i < N; i++) {
        /* Pattern: result = (data > threshold) ? data : 0 */
        out1[i] = (arr_int[i] > threshold1) ? arr_int[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with floating-point mask pattern */
    const float limit = 0.0f;
    for (int i = 0; i < N; i++) {
        /* Pattern: result = (data >= limit) ? data : constant */
        fout1[i] = (arr_float[i] >= limit) ? arr_float[i] : -1.0f;
    }
    
    /* Loop 3: LT_EXPR (<) with unsigned integer and nested condition */
    const unsigned int bound = 1000;
    for (int i = 0; i < N; i++) {
        /* Pattern: if (data < bound) with mask-based assignment */
        if (arr_uint[i] < bound) {
            out2[i] = arr_uint[i] * 2;
        } else {
            out2[i] = arr_uint[i];
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with double precision */
    const double cap = 250.0;
    for (int i = 0; i < N; i++) {
        /* Pattern: result = (data <= cap) ? data : scaled_value */
        dout[i] = (arr_double[i] <= cap) ? arr_double[i] : arr_double[i] * 0.5;
    }
    
    /* Loop 5: Mixed comparisons with logical operators (decomposes to individual comparisons) */
    const int low = -500, high = 500;
    for (int i = 0; i < N; i++) {
        /* Pattern: if (data > low && data < high) - will decompose to GT_EXPR and LT_EXPR */
        if (arr_int[i] > low && arr_int[i] < high) {
            out3[i] = arr_int[i] + 100;
        } else {
            out3[i] = arr_int[i];
        }
    }
    
    /* Loop 6: Another GE_EXPR pattern with signed char */
    const signed char min_val = -50;
    for (int i = 0; i < N; i++) {
        /* Pattern: result = (data >= min_val) ? data : min_val */
        out4[i] = (arr_char[i] >= min_val) ? arr_char[i] : min_val;
    }
    
    /* Loop 7: LE_EXPR with OR logical operator */
    const float x = -100.0f, y = 100.0f;
    for (int i = 0; i < N; i++) {
        /* Pattern: if (data <= x || data >= y) - decomposes to LE_EXPR and GE_EXPR */
        if (arr_float[i] <= x || arr_float[i] >= y) {
            fout2[i] = arr_float[i] * 2.0f;
        } else {
            fout2[i] = arr_float[i];
        }
    }
    
    /* Loop 8: Complex nested condition with all four operators */
    const int a = -300, b = -100, c = 100, d = 300;
    int temp_out[N];
    for (int i = 0; i < N; i++) {
        /* Complex condition that may decompose into multiple comparisons */
        if ((arr_int[i] > a && arr_int[i] <= b) || (arr_int[i] >= c && arr_int[i] < d)) {
            temp_out[i] = arr_int[i] * 3;
        } else {
            temp_out[i] = arr_int[i];
        }
    }
    
    /* Compute checksums to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i] + temp_out[i];
        checksum += (long long)fout1[i];
        checksum += (long long)fout2[i];
        checksum += (long long)dout[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
