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
    int low = 200;
    for (int i = 0; i < N; i++) {
        /* Complex predicate with LT_EXPR */
        if (arr_uint[i] < bound && arr_int[i] > low) {
            out2[i] = arr_uint[i];
        } else {
            out2[i] = 0;
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with double precision */
    double cap = 250.0;
    for (int i = 0; i < N; i++) {
        /* Pattern using if statement instead of ternary */
        if (arr_double[i] <= cap) {
            dout[i] = arr_double[i] * 2.0;
        } else {
            dout[i] = arr_double[i];
        }
    }
    
    /* Additional loops with mixed types and operators */
    
    /* Loop 5: GT_EXPR with signed char and logical OR */
    char min_val = -50;
    char max_val = 50;
    for (int i = 0; i < N; i++) {
        /* Pattern: (a > x || a < y) ? a : 0 */
        out3[i] = (arr_char[i] > max_val || arr_char[i] < min_val) ? arr_char[i] : 0;
    }
    
    /* Loop 6: GE_EXPR with accumulation pattern */
    float sum = 0.0f;
    float threshold2 = -100.0f;
    for (int i = 0; i < N; i++) {
        /* Accumulate only if condition is met */
        sum += (arr_float[i] >= threshold2) ? arr_float[i] : 0.0f;
    }
    
    /* Loop 7: LT_EXPR with while-style loop and array output */
    int j = 0;
    int limit2 = 500;
    while (j < N) {
        /* Using while to potentially trigger different vectorization paths */
        out4[j] = (arr_int[j] < limit2) ? arr_int[j] * 2 : arr_int[j];
        j++;
    }
    
    /* Loop 8: LE_EXPR with nested ternary */
    int threshold3 = -300;
    int threshold4 = 300;
    for (int i = 0; i < N; i++) {
        /* Complex nested conditional */
        fout2[i] = (arr_int[i] <= threshold3) ? -arr_float[i] :
                   (arr_int[i] >= threshold4) ? arr_float[i] : 0.0f;
    }
    
    /* Compute checksums to prevent dead code elimination */
    long long checksum_int = 0;
    float checksum_float = 0.0f;
    double checksum_double = 0.0;
    
    for (int i = 0; i < N; i++) {
        checksum_int += out1[i] + out2[i] + out3[i] + out4[i];
        checksum_float += fout1[i] + fout2[i];
        checksum_double += dout[i];
    }
    
    checksum_float += sum;
    
    printf("Checksums: int=%lld, float=%f, double=%f\n", 
           checksum_int, checksum_float, checksum_double);
    
    return 0;
}
