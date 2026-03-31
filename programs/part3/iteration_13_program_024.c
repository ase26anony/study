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
    return min + (lcg / (float)UINT_MAX) * (max - min);
}

int main() {
    /* Declare arrays with mixed types */
    int arr1[N], arr2[N], arr3[N], arr4[N];
    unsigned int uarr[N];
    float farr[N];
    double darr[N];
    
    /* Output arrays for results */
    int out1[N], out2[N], out3[N], out4[N];
    float fout[N];
    double dout[N];
    int mask_out[N];
    
    /* Initialize with deterministic values */
    for (int i = 0; i < N; i++) {
        arr1[i] = rand_int(-100, 100);
        arr2[i] = rand_int(0, 200);
        arr3[i] = rand_int(-50, 50);
        arr4[i] = rand_int(-1000, 1000);
        uarr[i] = (unsigned int)rand_int(0, 500);
        farr[i] = rand_float(-10.0f, 10.0f);
        darr[i] = (double)rand_float(-20.0, 20.0);
    }
    
    /* Loop 1: GT_EXPR (>) with integer mask selection */
    int threshold1 = 25;
    for (int i = 0; i < N; i++) {
        /* Direct ternary with > comparison */
        out1[i] = (arr1[i] > threshold1) ? arr1[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with floating-point mask */
    float limit = 5.0f;
    for (int i = 0; i < N; i++) {
        /* Mask-based computation with >= */
        fout[i] = (farr[i] >= limit) ? farr[i] * 2.0f : farr[i];
    }
    
    /* Loop 3: LT_EXPR (<) with unsigned integer */
    unsigned int bound = 300;
    for (int i = 0; i < N; i++) {
        /* if statement with < comparison */
        if (uarr[i] < bound) {
            out2[i] = uarr[i] * 2;
        } else {
            out2[i] = uarr[i];
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with double precision */
    double cap = 10.0;
    for (int i = 0; i < N; i++) {
        /* Ternary with <= for double */
        dout[i] = (darr[i] <= cap) ? darr[i] + 1.0 : darr[i] - 1.0;
    }
    
    /* Additional loops with mixed comparisons and logical operators */
    
    /* Nested conditionals with GT_EXPR and LT_EXPR */
    int low = -20, high = 30;
    for (int i = 0; i < N; i++) {
        /* Complex predicate: arr3[i] > low && arr3[i] < high */
        if (arr3[i] > low && arr3[i] < high) {
            out3[i] = arr3[i] * 3;
        } else {
            out3[i] = arr3[i];
        }
    }
    
    /* Logical OR with GE_EXPR and LE_EXPR */
    int x = -40, y = 40;
    for (int i = 0; i < N; i++) {
        /* arr4[i] <= x || arr4[i] >= y */
        if (arr4[i] <= x || arr4[i] >= y) {
            out4[i] = 0;
        } else {
            out4[i] = arr4[i];
        }
    }
    
    /* Mixed signed/unsigned with GT_EXPR */
    signed char schars[N];
    for (int i = 0; i < N; i++) {
        schars[i] = (signed char)(rand_int(-128, 127));
    }
    for (int i = 0; i < N; i++) {
        mask_out[i] = (schars[i] > 0) ? schars[i] : -schars[i];
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i] + mask_out[i];
        checksum += (long long)fout[i];
        checksum += (long long)dout[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
