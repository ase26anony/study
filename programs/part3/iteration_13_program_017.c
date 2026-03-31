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
    int arr1[N], arr2[N], arr3[N], arr4[N];
    unsigned int uarr[N];
    float farr[N];
    double darr[N];
    signed char carr[N];
    
    /* Output arrays for each comparison type */
    int out_gt[N], out_ge[N], out_lt[N], out_le[N];
    int out_mixed1[N], out_mixed2[N];
    float fout_gt[N], fout_ge[N];
    
    /* Initialize arrays with deterministic values */
    lcg = SEED;
    for (int i = 0; i < N; i++) {
        arr1[i] = rand_int(-100, 100);
        arr2[i] = rand_int(-50, 150);
        arr3[i] = rand_int(-200, 200);
        arr4[i] = rand_int(-100, 100);
        uarr[i] = (unsigned int)rand_int(0, 255);
        farr[i] = rand_float(-10.0f, 10.0f);
        darr[i] = (double)rand_float(-5.0f, 5.0f);
        carr[i] = (signed char)rand_int(-128, 127);
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
        /* Mask-based computation with < */
        out_lt[i] = (uarr[i] < bound) ? uarr[i] * 2 : uarr[i];
    }
    
    /* Loop 4: LE_EXPR (<=) with double precision */
    double cap = 1.0;
    for (int i = 0; i < N; i++) {
        /* Complex expression with <= */
        fout_gt[i] = (darr[i] <= cap) ? (float)(darr[i] * 3.0) : (float)darr[i];
    }
    
    /* Loop 5: Mixed comparisons with logical AND (nested conditionals) */
    int low = -30, high = 70;
    for (int i = 0; i < N; i++) {
        /* Combined > and < with && */
        if (arr2[i] > low && arr2[i] < high) {
            out_mixed1[i] = arr2[i] * 3;
        } else {
            out_mixed1[i] = arr2[i];
        }
    }
    
    /* Loop 6: Mixed comparisons with logical OR */
    int x = -20, y = 80;
    for (int i = 0; i < N; i++) {
        /* Combined <= and >= with || */
        if (arr3[i] <= x || arr3[i] >= y) {
            out_mixed2[i] = arr3[i] / 2;
        } else {
            out_mixed2[i] = arr3[i];
        }
    }
    
    /* Loop 7: Signed char with > comparison */
    signed char c_threshold = 0;
    for (int i = 0; i < N; i++) {
        /* Ternary with signed char > */
        out_ge[i] = (carr[i] > c_threshold) ? carr[i] * 2 : carr[i];
    }
    
    /* Loop 8: Nested ternary with <= and >= */
    int min_val = -10, max_val = 10;
    for (int i = 0; i < N; i++) {
        /* Complex conditional expression */
        out_le[i] = (arr4[i] <= min_val) ? -1 : 
                   (arr4[i] >= max_val) ? 1 : 0;
    }
    
    /* Loop 9: While loop with < comparison (alternative loop structure) */
    int j = 0;
    while (j < N) {
        /* While loop with < in condition */
        if (arr1[j] < 0) {
            out_gt[j] = -out_gt[j];
        }
        j++;
    }
    
    /* Loop 10: For loop with > in increment condition */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        /* Accumulation with conditional */
        sum += (arr1[i] > 0) ? arr1[i] : 0;
    }
    
    /* Compute checksums to prevent dead code elimination */
    long long checksum = 0;
    float fchecksum = 0.0f;
    
    for (int i = 0; i < N; i++) {
        checksum += out_gt[i] + out_ge[i] + out_lt[i] + out_le[i];
        checksum += out_mixed1[i] + out_mixed2[i];
        fchecksum += fout_gt[i] + fout_ge[i];
    }
    
    printf("Integer checksum: %lld\n", checksum);
    printf("Float checksum: %f\n", fchecksum);
    printf("Conditional sum: %d\n", sum);
    
    return 0;
}
