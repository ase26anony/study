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
    float out_float[N];
    double out_double[N];
    
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
    /* Pattern: result[i] = (data[i] > threshold) ? data[i] : 0 */
    const int threshold1 = 100;
    for (int i = 0; i < N; i++) {
        out1[i] = (arr_int[i] > threshold1) ? arr_int[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with unsigned integer array */
    /* Pattern: if (data[i] >= limit) accumulate */
    const unsigned int limit = 1000;
    unsigned int sum2 = 0;
    for (int i = 0; i < N; i++) {
        if (arr_uint[i] >= limit) {
            sum2 += arr_uint[i];
        }
    }
    
    /* Loop 3: LT_EXPR (<) with floating-point array */
    /* Pattern: result[i] = (data[i] < bound) ? data[i] : constant */
    const float bound = 250.0f;
    const float constant = -1.0f;
    for (int i = 0; i < N; i++) {
        out_float[i] = (arr_float[i] < bound) ? arr_float[i] : constant;
    }
    
    /* Loop 4: LE_EXPR (<=) with double array */
    /* Pattern: mask-based computation with nested condition */
    const double cap = 300.0;
    for (int i = 0; i < N; i++) {
        /* Complex predicate that will decompose to LE_EXPR */
        if (arr_double[i] <= cap && arr_double[i] > -cap) {
            out_double[i] = arr_double[i] * 2.0;
        } else {
            out_double[i] = arr_double[i];
        }
    }
    
    /* Additional loops with mixed types and logical operators */
    
    /* Loop 5: GT_EXPR with signed char and logical OR */
    const signed char low = -50;
    const signed char high = 50;
    for (int i = 0; i < N; i++) {
        /* This creates a predicate that will be decomposed */
        if (arr_char[i] > high || arr_char[i] < low) {
            out2[i] = 1;
        } else {
            out2[i] = 0;
        }
    }
    
    /* Loop 6: GE_EXPR with integer and nested ternary */
    const int limit2 = -200;
    for (int i = 0; i < N; i++) {
        out3[i] = (arr_int[i] >= limit2) ? 
                 ((arr_int[i] > 500) ? arr_int[i] : arr_int[i] / 2) : 
                 arr_int[i] * 3;
    }
    
    /* Loop 7: LT_EXPR with unsigned int and bitwise mask pattern */
    const unsigned int upper = 1500;
    for (int i = 0; i < N; i++) {
        /* Direct mask-like computation */
        out4[i] = (arr_uint[i] < upper) ? arr_uint[i] : (arr_uint[i] / 2);
    }
    
    /* Loop 8: LE_EXPR with float and complex condition */
    const float x = -100.0f;
    const float y = 100.0f;
    float sum8 = 0.0f;
    for (int i = 0; i < N; i++) {
        /* Compound condition that will expose LE_EXPR and GE_EXPR */
        if (arr_float[i] <= x || arr_float[i] >= y) {
            sum8 += arr_float[i];
        }
    }
    
    /* Compute checksums to prevent dead code elimination */
    long long checksum = 0;
    float fchecksum = 0.0f;
    double dchecksum = 0.0;
    
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i];
        fchecksum += out_float[i];
        dchecksum += out_double[i];
    }
    
    checksum += sum2;
    fchecksum += sum8;
    
    printf("Checksums: %lld, %f, %f\n", checksum, fchecksum, dchecksum);
    
    return 0;
}
