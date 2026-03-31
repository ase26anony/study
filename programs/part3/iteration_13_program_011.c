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
    return min + ((float)(lcg & 0x7FFF) / 32767.0f) * (max - min);
}

int main() {
    /* Declare arrays with different types */
    int arr1[N], arr2[N], arr3[N], arr4[N];
    unsigned int uarr[N];
    float farr[N];
    double darr[N];
    
    /* Output arrays for results */
    int out1[N] = {0}, out2[N] = {0}, out3[N] = {0}, out4[N] = {0};
    unsigned int uout[N] = {0};
    float fout[N] = {0};
    double dout[N] = {0};
    
    /* Initialize with deterministic values */
    lcg = SEED;
    for (int i = 0; i < N; i++) {
        arr1[i] = rand_int(-100, 100);
        arr2[i] = rand_int(-50, 150);
        arr3[i] = rand_int(-200, 200);
        arr4[i] = rand_int(0, 255);
        uarr[i] = (unsigned int)rand_int(0, 1000);
        farr[i] = rand_float(-10.0f, 10.0f);
        darr[i] = (double)rand_float(-5.0f, 5.0f);
    }
    
    /* Loop 1: GT_EXPR (>) with integer mask selection */
    int threshold1 = 25;
    for (int i = 0; i < N; i++) {
        /* Direct ternary with > comparison */
        out1[i] = (arr1[i] > threshold1) ? arr1[i] : 0;
        
        /* Additional if statement with > to increase coverage */
        if (arr2[i] > -10) {
            out1[i] += 1;
        }
    }
    
    /* Loop 2: GE_EXPR (>=) with floating-point and logical OR */
    float limit = 2.5f;
    for (int i = 0; i < N; i++) {
        /* Ternary with >= comparison */
        fout[i] = (farr[i] >= limit) ? farr[i] : 0.0f;
        
        /* Nested condition with >= and logical OR */
        if (farr[i] >= limit || farr[i] <= -limit) {
            fout[i] *= 2.0f;
        }
    }
    
    /* Loop 3: LT_EXPR (<) with unsigned integers and complex predicate */
    unsigned int bound = 500;
    int low = 100, high = 900;
    for (int i = 0; i < N; i++) {
        /* Ternary with < comparison */
        uout[i] = (uarr[i] < bound) ? uarr[i] : bound;
        
        /* Complex condition with < and > */
        if (uarr[i] < high && uarr[i] > low) {
            uout[i] += 10;
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with double and mask-based accumulation */
    double cap = 3.0;
    double sum = 0.0;
    for (int i = 0; i < N; i++) {
        /* Conditional assignment with <= */
        double val = (darr[i] <= cap) ? darr[i] : cap;
        dout[i] = val;
        
        /* Another <= comparison in if statement */
        if (darr[i] <= -cap) {
            dout[i] = -val;
        }
        sum += dout[i];
    }
    
    /* Additional loops with mixed types and comparisons */
    
    /* Loop 5: Mixed comparisons in same loop */
    int out5[N] = {0};
    for (int i = 0; i < N; i++) {
        /* Use all four comparisons in different contexts */
        if (arr3[i] > 0) {
            out5[i] += 1;
        }
        if (arr3[i] >= 50) {
            out5[i] += 2;
        }
        if (arr3[i] < -50) {
            out5[i] += 4;
        }
        if (arr3[i] <= -100) {
            out5[i] += 8;
        }
    }
    
    /* Loop 6: Character array with < and > comparisons */
    signed char carr[N];
    char cout[N] = {0};
    for (int i = 0; i < N; i++) {
        carr[i] = (signed char)(arr4[i] - 128);
        /* Multiple comparisons that should generate different tree codes */
        cout[i] = (carr[i] > 0) ? 'P' : 
                  (carr[i] < 0) ? 'N' : 'Z';
    }
    
    /* Loop 7: Nested loops with <= in inner loop */
    int matrix[32][32];
    int mout[32][32] = {0};
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            matrix[i][j] = i * 32 + j;
            /* Use <= in inner loop condition */
            if (matrix[i][j] <= 512) {
                mout[i][j] = matrix[i][j] * 2;
            }
        }
    }
    
    /* Compute checksums to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i] + out5[i];
        checksum += (long long)uout[i];
        checksum += (long long)fout[i];
        checksum += (long long)dout[i];
        checksum += cout[i];
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            checksum += mout[i][j];
        }
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
