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
    return min + ((float)(lcg % 10000) / 10000.0f) * (max - min);
}

int main() {
    /* Declare arrays with different types */
    int arr1[N], arr2[N], arr3[N], arr4[N];
    unsigned int uarr[N];
    float farr[N];
    double darr[N];
    signed char carr[N];
    
    /* Output arrays */
    int out1[N] = {0}, out2[N] = {0}, out3[N] = {0}, out4[N] = {0};
    float fout[N] = {0.0f};
    double dout[N] = {0.0};
    int mixed_out[N] = {0};
    
    /* Initialize with deterministic values */
    lcg = SEED;
    for (int i = 0; i < N; i++) {
        arr1[i] = rand_int(-100, 100);
        arr2[i] = rand_int(-50, 150);
        arr3[i] = rand_int(0, 200);
        arr4[i] = rand_int(-200, 200);
        uarr[i] = (unsigned int)rand_int(0, 255);
        farr[i] = rand_float(-10.0f, 10.0f);
        darr[i] = (double)rand_float(-20.0, 20.0);
        carr[i] = (signed char)rand_int(-128, 127);
    }
    
    /* Loop 1: GT_EXPR (>) with integer array */
    int threshold1 = 25;
    for (int i = 0; i < N; i++) {
        /* Direct ternary with > comparison */
        out1[i] = (arr1[i] > threshold1) ? arr1[i] : 0;
        
        /* Additional if statement with > to increase coverage */
        if (arr2[i] > -10) {
            out1[i] += 1;
        }
    }
    
    /* Loop 2: GE_EXPR (>=) with floating-point array */
    float limit = 2.5f;
    for (int i = 0; i < N; i++) {
        /* Mask-based computation with >= */
        fout[i] = (farr[i] >= limit) ? farr[i] : 0.0f;
        
        /* Nested condition with >= */
        if (farr[i] >= -limit && farr[i] <= limit * 2) {
            fout[i] *= 2.0f;
        }
    }
    
    /* Loop 3: LT_EXPR (<) with unsigned integer array */
    unsigned int bound = 100;
    for (int i = 0; i < N; i++) {
        /* Complex predicate with < operator */
        if (uarr[i] < bound || uarr[i] > 200) {
            out2[i] = uarr[i];
        } else {
            out2[i] = bound;
        }
        
        /* Additional < comparison in loop condition */
        int j = 0;
        while (j < 4) {
            out2[i] += j;
            j++;
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with double array */
    double cap = 5.0;
    for (int i = 0; i < N; i++) {
        /* Direct <= comparison controlling assignment */
        dout[i] = (darr[i] <= cap) ? darr[i] : cap;
        
        /* Combined comparisons with <= */
        if (darr[i] <= -cap || darr[i] >= cap) {
            dout[i] = -dout[i];
        }
    }
    
    /* Loop 5: Mixed comparisons with signed char */
    signed char low = -50;
    signed char high = 50;
    for (int i = 0; i < N; i++) {
        /* Nested comparisons using both < and > */
        if (carr[i] > low && carr[i] < high) {
            out3[i] = carr[i] * 2;
        } else if (carr[i] <= low) {
            out3[i] = low;
        } else if (carr[i] >= high) {
            out3[i] = high;
        }
    }
    
    /* Loop 6: Complex pattern with all four operators */
    int x = -30, y = 30, z = 60;
    for (int i = 0; i < N; i++) {
        /* Multiple comparisons in logical expression */
        if ((arr4[i] > x && arr4[i] <= y) || (arr4[i] < z && arr4[i] >= -z)) {
            out4[i] = arr4[i];
        }
        
        /* Additional mask computation */
        int temp = (arr4[i] > 0) ? arr4[i] : -arr4[i];
        out4[i] += (temp <= 100) ? temp : 100;
    }
    
    /* Loop 7: Floating-point with mixed comparisons for vectorization */
    float f_low = -3.0f, f_high = 3.0f;
    for (int i = 0; i < N; i++) {
        /* Pattern that should convert to bitwise mask operations */
        float val = farr[i];
        float result = 0.0f;
        
        if (val > f_low && val < f_high) {
            result = val;
        } else if (val <= f_low) {
            result = f_low;
        } else if (val >= f_high) {
            result = f_high;
        }
        
        mixed_out[i] = (int)(result * 100.0f);
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i] + mixed_out[i];
        checksum += (long long)fout[i];
        checksum += (long long)dout[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
