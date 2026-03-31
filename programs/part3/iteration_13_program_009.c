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
    
    int out1[N], out2[N], out3[N], out4[N];
    float fout[N];
    double dout[N];
    unsigned int uout[N];
    
    /* Initialize with deterministic values */
    for (int i = 0; i < N; i++) {
        arr1[i] = rand_int(-1000, 1000);
        arr2[i] = rand_int(-500, 500);
        arr3[i] = rand_int(0, 1000);
        arr4[i] = rand_int(-200, 200);
        uarr[i] = (unsigned int)rand_int(0, 2000);
        farr[i] = rand_float(-100.0f, 100.0f);
        darr[i] = (double)rand_float(-200.0, 200.0);
        carr[i] = (char)rand_int(-128, 127);
    }
    
    /* Loop 1: GT_EXPR (>) with integer mask pattern */
    int threshold1 = 100;
    for (int i = 0; i < N; i++) {
        /* Direct ternary with > comparison */
        out1[i] = (arr1[i] > threshold1) ? arr1[i] : 0;
        
        /* Additional > comparison in nested conditional */
        if (arr2[i] > -200 && arr1[i] > threshold1) {
            out1[i] += 5;
        }
    }
    
    /* Loop 2: GE_EXPR (>=) with floating-point mask pattern */
    float limit = 25.5f;
    for (int i = 0; i < N; i++) {
        /* Ternary with >= comparison */
        fout[i] = (farr[i] >= limit) ? farr[i] : limit;
        
        /* Complex predicate with >= */
        if (farr[i] >= limit || farr[i] <= -limit) {
            fout[i] *= 1.1f;
        }
    }
    
    /* Loop 3: LT_EXPR (<) with unsigned integer and mixed types */
    unsigned int bound = 1000;
    int low = 200;
    for (int i = 0; i < N; i++) {
        /* Multiple < comparisons */
        uout[i] = (uarr[i] < bound) ? uarr[i] : bound;
        
        /* Nested condition with < */
        if (arr3[i] < 800 && uarr[i] < bound) {
            uout[i] += (arr3[i] < low) ? 10 : 5;
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with double and character types */
    double cap = 150.0;
    char max_char = 100;
    for (int i = 0; i < N; i++) {
        /* Ternary with <= on double */
        dout[i] = (darr[i] <= cap) ? darr[i] : cap;
        
        /* Additional <= comparison with char */
        if (carr[i] <= max_char && darr[i] <= cap * 2) {
            dout[i] += (carr[i] <= 0) ? 0.5 : 0.25;
        }
    }
    
    /* Loop 5: Mixed comparisons in same loop for complex pattern */
    int out5[N];
    int high = 300, low2 = -300;
    for (int i = 0; i < N; i++) {
        /* Combined > and < in logical AND */
        out5[i] = (arr4[i] > low2 && arr4[i] < high) ? arr4[i] : 0;
        
        /* Combined <= and >= in logical OR */
        if (arr4[i] <= low2 || arr4[i] >= high) {
            out5[i] = -arr4[i];
        }
    }
    
    /* Loop 6: GT_EXPR with floating-point and type conversion */
    float threshold2 = -50.0f;
    int out6[N];
    for (int i = 0; i < N; i++) {
        /* > comparison with float to int conversion */
        out6[i] = (farr[i] > threshold2) ? (int)farr[i] : 0;
    }
    
    /* Loop 7: LE_EXPR with signed/unsigned mixing */
    int out7[N];
    for (int i = 0; i < N; i++) {
        /* <= with signed/unsigned comparison */
        out7[i] = ((int)uarr[i] <= arr1[i]) ? uarr[i] : arr1[i];
    }
    
    /* Compute checksums to prevent dead code elimination */
    long long checksum1 = 0, checksum2 = 0, checksum3 = 0, checksum4 = 0;
    float fchecksum = 0.0f;
    double dchecksum = 0.0;
    unsigned long long uchecksum = 0;
    long long checksum5 = 0, checksum6 = 0, checksum7 = 0;
    
    for (int i = 0; i < N; i++) {
        checksum1 += out1[i];
        checksum2 += out2[i];
        checksum3 += out3[i];
        checksum4 += out4[i];
        fchecksum += fout[i];
        dchecksum += dout[i];
        uchecksum += uout[i];
        checksum5 += out5[i];
        checksum6 += out6[i];
        checksum7 += out7[i];
    }
    
    /* Print checksums (prevents optimization) */
    printf("Checksums: %lld %lld %lld %lld %f %f %llu %lld %lld %lld\n",
           checksum1, checksum2, checksum3, checksum4,
           fchecksum, dchecksum, uchecksum,
           checksum5, checksum6, checksum7);
    
    return 0;
}
