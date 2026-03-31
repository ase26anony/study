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
    
    /* Initialize arrays with deterministic values */
    lcg = SEED;
    for (int i = 0; i < N; i++) {
        arr_int[i] = rand_int(-1000, 1000);
        arr_uint[i] = (unsigned int)rand_int(0, 2000);
        arr_float[i] = rand_float(-500.0f, 500.0f);
        arr_double[i] = (double)rand_float(-500.0f, 500.0f);
        arr_char[i] = (signed char)rand_int(-128, 127);
    }
    
    /* Loop 1: GT_EXPR (>) with integer mask-based computation */
    int threshold1 = 100;
    for (int i = 0; i < N; i++) {
        /* Direct ternary with > comparison */
        out1[i] = (arr_int[i] > threshold1) ? arr_int[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with floating-point and logical operators */
    float limit = 250.0f;
    for (int i = 0; i < N; i++) {
        /* Complex condition with >= and logical OR */
        if (arr_float[i] >= limit || arr_float[i] <= -limit) {
            fout1[i] = arr_float[i] * 2.0f;
        } else {
            fout1[i] = arr_float[i];
        }
    }
    
    /* Loop 3: LT_EXPR (<) with signed char and nested conditionals */
    signed char low = -50, high = 50;
    for (int i = 0; i < N; i++) {
        /* Nested comparisons using < and > */
        if (arr_char[i] < high && arr_char[i] > low) {
            out2[i] = arr_char[i] * 10;
        } else {
            out2[i] = arr_char[i];
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with unsigned integers */
    unsigned int cap = 1500;
    for (int i = 0; i < N; i++) {
        /* Ternary with <= comparison */
        uout[i] = (arr_uint[i] <= cap) ? arr_uint[i] : cap;
    }
    
    /* Additional loops to ensure all patterns are exercised */
    
    /* Mixed GT_EXPR and LE_EXPR in same loop */
    int threshold2 = -200;
    for (int i = 0; i < N; i++) {
        /* Creates both > and <= comparisons */
        out3[i] = (arr_int[i] > threshold2) ? 
                 ((arr_int[i] <= 500) ? arr_int[i] : 500) : threshold2;
    }
    
    /* GE_EXPR with floating-point mask */
    double dlimit = 300.0;
    for (int i = 0; i < N; i++) {
        /* Direct comparison with >= */
        dout[i] = (arr_double[i] >= dlimit) ? arr_double[i] : 0.0;
    }
    
    /* LT_EXPR in while-loop style computation */
    int bound = 800;
    int j = 0;
    while (j < N) {
        /* Using < comparison in loop condition and body */
        if (arr_int[j] < bound) {
            out4[j] = arr_int[j] * 2;
        } else {
            out4[j] = arr_int[j];
        }
        j++;
    }
    
    /* Compute checksums to prevent dead code elimination */
    long long checksum_int = 0;
    unsigned long long checksum_uint = 0;
    double checksum_float = 0.0;
    double checksum_double = 0.0;
    
    for (int i = 0; i < N; i++) {
        checksum_int += out1[i] + out2[i] + out3[i] + out4[i];
        checksum_uint += uout[i];
        checksum_float += fout1[i];
        checksum_double += dout[i];
    }
    
    /* Print checksums (use volatile to prevent optimization) */
    volatile long long v1 = checksum_int;
    volatile unsigned long long v2 = checksum_uint;
    volatile double v3 = checksum_float;
    volatile double v4 = checksum_double;
    
    printf("Checksums: %lld %llu %.2f %.2f\n", 
           v1, v2, v3, v4);
    
    return 0;
}
