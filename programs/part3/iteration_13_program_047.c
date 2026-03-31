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
    int checksum = 0;
    
    /* Initialize arrays with deterministic values */
    for (int i = 0; i < N; i++) {
        arr_int[i] = rand_int(-1000, 1000);
        arr_uint[i] = (unsigned int)rand_int(0, 2000);
        arr_float[i] = rand_float(-500.0f, 500.0f);
        arr_double[i] = (double)rand_float(-500.0f, 500.0f);
        arr_char[i] = (signed char)rand_int(-128, 127);
    }
    
    /* Loop 1: GT_EXPR (>) with integer type and ternary operator */
    const int threshold1 = 100;
    for (int i = 0; i < N; i++) {
        /* This should generate GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR pattern */
        out1[i] = (arr_int[i] > threshold1) ? arr_int[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with unsigned int and if statement */
    const unsigned int limit = 500;
    for (int i = 0; i < N; i++) {
        /* This should generate GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR pattern */
        if (arr_uint[i] >= limit) {
            out2[i] = arr_uint[i];
        } else {
            out2[i] = limit / 2;
        }
    }
    
    /* Loop 3: LT_EXPR (<) with float and nested condition */
    const float bound = 250.0f;
    for (int i = 0; i < N; i++) {
        /* This should generate LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
        if (arr_float[i] < bound && i % 2 == 0) {
            fout1[i] = arr_float[i];
        } else {
            fout1[i] = bound;
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with double and complex mask */
    const double cap = -100.0;
    for (int i = 0; i < N; i++) {
        /* This should generate LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
        dout[i] = (arr_double[i] <= cap) ? arr_double[i] : (cap * 2.0);
    }
    
    /* Loop 5: Mixed comparisons with signed char */
    const signed char low = -50;
    const signed char high = 50;
    for (int i = 0; i < N; i++) {
        /* Combined GT and LT with logical AND */
        if (arr_char[i] > low && arr_char[i] < high) {
            out3[i] = arr_char[i] * 2;
        } else {
            out3[i] = arr_char[i];
        }
    }
    
    /* Loop 6: GE and LE with logical OR */
    const int x = -200;
    const int y = 200;
    for (int i = 0; i < N; i++) {
        /* Combined LE and GE with logical OR */
        if (arr_int[i] <= x || arr_int[i] >= y) {
            out4[i] = arr_int[i] * 3;
        } else {
            out4[i] = arr_int[i];
        }
    }
    
    /* Loop 7: Floating-point comparisons with both > and >= */
    const float f_thresh1 = -300.0f;
    const float f_thresh2 = 300.0f;
    for (int i = 0; i < N; i++) {
        /* Nested floating-point comparisons */
        if (arr_float[i] > f_thresh1) {
            fout2[i] = (arr_float[i] >= f_thresh2) ? arr_float[i] * 2.0f : arr_float[i];
        } else {
            fout2[i] = f_thresh1;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i];
        checksum += (int)fout1[i] + (int)fout2[i] + (int)dout[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
