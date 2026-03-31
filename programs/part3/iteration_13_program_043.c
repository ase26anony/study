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
    int out_mixed1[N], out_mixed2[N];
    float fout_gt[N], fout_le[N];
    
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
    
    /* Loop 1: GT_EXPR pattern (> comparisons) */
    int threshold1 = 25;
    for (int i = 0; i < N; i++) {
        /* Direct ternary with > */
        out_gt[i] = (arr1[i] > threshold1) ? arr1[i] : 0;
        
        /* Nested condition with > */
        if (arr2[i] > -10 && arr2[i] < 100) {
            out_gt[i] += arr2[i];
        }
    }
    
    /* Loop 2: GE_EXPR pattern (>= comparisons) */
    int limit = 50;
    for (int i = 0; i < N; i++) {
        /* Mask-based computation with >= */
        out_ge[i] = (arr2[i] >= limit) ? arr2[i] * 2 : arr2[i];
        
        /* Complex predicate with >= */
        if (arr3[i] >= 0 || arr1[i] >= threshold1) {
            out_ge[i] += 1;
        }
    }
    
    /* Loop 3: LT_EXPR pattern (< comparisons) */
    int bound = 75;
    for (int i = 0; i < N; i++) {
        /* Ternary with < */
        out_lt[i] = (arr3[i] < bound) ? arr3[i] : bound;
        
        /* Nested comparisons with < */
        if (arr4[i] < 0 && arr1[i] < threshold1) {
            out_lt[i] -= arr4[i];
        }
    }
    
    /* Loop 4: LE_EXPR pattern (<= comparisons) */
    int cap = 100;
    for (int i = 0; i < N; i++) {
        /* Direct <= in conditional */
        if (arr4[i] <= cap) {
            out_le[i] = arr4[i] * 3;
        } else {
            out_le[i] = cap;
        }
        
        /* Combined with other comparison */
        if (arr2[i] <= limit || arr3[i] <= bound) {
            out_le[i] += 5;
        }
    }
    
    /* Loop 5: Mixed signed/unsigned with LE_EXPR */
    unsigned int ucap = 128;
    for (int i = 0; i < N; i++) {
        /* Unsigned <= comparison */
        out_mixed1[i] = (uarr[i] <= ucap) ? (int)uarr[i] : 0;
    }
    
    /* Loop 6: Floating-point GT_EXPR and LE_EXPR */
    float fthreshold = 2.5f;
    double dlimit = 1.0;
    for (int i = 0; i < N; i++) {
        /* Float > comparison */
        fout_gt[i] = (farr[i] > fthreshold) ? farr[i] : fthreshold;
        
        /* Double <= comparison */
        if (darr[i] <= dlimit && darr[i] >= -dlimit) {
            fout_le[i] = (float)darr[i];
        } else {
            fout_le[i] = 0.0f;
        }
    }
    
    /* Loop 7: Complex nested comparisons triggering multiple patterns */
    int low = -30, high = 80;
    for (int i = 0; i < N; i++) {
        /* This may decompose into GT_EXPR and LT_EXPR */
        if (arr1[i] > low && arr1[i] < high) {
            out_mixed2[i] = arr1[i];
        } else if (arr1[i] <= low || arr1[i] >= high) {
            out_mixed2[i] = 0;
        }
        
        /* Additional GE_EXPR pattern */
        out_mixed2[i] += (arr2[i] >= 0) ? 1 : -1;
    }
    
    /* Loop 8: While loop with < comparison */
    int temp[N];
    for (int i = 0; i < N; i++) temp[i] = i;
    int j = 0;
    while (j < N) {
        /* While condition uses < */
        if (temp[j] < N/2) {
            out_gt[j] += temp[j];
        }
        j++;
    }
    
    /* Loop 9: For loop with <= in condition */
    int sum = 0;
    for (int i = 0; i <= N-1; i++) {  /* Note: <= in loop condition */
        sum += (arr1[i] <= threshold1) ? arr1[i] : 0;
    }
    
    /* Compute checksums to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out_gt[i] + out_ge[i] + out_lt[i] + out_le[i];
        checksum += out_mixed1[i] + out_mixed2[i];
        checksum += (long long)fout_gt[i] + (long long)fout_le[i];
    }
    checksum += sum;
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
