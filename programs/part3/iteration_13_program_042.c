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
    
    /* Loop 3: LT_EXPR (<) with nested conditionals */
    int low = -200, high = 300;
    for (int i = 0; i < N; i++) {
        /* Complex predicate with LT_EXPR */
        if (arr_int[i] < high && arr_int[i] > low) {
            out2[i] = arr_int[i] * 2;
        } else {
            out2[i] = arr_int[i];
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with unsigned integers */
    unsigned int cap = 1500;
    for (int i = 0; i < N; i++) {
        /* Pattern using LE_EXPR in mask computation */
        uout[i] = (arr_uint[i] <= cap) ? arr_uint[i] : cap;
    }
    
    /* Loop 5: Mixed comparisons with logical OR (exposes GE_EXPR and LE_EXPR) */
    float x = -100.0f, y = 100.0f;
    for (int i = 0; i < N; i++) {
        /* Logical OR decomposition may expose both GE_EXPR and LE_EXPR */
        if (arr_float[i] <= x || arr_float[i] >= y) {
            fout2[i] = arr_float[i] * 0.5f;
        } else {
            fout2[i] = arr_float[i];
        }
    }
    
    /* Loop 6: LT_EXPR with signed char */
    signed char bound = 50;
    for (int i = 0; i < N; i++) {
        /* Direct comparison with LT_EXPR */
        out3[i] = (arr_char[i] < bound) ? (int)arr_char[i] * 3 : (int)arr_char[i];
    }
    
    /* Loop 7: GT_EXPR with double precision */
    double dthreshold = -250.0;
    for (int i = 0; i < N; i++) {
        /* Ternary with GT_EXPR on doubles */
        dout[i] = (arr_double[i] > dthreshold) ? arr_double[i] : dthreshold;
    }
    
    /* Loop 8: LE_EXPR in while-style loop simulation */
    int counter = 0;
    int limit_val = 500;
    for (int i = 0; i < N; i++) {
        /* Simulate while condition pattern */
        int val = arr_int[i];
        while (val <= limit_val && counter < 10) {
            val += 10;
            counter++;
        }
        out4[i] = val;
        counter = 0;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i];
        checksum += (long long)uout[i];
        checksum += (long long)fout1[i];
        checksum += (long long)fout2[i];
        checksum += (long long)dout[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    return 0;
}
