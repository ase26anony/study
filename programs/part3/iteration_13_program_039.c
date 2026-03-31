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
    
    /* Output arrays for results */
    int out1[N], out2[N], out3[N], out4[N];
    unsigned int uout[N];
    float fout[N];
    double dout[N];
    
    /* Initialize with deterministic values */
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
        
        /* Additional if statement with > for more coverage */
        if (arr2[i] > -10) {
            out1[i] += 1;
        }
    }
    
    /* Loop 2: GE_EXPR (>=) with floating-point and logical OR */
    float limit = 2.5f;
    int constant = 5;
    for (int i = 0; i < N; i++) {
        /* Ternary with >= comparison */
        fout[i] = (farr[i] >= limit) ? farr[i] : 0.0f;
        
        /* Nested condition with >= and logical OR */
        if (farr[i] >= limit || farr[i] <= -limit) {
            fout[i] *= 2.0f;
        }
    }
    
    /* Loop 3: LT_EXPR (<) with mixed types and mask */
    int bound = 100;
    for (int i = 0; i < N; i++) {
        /* Direct < comparison in ternary */
        out2[i] = (arr3[i] < bound) ? arr3[i] : bound;
        
        /* Complex condition with < and && */
        if (arr3[i] < bound && arr3[i] > -bound) {
            out2[i] += arr3[i];
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with unsigned and double types */
    unsigned int cap = 500;
    double max_val = 3.0;
    for (int i = 0; i < N; i++) {
        /* Unsigned <= comparison */
        uout[i] = (uarr[i] <= cap) ? uarr[i] : cap;
        
        /* Double <= comparison with mask */
        dout[i] = (darr[i] <= max_val) ? darr[i] : max_val;
        
        /* Additional <= in if condition */
        if (darr[i] <= max_val / 2.0) {
            dout[i] *= 0.5;
        }
    }
    
    /* Loop 5: Mixed comparisons in same loop for complex pattern */
    int low = -50, high = 50;
    for (int i = 0; i < N; i++) {
        /* Combined condition with > and < */
        out3[i] = (arr1[i] > low && arr1[i] < high) ? arr1[i] : 0;
        
        /* Another with >= and <= */
        out4[i] = (arr2[i] >= -30 && arr2[i] <= 30) ? arr2[i] : -1;
    }
    
    /* Loop 6: While loop with < comparison */
    int j = 0;
    int temp_sum = 0;
    while (j < N) {
        if (arr4[j] < 128) {
            temp_sum += arr4[j];
        }
        j++;
    }
    
    /* Loop 7: For loop with decreasing index and > comparison */
    int out5[N];
    for (int i = N-1; i >= 0; i--) {
        out5[i] = (arr1[i] > 0) ? arr1[i] : -arr1[i];
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i] + out5[i];
        checksum += (long long)uout[i];
        checksum += (long long)fout[i];
        checksum += (long long)dout[i];
    }
    checksum += temp_sum;
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
