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
    lcg = SEED;
    for (int i = 0; i < N; i++) {
        arr_int[i] = rand_int(-1000, 1000);
        arr_uint[i] = (unsigned int)rand_int(0, 2000);
        arr_float[i] = rand_float(-500.0f, 500.0f);
        arr_double[i] = (double)rand_float(-500.0f, 500.0f);
        arr_char[i] = (signed char)rand_int(-128, 127);
    }
    
    /* Loop 1: GT_EXPR (>) with integer type and ternary operator */
    int threshold1 = 100;
    for (int i = 0; i < N; i++) {
        /* This should generate GT_EXPR pattern */
        out1[i] = (arr_int[i] > threshold1) ? arr_int[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with unsigned integer and if statement */
    unsigned int limit = 500;
    for (int i = 0; i < N; i++) {
        /* This should generate GE_EXPR pattern */
        if (arr_uint[i] >= limit) {
            out2[i] = arr_uint[i];
        } else {
            out2[i] = limit / 2;
        }
    }
    
    /* Loop 3: LT_EXPR (<) with floating-point and nested condition */
    float bound = 250.0f;
    for (int i = 0; i < N; i++) {
        /* This should generate LT_EXPR pattern */
        fout1[i] = (arr_float[i] < bound) ? arr_float[i] : bound;
    }
    
    /* Loop 4: LE_EXPR (<=) with double and complex condition */
    double cap = -100.0;
    for (int i = 0; i < N; i++) {
        /* This should generate LE_EXPR pattern */
        dout[i] = (arr_double[i] <= cap) ? arr_double[i] : cap * 2.0;
    }
    
    /* Additional loops with mixed comparisons and logical operators */
    
    /* Loop 5: GT_EXPR with signed char and logical AND */
    signed char low = -50;
    signed char high = 50;
    for (int i = 0; i < N; i++) {
        /* Complex predicate that may decompose to individual comparisons */
        if (arr_char[i] > low && arr_char[i] < high) {
            out3[i] = arr_char[i] * 2;
        } else {
            out3[i] = arr_char[i];
        }
    }
    
    /* Loop 6: GE_EXPR and LE_EXPR with logical OR */
    float x = -200.0f;
    float y = 200.0f;
    for (int i = 0; i < N; i++) {
        /* OR condition that may expose both GE_EXPR and LE_EXPR */
        if (arr_float[i] <= x || arr_float[i] >= y) {
            fout2[i] = arr_float[i] * 0.5f;
        } else {
            fout2[i] = arr_float[i];
        }
    }
    
    /* Loop 7: LT_EXPR with while-style loop condition simulation */
    int temp[N];
    int val = 0;
    for (int i = 0; i < N; i++) {
        /* Simulate while condition pattern */
        int j = 0;
        do {
            temp[j] = (arr_int[i] < threshold1 + j) ? 1 : 0;
            j++;
        } while (j < 4);
        out4[i] = temp[0] + temp[1] + temp[2] + temp[3];
    }
    
    /* Loop 8: All four comparisons in one loop with different data types */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        /* Mix all comparison operators */
        int cmp1 = (arr_int[i] > 0) ? 1 : 0;          /* GT_EXPR */
        int cmp2 = (arr_uint[i] >= 1000) ? 2 : 0;    /* GE_EXPR */
        int cmp3 = (arr_float[i] < 0.0f) ? 4 : 0;    /* LT_EXPR */
        int cmp4 = (arr_double[i] <= 0.0) ? 8 : 0;   /* LE_EXPR */
        checksum += cmp1 + cmp2 + cmp3 + cmp4;
    }
    
    /* Compute final checksum to prevent dead code elimination */
    long long final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += out1[i] + out2[i] + out3[i] + out4[i];
        final_sum += (long long)fout1[i] + (long long)fout2[i];
        final_sum += (long long)dout[i];
    }
    final_sum += checksum;
    
    printf("Checksum: %lld\n", final_sum);
    
    return 0;
}
