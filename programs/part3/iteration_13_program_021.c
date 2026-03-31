#include <stdio.h>
#include <stdlib.h>

#define N 1024

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 12345;
static unsigned int rand_int(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

static float rand_float(void) {
    return (rand_int() % 1000) / 10.0f;
}

int main(void) {
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
        arr_int[i] = (int)(rand_int() % 2000) - 1000;
        arr_uint[i] = rand_int() % 2000;
        arr_float[i] = rand_float();
        arr_double[i] = arr_float[i] * 2.0;
        arr_char[i] = (signed char)((rand_int() % 256) - 128);
    }
    
    /* Loop 1: GT_EXPR (>) with integer type */
    int threshold = 100;
    for (int i = 0; i < N; i++) {
        /* Mask-based computation using > */
        out1[i] = (arr_int[i] > threshold) ? arr_int[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with floating-point type */
    float limit = 50.0f;
    for (int i = 0; i < N; i++) {
        /* Mask-based computation using >= */
        fout1[i] = (arr_float[i] >= limit) ? arr_float[i] : limit;
    }
    
    /* Loop 3: LT_EXPR (<) with unsigned integer */
    unsigned int bound = 1000;
    for (int i = 0; i < N; i++) {
        /* Mask-based computation using < */
        out2[i] = (arr_uint[i] < bound) ? (int)arr_uint[i] : (int)bound;
    }
    
    /* Loop 4: LE_EXPR (<=) with double precision */
    double cap = 75.0;
    for (int i = 0; i < N; i++) {
        /* Mask-based computation using <= */
        dout[i] = (arr_double[i] <= cap) ? arr_double[i] : cap;
    }
    
    /* Additional loops with mixed/complex conditions */
    
    /* Loop 5: GT_EXPR with signed char and nested condition */
    signed char low = -50;
    for (int i = 0; i < N; i++) {
        /* Complex predicate with > */
        if (arr_char[i] > low && arr_char[i] < 100) {
            out3[i] = arr_char[i];
        } else {
            out3[i] = 0;
        }
    }
    
    /* Loop 6: LE_EXPR with logical OR */
    float x = 25.0f, y = 75.0f;
    for (int i = 0; i < N; i++) {
        /* Condition with <= and >= combined with OR */
        fout2[i] = (arr_float[i] <= x || arr_float[i] >= y) ? 
                   arr_float[i] : (x + y) / 2.0f;
    }
    
    /* Loop 7: GE_EXPR with if-else (alternative to ternary) */
    int limit2 = -500;
    for (int i = 0; i < N; i++) {
        if (arr_int[i] >= limit2) {
            out4[i] = arr_int[i];
        } else {
            out4[i] = limit2;
        }
    }
    
    /* Loop 8: LT_EXPR with while-style loop (unrolled pattern) */
    int temp[N];
    for (int i = 0; i < N; i++) {
        temp[i] = 0;
        int j = 0;
        /* While condition with < operator */
        while (j < 4) {
            temp[i] += arr_int[(i + j) % N];
            j++;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i];
        checksum += (long long)fout1[i] + (long long)fout2[i];
        checksum += (long long)dout[i] + temp[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
