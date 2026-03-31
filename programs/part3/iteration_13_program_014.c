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
    
    /* Loop 1: GT_EXPR (>) with integer type and ternary operator */
    int threshold1 = 100;
    for (int i = 0; i < N; i++) {
        /* This should generate GT_EXPR pattern */
        out1[i] = (arr_int[i] > threshold1) ? arr_int[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with unsigned integer and if statement */
    unsigned int limit = 1000;
    for (int i = 0; i < N; i++) {
        /* This should generate GE_EXPR pattern */
        if (arr_uint[i] >= limit) {
            out2[i] = arr_uint[i];
        } else {
            out2[i] = limit;
        }
    }
    
    /* Loop 3: LT_EXPR (<) with floating point and nested condition */
    float bound = 250.0f;
    for (int i = 0; i < N; i++) {
        /* This should generate LT_EXPR pattern */
        fout1[i] = (arr_float[i] < bound) ? arr_float[i] : bound;
        
        /* Additional nested conditional to create complex predicate */
        if (arr_float[i] < -bound && arr_float[i] > -500.0f) {
            fout1[i] = -bound;
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with double and logical OR */
    double cap = 300.0;
    for (int i = 0; i < N; i++) {
        /* This should generate LE_EXPR pattern */
        dout[i] = (arr_double[i] <= cap) ? arr_double[i] : cap;
        
        /* Combined with OR operator */
        if (arr_double[i] <= -cap || arr_double[i] >= cap * 2) {
            dout[i] = 0.0;
        }
    }
    
    /* Loop 5: Mixed comparisons with signed char */
    signed char low = -50;
    signed char high = 50;
    for (int i = 0; i < N; i++) {
        /* Complex condition with both GT and LT */
        if (arr_char[i] > low && arr_char[i] < high) {
            out3[i] = arr_char[i];
        } else {
            out3[i] = 0;
        }
    }
    
    /* Loop 6: GE_EXPR with floating point and mask computation */
    float threshold2 = -100.0f;
    for (int i = 0; i < N; i++) {
        /* Direct mask-based computation */
        fout2[i] = arr_float[i];
        if (arr_float[i] >= threshold2) {
            fout2[i] *= 2.0f;
        }
    }
    
    /* Loop 7: LE_EXPR with integer in while-like loop pattern */
    int max_val = 500;
    for (int i = 0; i < N; i++) {
        int temp = arr_int[i];
        /* Simulate while condition pattern */
        while (temp <= max_val && temp > -max_val) {
            temp += 10;
            if (temp > max_val * 2) break;
        }
        out4[i] = temp;
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
