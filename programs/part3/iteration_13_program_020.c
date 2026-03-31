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

int main(void) {
    /* Declare arrays with different types */
    int arr1[N], arr2[N], arr3[N], arr4[N];
    unsigned int uarr[N];
    float farr[N];
    double darr[N];
    
    /* Output arrays for each comparison type */
    int out_gt[N], out_ge[N], out_lt[N], out_le[N];
    int out_mixed1[N], out_mixed2[N];
    float fout_gt[N], fout_ge[N];
    
    /* Initialize with deterministic values */
    lcg = SEED;
    for (int i = 0; i < N; i++) {
        arr1[i] = rand_int(-100, 100);
        arr2[i] = rand_int(-50, 150);
        arr3[i] = rand_int(0, 200);
        arr4[i] = rand_int(-100, 100);
        uarr[i] = (unsigned int)rand_int(0, 255);
        farr[i] = rand_float(-10.0f, 10.0f);
        darr[i] = (double)rand_float(-5.0f, 5.0f);
    }
    
    /* Loop 1: GT_EXPR (>) with integer mask pattern */
    int threshold1 = 25;
    for (int i = 0; i < N; i++) {
        /* Direct ternary with > comparison */
        out_gt[i] = (arr1[i] > threshold1) ? arr1[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with floating-point mask pattern */
    float limit = 2.5f;
    for (int i = 0; i < N; i++) {
        /* Using if statement with >= comparison */
        if (farr[i] >= limit) {
            fout_ge[i] = farr[i] * 2.0f;
        } else {
            fout_ge[i] = farr[i];
        }
    }
    
    /* Loop 3: LT_EXPR (<) with unsigned integer */
    unsigned int bound = 128;
    for (int i = 0; i < N; i++) {
        /* Mask-based computation with < comparison */
        out_lt[i] = (uarr[i] < bound) ? (int)uarr[i] * 2 : (int)uarr[i];
    }
    
    /* Loop 4: LE_EXPR (<=) with double precision */
    double cap = 1.0;
    float sum = 0.0f;
    for (int i = 0; i < N; i++) {
        /* Accumulation with <= comparison */
        sum += (darr[i] <= cap) ? (float)darr[i] : 0.0f;
        fout_gt[i] = sum;  /* Store intermediate sum */
    }
    
    /* Loop 5: Mixed comparisons with logical operators */
    int low = -30, high = 70;
    for (int i = 0; i < N; i++) {
        /* Nested condition with > and < (decomposes to individual comparisons) */
        if (arr2[i] > low && arr2[i] < high) {
            out_mixed1[i] = arr2[i] * 3;
        } else {
            out_mixed1[i] = arr2[i];
        }
    }
    
    /* Loop 6: Complex predicate with >= and <= */
    int x = 20, y = 80;
    for (int i = 0; i < N; i++) {
        /* OR condition that may decompose to >= and <= */
        if (arr3[i] <= x || arr3[i] >= y) {
            out_mixed2[i] = arr3[i] + 100;
        } else {
            out_mixed2[i] = arr3[i] - 50;
        }
    }
    
    /* Loop 7: Signed char with > comparison */
    signed char carr[N];
    char cout[N];
    for (int i = 0; i < N; i++) {
        carr[i] = (signed char)(arr4[i] % 128);
        /* Simple > comparison with char */
        cout[i] = (carr[i] > 0) ? carr[i] : (char)0;
    }
    
    /* Loop 8: Multiple comparisons in same loop */
    int th1 = -10, th2 = 10;
    for (int i = 0; i < N; i++) {
        /* Two separate comparisons that should both be vectorized */
        int mask1 = (arr1[i] >= th1) ? 1 : 0;
        int mask2 = (arr1[i] <= th2) ? 1 : 0;
        out_le[i] = mask1 & mask2;  /* Combine masks */
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out_gt[i] + out_ge[i] + out_lt[i] + out_le[i];
        checksum += out_mixed1[i] + out_mixed2[i];
        checksum += (int)fout_gt[i] + (int)fout_ge[i];
        checksum += cout[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
