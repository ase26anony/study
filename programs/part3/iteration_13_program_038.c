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
    const int threshold1 = 100;
    for (int i = 0; i < N; i++) {
        /* This should generate GT_EXPR pattern */
        out1[i] = (arr_int[i] > threshold1) ? arr_int[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with unsigned int and if statement */
    const unsigned int limit = 1000;
    for (int i = 0; i < N; i++) {
        /* This should generate GE_EXPR pattern */
        if (arr_uint[i] >= limit) {
            out2[i] = arr_uint[i];
        } else {
            out2[i] = limit;
        }
    }
    
    /* Loop 3: LT_EXPR (<) with float and complex conditional */
    const float bound = 250.0f;
    const float alt_value = -1.0f;
    for (int i = 0; i < N; i++) {
        /* This should generate LT_EXPR pattern */
        fout1[i] = (arr_float[i] < bound) ? arr_float[i] : alt_value;
    }
    
    /* Loop 4: LE_EXPR (<=) with double and nested condition */
    const double cap = 300.0;
    for (int i = 0; i < N; i++) {
        /* This should generate LE_EXPR pattern */
        if (arr_double[i] <= cap) {
            dout[i] = arr_double[i] * 2.0;
        } else {
            dout[i] = cap;
        }
    }
    
    /* Additional loops with mixed comparisons and logical operators */
    
    /* Loop 5: GT_EXPR with signed char and logical AND */
    const signed char low = -50;
    const signed char high = 50;
    for (int i = 0; i < N; i++) {
        /* Complex predicate that may decompose to individual comparisons */
        if (arr_char[i] > low && arr_char[i] < high) {
            out3[i] = arr_char[i] * 2;
        } else {
            out3[i] = arr_char[i];
        }
    }
    
    /* Loop 6: GE_EXPR and LE_EXPR with logical OR */
    const int x = -200;
    const int y = 200;
    for (int i = 0; i < i + 1 && i < N; i++) {  /* Ensure loop condition is simple */
        /* This may generate both GE_EXPR and LE_EXPR patterns */
        if (arr_int[i] <= x || arr_int[i] >= y) {
            out4[i] = 0;
        } else {
            out4[i] = arr_int[i];
        }
    }
    
    /* Loop 7: LT_EXPR with floating point and mask computation */
    const float zero = 0.0f;
    for (int i = 0; i < N; i++) {
        /* Direct mask-like computation */
        fout2[i] = (arr_float[i] < zero) ? -arr_float[i] : arr_float[i];
    }
    
    /* Loop 8: LE_EXPR in while-style loop with increment */
    int j = 0;
    int temp_out[N];
    const int max_val = 800;
    while (j < N) {
        /* Using <= in loop condition and body */
        if (arr_int[j] <= max_val) {
            temp_out[j] = arr_int[j] + max_val;
        } else {
            temp_out[j] = arr_int[j] - max_val;
        }
        j++;
    }
    
    /* Compute checksums to prevent dead code elimination */
    long long checksum1 = 0, checksum2 = 0, checksum3 = 0, checksum4 = 0;
    double checksum5 = 0.0, checksum6 = 0.0, checksum7 = 0.0;
    
    for (int i = 0; i < N; i++) {
        checksum1 += out1[i];
        checksum2 += out2[i];
        checksum3 += out3[i];
        checksum4 += out4[i];
        checksum5 += fout1[i];
        checksum6 += dout[i];
        checksum7 += fout2[i];
    }
    
    /* Print checksums to ensure all computations are used */
    printf("Checksum1: %lld\n", checksum1);
    printf("Checksum2: %lld\n", checksum2);
    printf("Checksum3: %lld\n", checksum3);
    printf("Checksum4: %lld\n", checksum4);
    printf("Checksum5: %f\n", checksum5);
    printf("Checksum6: %f\n", checksum6);
    printf("Checksum7: %f\n", checksum7);
    
    return 0;
}
