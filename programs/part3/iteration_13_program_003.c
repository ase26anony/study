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
        arr_double[i] = (double)rand_float(-1000.0f, 1000.0f);
        arr_char[i] = (signed char)rand_int(-128, 127);
    }
    
    /* Loop 1: GT_EXPR (>) with integer type and ternary operator */
    int threshold1 = 100;
    for (int i = 0; i < N; i++) {
        /* This should generate GT_EXPR pattern */
        out1[i] = (arr_int[i] > threshold1) ? arr_int[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with unsigned int and if statement */
    unsigned int limit = 1000;
    for (int i = 0; i < N; i++) {
        /* This should generate GE_EXPR pattern */
        if (arr_uint[i] >= limit) {
            out2[i] = arr_uint[i];
        } else {
            out2[i] = limit;
        }
    }
    
    /* Loop 3: LT_EXPR (<) with float and nested condition */
    float bound = 250.0f;
    for (int i = 0; i < N; i++) {
        /* This should generate LT_EXPR pattern */
        fout1[i] = (arr_float[i] < bound) ? arr_float[i] : bound;
    }
    
    /* Loop 4: LE_EXPR (<=) with double and complex mask */
    double cap = 500.0;
    for (int i = 0; i < N; i++) {
        /* This should generate LE_EXPR pattern */
        dout[i] = (arr_double[i] <= cap) ? arr_double[i] : 0.0;
    }
    
    /* Additional loops with mixed comparisons and logical operators */
    
    /* Loop 5: GT_EXPR with signed char and logical AND */
    signed char low = -50, high = 50;
    for (int i = 0; i < N; i++) {
        /* Complex predicate that may decompose to GT_EXPR and LT_EXPR */
        if (arr_char[i] > low && arr_char[i] < high) {
            out3[i] = arr_char[i];
        } else {
            out3[i] = 0;
        }
    }
    
    /* Loop 6: GE_EXPR and LE_EXPR with logical OR */
    float x = -100.0f, y = 100.0f;
    for (int i = 0; i < N; i++) {
        /* This may generate both GE_EXPR and LE_EXPR patterns */
        if (arr_float[i] <= x || arr_float[i] >= y) {
            fout2[i] = arr_float[i] * 2.0f;
        } else {
            fout2[i] = arr_float[i];
        }
    }
    
    /* Loop 7: LT_EXPR with while-style loop condition emulation */
    int temp[N];
    for (int i = 0; i < N; i++) {
        temp[i] = arr_int[i];
    }
    
    int iterations = 0;
    for (int i = 0; i < N; i++) {
        /* Simulate while condition with LT_EXPR */
        while (temp[i] < 0 && iterations < 10) {
            temp[i] += 10;
            iterations++;
        }
        out4[i] = temp[i];
    }
    
    /* Loop 8: LE_EXPR with floating-point and fast-math pattern */
    float epsilon = 0.001f;
    for (int i = 0; i < N; i++) {
        /* Pattern that might trigger special handling */
        float diff = arr_float[i] - bound;
        if (diff <= epsilon && diff >= -epsilon) {
            fout1[i] = 0.0f;  /* Reset to zero if close to bound */
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i];
        checksum += (long long)fout1[i];
        checksum += (long long)fout2[i];
        checksum += (long long)dout[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
