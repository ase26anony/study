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
    
    /* Output arrays */
    int out1[N] = {0}, out2[N] = {0}, out3[N] = {0}, out4[N] = {0};
    float fout[N] = {0.0f};
    double dout[N] = {0.0};
    
    /* Initialize with deterministic values */
    for (int i = 0; i < N; i++) {
        arr1[i] = rand_int(-100, 100);
        arr2[i] = rand_int(-50, 150);
        arr3[i] = rand_int(0, 200);
        arr4[i] = rand_int(-1000, 1000);
        uarr[i] = (unsigned int)rand_int(0, 255);
        farr[i] = rand_float(-10.0f, 10.0f);
        darr[i] = (double)rand_float(-20.0f, 20.0f);
    }
    
    /* Loop 1: GT_EXPR (>) with integer mask */
    int threshold1 = 25;
    for (int i = 0; i < N; i++) {
        /* Direct ternary with > comparison */
        out1[i] = (arr1[i] > threshold1) ? arr1[i] : 0;
        
        /* Additional if statement with > to ensure pattern */
        if (arr2[i] > -10) {
            out1[i] += arr2[i];
        }
    }
    
    /* Loop 2: GE_EXPR (>=) with floating-point mask */
    float limit = 2.5f;
    for (int i = 0; i < N; i++) {
        /* Ternary with >= comparison */
        fout[i] = (farr[i] >= limit) ? farr[i] : 0.0f;
        
        /* Nested condition with >= */
        if (i >= N/2 && farr[i] >= -limit) {
            fout[i] *= 2.0f;
        }
    }
    
    /* Loop 3: LT_EXPR (<) with unsigned integer */
    unsigned int bound = 128;
    for (int i = 0; i < N; i++) {
        /* Mask using < comparison */
        out2[i] = (uarr[i] < bound) ? (int)uarr[i] : 0;
        
        /* Complex predicate with < */
        if (arr3[i] < 100 && uarr[i] < 200) {
            out2[i] += arr3[i];
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with double precision */
    double cap = 5.0;
    for (int i = 0; i < N; i++) {
        /* Ternary with <= comparison */
        dout[i] = (darr[i] <= cap) ? darr[i] : 0.0;
        
        /* Logical OR with <= */
        if (darr[i] <= -cap || darr[i] <= 0.0) {
            dout[i] += 1.0;
        }
    }
    
    /* Loop 5: Mixed comparisons in nested loop */
    int low = -30, high = 70;
    for (int i = 0; i < N; i++) {
        /* Combined GT and LT (range check) */
        int in_range = (arr4[i] > low) && (arr4[i] < high);
        out3[i] = in_range ? arr4[i] : -arr4[i];
        
        /* Additional LE comparison */
        if (arr1[i] <= threshold1) {
            out3[i] += arr1[i];
        }
    }
    
    /* Loop 6: GE and LE with logical OR */
    int x = -50, y = 50;
    for (int i = 0; i < N; i++) {
        /* GE or LE comparison */
        if (arr4[i] <= x || arr4[i] >= y) {
            out4[i] = 1;
        } else {
            out4[i] = (arr2[i] >= 0) ? arr2[i] : 0;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i];
        checksum += (long long)fout[i];
        checksum += (long long)dout[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
