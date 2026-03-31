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
    char carr[N];
    
    /* Output arrays for each comparison type */
    int out_gt[N], out_ge[N], out_lt[N], out_le[N];
    int out_mixed1[N], out_mixed2[N];
    float fout_gt[N], fout_ge[N];
    
    /* Initialize arrays with deterministic values */
    for (int i = 0; i < N; i++) {
        arr1[i] = rand_int(-100, 100);
        arr2[i] = rand_int(-50, 150);
        arr3[i] = rand_int(0, 200);
        arr4[i] = rand_int(-200, 200);
        uarr[i] = (unsigned int)rand_int(0, 255);
        farr[i] = rand_float(-10.0f, 10.0f);
        darr[i] = (double)rand_float(-5.0f, 5.0f);
        carr[i] = (char)rand_int(-128, 127);
    }
    
    /* Loop 1: GT_EXPR (>) with integer mask pattern */
    int threshold1 = 25;
    for (int i = 0; i < N; i++) {
        /* Direct ternary with > comparison */
        out_gt[i] = (arr1[i] > threshold1) ? arr1[i] : 0;
        
        /* Additional > comparison in nested conditional */
        if (arr2[i] > -10 && arr2[i] < 10) {
            out_gt[i] += 1;
        }
    }
    
    /* Loop 2: GE_EXPR (>=) with floating-point mask */
    float limit = 2.5f;
    for (int i = 0; i < N; i++) {
        /* Floating-point >= comparison creating mask */
        fout_ge[i] = (farr[i] >= limit) ? farr[i] : limit;
        
        /* Combined with logical OR */
        if (farr[i] >= 5.0f || farr[i] <= -5.0f) {
            fout_ge[i] *= 2.0f;
        }
    }
    
    /* Loop 3: LT_EXPR (<) with unsigned integers */
    unsigned int bound = 128;
    for (int i = 0; i < N; i++) {
        /* Unsigned < comparison */
        out_lt[i] = (uarr[i] < bound) ? (int)uarr[i] : (int)bound;
        
        /* Nested < comparisons */
        if (uarr[i] < 64 && uarr[i] > 32) {
            out_lt[i] += 100;
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with double precision */
    double cap = 1.0;
    int sum_le = 0;
    for (int i = 0; i < N; i++) {
        /* Double <= comparison in accumulation */
        sum_le += (darr[i] <= cap) ? (int)(darr[i] * 100) : 0;
        
        /* Additional <= in conditional assignment */
        out_le[i] = (darr[i] <= -cap) ? -100 : 50;
    }
    
    /* Loop 5: Mixed comparisons with char type */
    char low = -50, high = 50;
    for (int i = 0; i < N; i++) {
        /* Complex predicate with > and < */
        out_mixed1[i] = (carr[i] > low && carr[i] < high) ? carr[i] * 2 : carr[i];
        
        /* Another with <= and >= */
        out_mixed2[i] = (carr[i] <= 0 || carr[i] >= 100) ? -1 : 1;
    }
    
    /* Loop 6: GT_EXPR with floating-point in while-style loop */
    float fthreshold = -1.0f;
    for (int i = 0; i < N; i++) {
        /* While-like condition simulated with if */
        float val = farr[i];
        while (val > fthreshold) {  /* This will vectorize as comparison */
            val -= 0.5f;
        }
        fout_gt[i] = val;
    }
    
    /* Loop 7: GE_EXPR with integer array and loop-invariant */
    int limit2 = 75;
    int temp[N];
    for (int i = 0; i < N; i++) {
        /* Simple >= mask pattern */
        temp[i] = (arr3[i] >= limit2) ? arr3[i] - limit2 : 0;
    }
    
    /* Loop 8: LT_EXPR with floating-point and logical AND */
    float lower = -3.0f, upper = 3.0f;
    for (int i = 0; i < N; i++) {
        /* Combined < and > comparisons */
        if (farr[i] < upper && farr[i] > lower) {
            farr[i] = (farr[i] * 2.0f);
        }
    }
    
    /* Loop 9: LE_EXPR with signed/unsigned mix */
    for (int i = 0; i < N; i++) {
        /* Mixed type comparison */
        if ((int)uarr[i] <= 200) {
            arr4[i] = uarr[i] % 10;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out_gt[i] + out_ge[i] + out_lt[i] + out_le[i];
        checksum += out_mixed1[i] + out_mixed2[i];
        checksum += (long long)fout_gt[i] + (long long)fout_ge[i];
        checksum += temp[i] + arr4[i];
    }
    checksum += sum_le;
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
