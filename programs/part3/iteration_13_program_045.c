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

int main(void) {
    /* Declare arrays with different types */
    int arr1[N], arr2[N], arr3[N], arr4[N];
    unsigned int uarr[N];
    float farr[N];
    double darr[N];
    
    /* Output arrays for each comparison type */
    int out_gt[N], out_ge[N], out_lt[N], out_le[N];
    int out_mixed[N];
    float fout_gt[N], fout_lt[N];
    
    /* Initialize arrays with deterministic values */
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
    
    /* Loop 1: GT_EXPR (>) with integer mask */
    int threshold1 = 25;
    for (int i = 0; i < N; i++) {
        /* Direct ternary with > comparison */
        out_gt[i] = (arr1[i] > threshold1) ? arr1[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with floating-point mask */
    float limit = 2.5f;
    for (int i = 0; i < N; i++) {
        /* if statement with >= comparison */
        if (farr[i] >= limit) {
            fout_gt[i] = farr[i];
        } else {
            fout_gt[i] = 0.0f;
        }
    }
    
    /* Loop 3: LT_EXPR (<) with unsigned integer */
    unsigned int bound = 128;
    for (int i = 0; i < N; i++) {
        /* Ternary with < comparison on unsigned */
        out_lt[i] = (uarr[i] < bound) ? (int)uarr[i] : 0;
    }
    
    /* Loop 4: LE_EXPR (<=) with double precision */
    double cap = 1.0;
    int constant = 7;
    for (int i = 0; i < N; i++) {
        /* Accumulation pattern with <= comparison */
        out_le[i] = (darr[i] <= cap) ? constant : 0;
    }
    
    /* Loop 5: Mixed comparisons with logical AND (GT and LT) */
    int low = -30, high = 70;
    for (int i = 0; i < N; i++) {
        /* Nested condition with > and < */
        if (arr2[i] > low && arr2[i] < high) {
            out_mixed[i] = arr2[i];
        } else {
            out_mixed[i] = 0;
        }
    }
    
    /* Loop 6: Mixed comparisons with logical OR (LE and GE) */
    int x = -20, y = 80;
    for (int i = 0; i < N; i++) {
        /* Complex predicate with <= and >= */
        out_mixed[i] += (arr3[i] <= x || arr3[i] >= y) ? 1 : 0;
    }
    
    /* Loop 7: LT_EXPR with floating-point and type conversion */
    float fbound = -1.0f;
    for (int i = 0; i < N; i++) {
        /* if-else with < comparison */
        if (farr[i] < fbound) {
            fout_lt[i] = farr[i] * 2.0f;
        } else {
            fout_lt[i] = farr[i];
        }
    }
    
    /* Loop 8: GE_EXPR with signed char (promoted to int) */
    signed char carr[N];
    for (int i = 0; i < N; i++) {
        carr[i] = (signed char)(arr4[i] % 128);
    }
    signed char c_threshold = 64;
    int char_sum = 0;
    for (int i = 0; i < N; i++) {
        /* Comparison with signed char (promotion happens) */
        char_sum += (carr[i] >= c_threshold) ? carr[i] : 0;
    }
    
    /* Loop 9: LE_EXPR in while-loop style */
    int j = 0;
    int sum_le = 0;
    while (j < N) {
        /* while condition uses <, body uses <= */
        if (arr4[j] <= -50) {
            sum_le += arr4[j];
        }
        j++;
    }
    
    /* Compute checksums to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out_gt[i] + out_ge[i] + out_lt[i] + out_le[i] + out_mixed[i];
        checksum += (long long)fout_gt[i] + (long long)fout_lt[i];
    }
    checksum += char_sum + sum_le;
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
