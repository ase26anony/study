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
    unsigned int uout[N];
    
    /* Initialize with deterministic values */
    for (int i = 0; i < N; i++) {
        arr_int[i] = rand_int(-1000, 1000);
        arr_uint[i] = (unsigned int)rand_int(0, 2000);
        arr_float[i] = rand_float(-500.0f, 500.0f);
        arr_double[i] = (double)rand_float(-500.0f, 500.0f);
        arr_char[i] = (signed char)rand_int(-128, 127);
    }
    
    /* Loop 1: GT_EXPR (>) with integer mask pattern */
    int threshold1 = 100;
    for (int i = 0; i < N; i++) {
        /* Pattern: result = (data > threshold) ? data : 0 */
        out1[i] = (arr_int[i] > threshold1) ? arr_int[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with floating-point mask pattern */
    float limit = 50.0f;
    for (int i = 0; i < N; i++) {
        /* Pattern: result = (data >= limit) ? data : constant */
        fout1[i] = (arr_float[i] >= limit) ? arr_float[i] : -1.0f;
    }
    
    /* Loop 3: LT_EXPR (<) with unsigned integer and nested condition */
    unsigned int bound = 1000;
    for (int i = 0; i < N; i++) {
        /* Pattern: if (data < bound) with mask-based computation */
        if (arr_uint[i] < bound) {
            uout[i] = arr_uint[i] * 2;
        } else {
            uout[i] = arr_uint[i];
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with double precision */
    double cap = 250.0;
    for (int i = 0; i < N; i++) {
        /* Pattern: result = (data <= cap) ? data : scaled_value */
        dout[i] = (arr_double[i] <= cap) ? arr_double[i] : arr_double[i] * 0.5;
    }
    
    /* Loop 5: Mixed comparisons with logical operators (exposes individual comparisons) */
    int low = -500, high = 500;
    for (int i = 0; i < N; i++) {
        /* Decomposes into GT_EXPR and LT_EXPR */
        if (arr_int[i] > low && arr_int[i] < high) {
            out2[i] = arr_int[i] * 3;
        } else {
            out2[i] = arr_int[i];
        }
    }
    
    /* Loop 6: Another GE_EXPR pattern with signed char */
    signed char char_limit = 50;
    for (int i = 0; i < N; i++) {
        /* Pattern using >= with different type */
        out3[i] = (arr_char[i] >= char_limit) ? (int)arr_char[i] : 0;
    }
    
    /* Loop 7: LE_EXPR with OR logical operator */
    float x = -100.0f, y = 100.0f;
    for (int i = 0; i < N; i++) {
        /* Decomposes into LE_EXPR and GE_EXPR */
        if (arr_float[i] <= x || arr_float[i] >= y) {
            fout2[i] = arr_float[i] * 2.0f;
        } else {
            fout2[i] = arr_float[i];
        }
    }
    
    /* Loop 8: Complex nested condition with all four operators */
    int a = -300, b = -100, c = 100, d = 300;
    for (int i = 0; i < N; i++) {
        /* Contains >, >=, <, <= in different branches */
        if (arr_int[i] > a && arr_int[i] < b) {
            out4[i] = arr_int[i] + 10;
        } else if (arr_int[i] >= c && arr_int[i] <= d) {
            out4[i] = arr_int[i] - 10;
        } else {
            out4[i] = arr_int[i];
        }
    }
    
    /* Compute checksums to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i];
        checksum += (long long)uout[i];
        checksum += (long long)fout1[i];
        checksum += (long long)fout2[i];
        checksum += (long long)dout[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
