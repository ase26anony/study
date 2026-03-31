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
    int arr1[N], arr2[N], arr3[N], arr4[N];
    unsigned int uarr[N];
    float farr[N];
    double darr[N];
    
    /* Output arrays for each comparison pattern */
    int out_gt[N], out_ge[N], out_lt[N], out_le[N];
    int out_mixed[N];
    float fout_gt[N], fout_le[N];
    
    /* Initialize arrays with deterministic values */
    for (int i = 0; i < N; i++) {
        arr1[i] = rand_int(-100, 100);
        arr2[i] = rand_int(0, 200);
        arr3[i] = rand_int(-50, 50);
        arr4[i] = rand_int(-1000, 1000);
        uarr[i] = (unsigned int)rand_int(0, 255);
        farr[i] = rand_float(-10.0f, 10.0f);
        darr[i] = (double)rand_float(-5.0, 5.0);
    }
    
    /* Loop 1: GT_EXPR pattern (> comparison) */
    int threshold_gt = 25;
    for (int i = 0; i < N; i++) {
        /* Direct ternary with > operator */
        out_gt[i] = (arr1[i] > threshold_gt) ? arr1[i] : 0;
    }
    
    /* Loop 2: GE_EXPR pattern (>= comparison) */
    int limit_ge = -10;
    for (int i = 0; i < N; i++) {
        /* Mask-based computation with >= */
        if (arr2[i] >= limit_ge) {
            out_ge[i] = arr2[i] * 2;
        } else {
            out_ge[i] = arr2[i] / 2;
        }
    }
    
    /* Loop 3: LT_EXPR pattern (< comparison) */
    int bound_lt = 30;
    for (int i = 0; i < i + 1; i++) {  /* Simple loop to enable vectorization */
        if (i >= N) break;
        /* Using < in conditional assignment */
        out_lt[i] = (arr3[i] < bound_lt) ? arr3[i] + 100 : arr3[i] - 100;
    }
    
    /* Loop 4: LE_EXPR pattern (<= comparison) */
    int cap_le = 75;
    for (int i = 0; i < N; i++) {
        /* Complex expression with <= */
        out_le[i] = (arr4[i] <= cap_le) ? arr4[i] * 3 : arr4[i];
    }
    
    /* Mixed type comparisons to exercise different semantics */
    
    /* Unsigned integer with <= (LE_EXPR with unsigned) */
    unsigned int ucap = 128;
    for (int i = 0; i < N; i++) {
        out_mixed[i] = (uarr[i] <= ucap) ? (int)uarr[i] : 0;
    }
    
    /* Floating-point with > (GT_EXPR with float) */
    float fthreshold = 2.5f;
    for (int i = 0; i < N; i++) {
        fout_gt[i] = (farr[i] > fthreshold) ? farr[i] : 0.0f;
    }
    
    /* Floating-point with <= (LE_EXPR with double) */
    double dlimit = 1.0;
    for (int i = 0; i < N; i++) {
        fout_le[i] = (darr[i] <= dlimit) ? (float)darr[i] : 1.0f;
    }
    
    /* Nested conditionals with logical operators */
    int low = -20, high = 20;
    for (int i = 0; i < N; i++) {
        /* Combined > and < with && */
        if (arr1[i] > low && arr1[i] < high) {
            out_mixed[i] += 50;
        }
    }
    
    /* Another nested pattern with || */
    int x = -30, y = 30;
    for (int i = 0; i < N; i++) {
        /* Combined <= and >= with || */
        if (arr3[i] <= x || arr3[i] >= y) {
            out_mixed[i] -= 25;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out_gt[i] + out_ge[i] + out_lt[i] + out_le[i];
        checksum += out_mixed[i] + (int)fout_gt[i] + (int)fout_le[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
