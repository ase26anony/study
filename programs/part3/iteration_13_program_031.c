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
    return min + ((float)(lcg & 0x7FFFFFFF) / (float)0x7FFFFFFF) * (max - min);
}

int main() {
    /* Declare arrays with different types */
    int arr1[N], arr2[N], arr3[N], arr4[N];
    float farr1[N], farr2[N];
    unsigned int uarr[N];
    signed char sarr[N];
    
    int out1[N], out2[N], out3[N], out4[N];
    float fout1[N], fout2[N];
    int mask_out[N];
    
    /* Initialize with deterministic values */
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
    
    /* Loop 1: GT_EXPR (>) with integer mask pattern */
    int threshold1 = 100;
    for (int i = 0; i < N; i++) {
        /* Direct ternary with > comparison */
        out1[i] = (arr1[i] > threshold1) ? arr1[i] : 0;
        
        /* Additional if statement with > to ensure pattern */
        if (farr1[i] > 50.0f) {
            fout1[i] = farr1[i] * 2.0f;
        } else {
            fout1[i] = farr1[i];
        }
    }
    
    /* Loop 2: GE_EXPR (>=) with mixed types */
    int limit = -50;
    float float_limit = -25.0f;
    for (int i = 0; i < N; i++) {
        /* Integer >= with mask */
        out2[i] = (arr2[i] >= limit) ? arr2[i] * 2 : arr2[i];
        
        /* Float >= in conditional assignment */
        float temp = (farr2[i] >= float_limit) ? farr2[i] : 0.0f;
        fout2[i] = temp + 1.0f;
        
        /* Unsigned int >= */
        mask_out[i] = (uarr[i] >= 500) ? 1 : 0;
    }
    
    /* Loop 3: LT_EXPR (<) with nested conditionals */
    int low = 200, high = 800;
    float bound = 75.0f;
    for (int i = 0; i < N; i++) {
        /* Simple < comparison */
        out3[i] = (arr3[i] < high) ? arr3[i] : high;
        
        /* Nested with < and > */
        if (arr3[i] < high && arr3[i] > low) {
            out3[i] += 100;
        }
        
        /* Float < with logical OR */
        if (farr1[i] < -bound || farr1[i] > bound) {
            fout1[i] = -fout1[i];
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with signed char and complex predicate */
    int cap = 0;
    signed char char_limit = 64;
    for (int i = 0; i < N; i++) {
        /* Simple <= with signed char */
        out4[i] = (sarr[i] <= char_limit) ? sarr[i] * 3 : sarr[i];
        
        /* Integer <= in compound condition */
        if (arr4[i] <= cap || arr4[i] >= -cap) {
            out4[i] = arr4[i] * 2;
        }
        
        /* Float <= with arithmetic */
        fout2[i] += (farr2[i] <= 25.0f) ? 5.0f : 0.0f;
    }
    
    /* Additional loop: Mixed comparisons in same loop */
    int results[N];
    for (int i = 0; i < N; i++) {
        int val = 0;
        /* Chain of different comparisons */
        if (arr1[i] > 0) val += 1;
        if (arr2[i] >= 0) val += 2;
        if (arr3[i] < 500) val += 4;
        if (arr4[i] <= 0) val += 8;
        if (uarr[i] > 750) val += 16;
        if (sarr[i] < 0) val += 32;
        results[i] = val;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i];
        checksum += (int)fout1[i] + (int)fout2[i];
        checksum += mask_out[i] + results[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
