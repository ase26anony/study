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
        arr1[i] = rand_int(-1000, 1000);
        arr2[i] = rand_int(-500, 500);
        arr3[i] = rand_int(0, 1000);
        arr4[i] = rand_int(-200, 200);
        farr1[i] = rand_float(-100.0f, 100.0f);
        farr2[i] = rand_float(0.0f, 50.0f);
        uarr[i] = (unsigned int)rand_int(0, 1000);
        sarr[i] = (signed char)rand_int(-128, 127);
    }
    
    /* Loop 1: GT_EXPR (>) with integer mask selection */
    int threshold1 = 100;
    for (int i = 0; i < N; i++) {
        /* Direct ternary with > comparison */
        out1[i] = (arr1[i] > threshold1) ? arr1[i] : 0;
        
        /* Additional if statement with > to increase exposure */
        if (arr2[i] > -200) {
            out1[i] += 1;
        }
    }
    
    /* Loop 2: GE_EXPR (>=) with floating point and logical OR */
    float limit = 25.5f;
    for (int i = 0; i < N; i++) {
        /* Ternary with >= comparison */
        fout1[i] = (farr1[i] >= limit) ? farr1[i] : limit;
        
        /* Combined condition with || to potentially decompose */
        if (farr1[i] >= -limit || farr2[i] >= 10.0f) {
            fout1[i] *= 1.1f;
        }
    }
    
    /* Loop 3: LT_EXPR (<) with unsigned integers */
    unsigned int bound = 500;
    for (int i = 0; i < N; i++) {
        /* Mask-based computation with < */
        uout[i] = (uarr[i] < bound) ? uarr[i] * 2 : uarr[i];
        
        /* Nested condition with < */
        if (uarr[i] < 800) {
            if (uarr[i] < 300) {
                uout[i] += 5;
            }
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with signed char and complex predicate */
    signed char cap = 50;
    int low = -30, high = 30;
    for (int i = 0; i < N; i++) {
        /* Direct <= comparison in ternary */
        sout[i] = (sarr[i] <= cap) ? sarr[i] : cap;
        
        /* Combined condition with && using <= and >= */
        if (sarr[i] <= high && sarr[i] >= low) {
            sout[i] = 0;
        }
        
        /* Additional <= comparison in assignment */
        out4[i] = (arr4[i] <= 0) ? -arr4[i] : arr4[i];
    }
    
    /* Loop 5: Mixed comparisons in same loop to trigger multiple cases */
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    float fsum1 = 0.0f, fsum2 = 0.0f;
    
    for (int i = 0; i < N; i++) {
        /* GT_EXPR */
        sum1 += (arr1[i] > 0) ? arr1[i] : 0;
        
        /* GE_EXPR */
        sum2 += (arr2[i] >= -100) ? arr2[i] : -100;
        
        /* LT_EXPR with floating point */
        fsum1 += (farr1[i] < 0.0f) ? -farr1[i] : farr1[i];
        
        /* LE_EXPR */
        sum3 += (arr3[i] <= 750) ? arr3[i] : 750;
        
        /* Complex condition mixing < and > */
        if (arr4[i] < 100 && arr4[i] > -100) {
            sum4 += arr4[i];
        }
        
        /* Nested comparisons */
        if (uarr[i] <= 600) {
            if (uarr[i] >= 200) {
                fsum2 += uarr[i] / 10.0f;
            }
        }
    }
    
    /* Compute checksums to prevent dead code elimination */
    int total_int = 0;
    float total_float = 0.0f;
    unsigned int total_unsigned = 0;
    int total_char = 0;
    
    for (int i = 0; i < N; i++) {
        total_int += out1[i] + out4[i];
        total_float += fout1[i];
        total_unsigned += uout[i];
        total_char += sout[i];
    }
    
    total_int += sum1 + sum2 + sum3 + sum4;
    total_float += fsum1 + fsum2;
    
    printf("Checksums: int=%d, float=%.2f, unsigned=%u, char=%d\n",
           total_int, total_float, total_unsigned, total_char);
    
    return 0;
}
