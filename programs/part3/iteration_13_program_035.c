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
    char carr[N];
    
    /* Output arrays */
    int out1[N] = {0}, out2[N] = {0}, out3[N] = {0}, out4[N] = {0};
    float fout[N] = {0.0f};
    double dout[N] = {0.0};
    int mask_out[N] = {0};
    
    /* Initialize with deterministic values */
    lcg = SEED;
    for (int i = 0; i < N; i++) {
        arr1[i] = rand_int(-100, 100);
        arr2[i] = rand_int(0, 200);
        arr3[i] = rand_int(-50, 50);
        arr4[i] = rand_int(-1000, 1000);
        uarr[i] = (unsigned int)rand_int(0, 300);
        farr[i] = rand_float(-10.0f, 10.0f);
        darr[i] = (double)rand_float(-20.0, 20.0);
        carr[i] = (char)rand_int(-128, 127);
    }
    
    /* Loop 1: GT_EXPR (>) with integer array */
    /* Pattern: result[i] = (data[i] > threshold) ? data[i] : 0 */
    const int threshold1 = 25;
    for (int i = 0; i < N; i++) {
        out1[i] = (arr1[i] > threshold1) ? arr1[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with float array */
    /* Pattern: if (data[i] >= limit) accumulate */
    const float limit = 2.5f;
    float sum = 0.0f;
    for (int i = 0; i < N; i++) {
        if (farr[i] >= limit) {
            sum += farr[i];
            fout[i] = farr[i];
        } else {
            fout[i] = 0.0f;
        }
    }
    
    /* Loop 3: LT_EXPR (<) with unsigned array */
    /* Pattern: mask-based selection with swap */
    const unsigned int bound = 150;
    for (int i = 0; i < N; i++) {
        /* This should trigger std::swap(cond_expr0, cond_expr1) for LT_EXPR */
        out3[i] = (uarr[i] < bound) ? (int)uarr[i] : 0;
    }
    
    /* Loop 4: LE_EXPR (<=) with double array */
    /* Pattern: conditional assignment with <= */
    const double cap = 5.0;
    for (int i = 0; i < N; i++) {
        dout[i] = (darr[i] <= cap) ? darr[i] : 0.0;
    }
    
    /* Loop 5: Mixed comparisons with nested conditionals */
    /* Combines GT_EXPR and LT_EXPR with logical AND */
    const int low = -30;
    const int high = 30;
    for (int i = 0; i < N; i++) {
        /* if (arr3[i] > low && arr3[i] < high) */
        if (arr3[i] > low && arr3[i] < high) {
            out4[i] = arr3[i];
        } else {
            out4[i] = 0;
        }
    }
    
    /* Loop 6: LE_EXPR and GE_EXPR with logical OR */
    /* if (arr4[i] <= -500 || arr4[i] >= 500) */
    const int x = -500;
    const int y = 500;
    for (int i = 0; i < N; i++) {
        if (arr4[i] <= x || arr4[i] >= y) {
            mask_out[i] = 1;
        } else {
            mask_out[i] = 0;
        }
    }
    
    /* Loop 7: Signed char with GT_EXPR */
    /* Pattern: ternary with > */
    const char char_threshold = 0;
    int char_sum = 0;
    for (int i = 0; i < N; i++) {
        char_sum += (carr[i] > char_threshold) ? carr[i] : 0;
    }
    
    /* Loop 8: Complex mask computation with GE_EXPR and LE_EXPR */
    /* Creates a mask that selects between two arrays */
    for (int i = 0; i < N; i++) {
        /* Equivalent to: mask = (arr1[i] >= 0) ? arr1[i] : arr2[i] */
        out2[i] = (arr1[i] >= 0) ? arr1[i] : arr2[i];
    }
    
    /* Compute checksums to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i] + mask_out[i];
        checksum += (long long)fout[i];
        checksum += (long long)dout[i];
    }
    checksum += (long long)sum + char_sum;
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
