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
    unsigned int uarr[N];
    float farr[N];
    double darr[N];
    char carr[N];
    
    /* Output arrays for results */
    int out1[N], out2[N], out3[N], out4[N];
    int out5[N], out6[N], out7[N], out8[N];
    float fout[N];
    double dout[N];
    
    /* Initialize arrays with deterministic values */
    lcg = SEED;
    for (int i = 0; i < N; i++) {
        arr1[i] = rand_int(-100, 100);
        arr2[i] = rand_int(-50, 150);
        arr3[i] = rand_int(0, 200);
        arr4[i] = rand_int(-1000, 1000);
        uarr[i] = (unsigned int)rand_int(0, 255);
        farr[i] = rand_float(-10.0f, 10.0f);
        darr[i] = (double)rand_float(-20.0f, 20.0f);
        carr[i] = (char)rand_int(-128, 127);
    }
    
    /* Loop 1: GT_EXPR (>) with integer array */
    int threshold1 = 25;
    for (int i = 0; i < N; i++) {
        /* Direct ternary with > comparison */
        out1[i] = (arr1[i] > threshold1) ? arr1[i] : 0;
        
        /* Additional if statement with > */
        if (arr2[i] > -10) {
            out2[i] = arr2[i] * 2;
        } else {
            out2[i] = arr2[i];
        }
    }
    
    /* Loop 2: GE_EXPR (>=) with float array */
    float limit = 2.5f;
    for (int i = 0; i < N; i++) {
        /* Ternary with >= comparison */
        fout[i] = (farr[i] >= limit) ? farr[i] : limit;
        
        /* Mask-based computation with >= */
        int mask = (farr[i] >= -limit) ? 1 : 0;
        out3[i] = mask * (int)farr[i];
    }
    
    /* Loop 3: LT_EXPR (<) with unsigned array */
    unsigned int bound = 100;
    for (int i = 0; i < N; i++) {
        /* Ternary with < comparison */
        out4[i] = (uarr[i] < bound) ? (int)uarr[i] : (int)bound;
        
        /* Nested conditionals with < */
        if (uarr[i] < 50) {
            out5[i] = 1;
        } else if (uarr[i] < 150) {
            out5[i] = 2;
        } else {
            out5[i] = 3;
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with double array */
    double cap = 5.0;
    for (int i = 0; i < N; i++) {
        /* Ternary with <= comparison */
        dout[i] = (darr[i] <= cap) ? darr[i] : cap;
        
        /* Complex predicate with <= */
        if (darr[i] <= 0.0 || darr[i] >= 10.0) {
            out6[i] = 100;
        } else {
            out6[i] = (int)(darr[i] * 10.0);
        }
    }
    
    /* Loop 5: Mixed comparisons with char array */
    char low = -50, high = 50;
    for (int i = 0; i < N; i++) {
        /* Combined > and < in logical AND */
        if (carr[i] > low && carr[i] < high) {
            out7[i] = carr[i] * 2;
        } else {
            out7[i] = carr[i];
        }
        
        /* Combined <= and >= in logical OR */
        if (carr[i] <= -100 || carr[i] >= 100) {
            out8[i] = 255;
        } else {
            out8[i] = carr[i] + 128;
        }
    }
    
    /* Loop 6: All four comparisons in one loop for completeness */
    int val1 = -30, val2 = 30, val3 = -20, val4 = 20;
    int temp_out[N];
    for (int i = 0; i < N; i++) {
        int result = 0;
        
        /* GT_EXPR */
        if (arr3[i] > val1) result += 1;
        
        /* GE_EXPR */
        if (arr3[i] >= val2) result += 2;
        
        /* LT_EXPR */
        if (arr3[i] < val3) result += 4;
        
        /* LE_EXPR */
        if (arr3[i] <= val4) result += 8;
        
        temp_out[i] = result;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i];
        checksum += out5[i] + out6[i] + out7[i] + out8[i];
        checksum += (int)fout[i] + (int)dout[i] + temp_out[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
