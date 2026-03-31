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
    for (int i = 0; i < N; i++) {
        arr_int[i] = rand_int(-1000, 1000);
        arr_uint[i] = (unsigned int)rand_int(0, 2000);
        arr_float[i] = rand_float(-500.0f, 500.0f);
        arr_double[i] = (double)rand_float(-500.0f, 500.0f);
        arr_char[i] = (signed char)rand_int(-128, 127);
    }
    
    /* Loop 1: GT_EXPR (>) with integer type and mask-based computation */
    int threshold1 = 100;
    for (int i = 0; i < N; i++) {
        /* This should generate GT_EXPR -> BIT_NOT_EXPR, BIT_AND_EXPR pattern */
        out1[i] = (arr_int[i] > threshold1) ? arr_int[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with floating-point and nested conditionals */
    float limit = 0.0f;
    for (int i = 0; i < N; i++) {
        /* Complex predicate with GE_EXPR that may be decomposed */
        if (arr_float[i] >= limit && arr_float[i] < 100.0f) {
            fout1[i] = arr_float[i];
        } else {
            fout1[i] = limit;
        }
    }
    
    /* Loop 3: LT_EXPR (<) with unsigned integers and logical OR */
    unsigned int bound = 1000;
    for (int i = 0; i < N; i++) {
        /* LT_EXPR with unsigned, should trigger operand swap */
        out2[i] = (arr_uint[i] < bound) ? (int)arr_uint[i] : (int)bound;
    }
    
    /* Loop 4: LE_EXPR (<=) with double precision and mask */
    double cap = 250.0;
    for (int i = 0; i < N; i++) {
        /* LE_EXPR with double, should trigger operand swap */
        dout[i] = (arr_double[i] <= cap) ? arr_double[i] : cap;
    }
    
    /* Loop 5: Mixed comparisons with signed char */
    signed char low = -50, high = 50;
    for (int i = 0; i < N; i++) {
        /* Combined GT and LT comparisons */
        out3[i] = (arr_char[i] > low && arr_char[i] < high) ? arr_char[i] * 2 : arr_char[i];
    }
    
    /* Loop 6: GE_EXPR with accumulation pattern */
    int sum = 0;
    int limit2 = -200;
    for (int i = 0; i < N; i++) {
        /* GE_EXPR used in accumulation */
        sum += (arr_int[i] >= limit2) ? arr_int[i] : 0;
    }
    
    /* Loop 7: LE_EXPR with floating point and ternary */
    float threshold2 = -100.0f;
    for (int i = 0; i < N; i++) {
        /* LE_EXPR pattern that should be converted */
        fout2[i] = (arr_float[i] <= threshold2) ? arr_float[i] * 2.0f : arr_float[i];
    }
    
    /* Loop 8: GT_EXPR with while loop style */
    int j = 0;
    int threshold3 = 500;
    while (j < N) {
        /* GT_EXPR in while condition context */
        out4[j] = (arr_int[j] > threshold3) ? 1 : 0;
        j++;
    }
    
    /* Compute checksums to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i];
        checksum += (long long)fout1[i] + (long long)fout2[i];
        checksum += (long long)dout[i];
    }
    checksum += sum;
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
