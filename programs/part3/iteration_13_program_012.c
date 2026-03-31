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
    float farr1[N], farr2[N];
    unsigned int uarr[N];
    signed char sarr[N];
    
    int out1[N], out2[N], out3[N], out4[N];
    float fout1[N], fout2[N];
    unsigned int uout[N];
    signed char sout[N];
    
    /* Initialize arrays with deterministic values */
    lcg = SEED;
    for (int i = 0; i < N; i++) {
        arr1[i] = rand_int(-100, 100);
        arr2[i] = rand_int(-50, 150);
        arr3[i] = rand_int(-200, 200);
        arr4[i] = rand_int(-100, 100);
        farr1[i] = rand_float(-10.0f, 10.0f);
        farr2[i] = rand_float(-5.0f, 15.0f);
        uarr[i] = (unsigned int)rand_int(0, 255);
        sarr[i] = (signed char)rand_int(-128, 127);
    }
    
    /* Loop 1: GT_EXPR (>) with integer mask selection */
    int threshold1 = 25;
    for (int i = 0; i < N; i++) {
        /* Direct ternary with > comparison */
        out1[i] = (arr1[i] > threshold1) ? arr1[i] : 0;
        
        /* Additional if statement with > to increase exposure */
        if (arr2[i] > -10) {
            out1[i] += 1;
        }
    }
    
    /* Loop 2: GE_EXPR (>=) with floating point and logical OR */
    float limit = 2.5f;
    for (int i = 0; i < N; i++) {
        /* Ternary with >= comparison */
        fout1[i] = (farr1[i] >= limit) ? farr1[i] : 0.0f;
        
        /* Complex condition with || to potentially decompose */
        if (farr1[i] >= -limit || farr2[i] >= 0.0f) {
            fout1[i] *= 2.0f;
        }
    }
    
    /* Loop 3: LT_EXPR (<) with unsigned integers */
    unsigned int bound = 128;
    for (int i = 0; i < N; i++) {
        /* Mask-based computation with < */
        uout[i] = (uarr[i] < bound) ? uarr[i] : bound;
        
        /* Nested conditionals with < */
        if (uarr[i] < 64) {
            uout[i] += 10;
        } else if (uarr[i] < 192) {
            uout[i] += 5;
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with signed char and complex predicate */
    signed char cap = 50;
    int low = -30, high = 30;
    for (int i = 0; i < N; i++) {
        /* Direct <= comparison */
        sout[i] = (sarr[i] <= cap) ? sarr[i] : cap;
        
        /* Compound condition with && that may decompose */
        if (sarr[i] <= high && sarr[i] >= low) {
            sout[i] = 0;
        }
    }
    
    /* Additional loops with mixed comparisons for more coverage */
    
    /* Loop 5: Mixed comparisons in same loop */
    int out5[N];
    int val1 = 10, val2 = 20;
    for (int i = 0; i < N; i++) {
        /* Multiple comparisons that could be vectorized separately */
        int cond1 = (arr3[i] > val1) ? 1 : 0;
        int cond2 = (arr3[i] < val2) ? 1 : 0;
        int cond3 = (arr3[i] >= 0) ? 1 : 0;
        int cond4 = (arr3[i] <= 100) ? 1 : 0;
        
        out5[i] = cond1 + cond2 + cond3 + cond4;
    }
    
    /* Loop 6: While loop with < comparison */
    int out6[N];
    int j = 0;
    while (j < N) {
        out6[j] = (arr4[j] < 0) ? -arr4[j] : arr4[j];
        j++;
    }
    
    /* Loop 7: Nested loops with <= in inner loop */
    int out7[N];
    int block_size = 16;
    for (int block = 0; block < N; block += block_size) {
        for (int k = 0; k < block_size; k++) {
            int idx = block + k;
            if (idx <= N - 1) {
                out7[idx] = (arr1[idx] <= arr2[idx]) ? arr1[idx] : arr2[idx];
            }
        }
    }
    
    /* Compute checksums to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i] + out5[i] + out6[i] + out7[i];
        checksum += (int)uout[i] + (int)sout[i];
        checksum += (int)fout1[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
