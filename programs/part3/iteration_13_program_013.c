#include <stdio.h>
#include <stdlib.h>

#define N 1024

/* Simple deterministic pseudo-random generator */
static unsigned seed = 12345;
static unsigned simple_rand(void) {
    seed = seed * 1103515245 + 12345;
    return (unsigned)(seed / 65536) % 32768;
}

int main(void) {
    /* Declare arrays with mixed types */
    int arr1[N], arr2[N], out1[N], out2[N], out3[N], out4[N];
    unsigned int uarr[N];
    float farr[N];
    double darr[N];
    float fout1[N], fout2[N];
    double dout1[N], dout2[N];
    
    /* Initialize arrays with deterministic values */
    for (int i = 0; i < N; i++) {
        arr1[i] = simple_rand() % 1000 - 500;      /* Signed int: -500 to 499 */
        arr2[i] = simple_rand() % 1000;            /* Positive int: 0 to 999 */
        uarr[i] = simple_rand() % 1000;            /* Unsigned int: 0 to 999 */
        farr[i] = (simple_rand() % 2000 - 1000) / 10.0f; /* Float: -100.0 to 99.9 */
        darr[i] = (simple_rand() % 2000 - 1000) / 5.0;   /* Double: -200.0 to 199.8 */
    }
    
    /* Loop 1: GT_EXPR (>) with signed integers - creates mask pattern */
    int threshold1 = 100;
    for (int i = 0; i < N; i++) {
        /* Ternary operator creating mask: (arr1[i] > threshold1) ? arr1[i] : 0 */
        out1[i] = (arr1[i] > threshold1) ? arr1[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with unsigned integers */
    unsigned int limit = 500;
    for (int i = 0; i < N; i++) {
        /* if statement with >= comparison */
        if (uarr[i] >= limit) {
            out2[i] = uarr[i];
        } else {
            out2[i] = limit / 2;
        }
    }
    
    /* Loop 3: LT_EXPR (<) with floating-point - nested in logical AND */
    float bound = 50.0f;
    float low = -25.0f;
    for (int i = 0; i < N; i++) {
        /* Complex predicate with LT_EXPR */
        if (farr[i] < bound && farr[i] > low) {
            fout1[i] = farr[i] * 2.0f;
        } else {
            fout1[i] = farr[i];
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with double precision */
    double cap = 100.0;
    for (int i = 0; i < N; i++) {
        /* Ternary with <= comparison */
        dout1[i] = (darr[i] <= cap) ? darr[i] : cap;
    }
    
    /* Additional loops to ensure all patterns are hit */
    
    /* Loop 5: GT_EXPR with mixed types in nested condition */
    int threshold2 = -200;
    for (int i = 0; i < N; i++) {
        /* Nested ternary with > comparison */
        out3[i] = (arr1[i] > threshold2) ? 
                  ((arr2[i] > 700) ? arr1[i] + arr2[i] : arr1[i]) : 
                  arr2[i];
    }
    
    /* Loop 6: LE_EXPR with logical OR */
    int upper = 800;
    int lower = 200;
    for (int i = 0; i < N; i++) {
        /* LE_EXPR in logical OR */
        if (arr2[i] <= lower || arr2[i] >= upper) {
            out4[i] = arr2[i] * 2;
        } else {
            out4[i] = arr2[i];
        }
    }
    
    /* Loop 7: GE_EXPR with floating point and arithmetic */
    float fthreshold = -10.0f;
    for (int i = 0; i < N; i++) {
        /* GE_EXPR controlling arithmetic */
        fout2[i] = (farr[i] >= fthreshold) ? farr[i] + 5.0f : farr[i] - 5.0f;
    }
    
    /* Loop 8: LT_EXPR with while-style loop emulation */
    double dlimit = 0.0;
    for (int i = 0; i < N; i++) {
        /* LT_EXPR in assignment with mask */
        dout2[i] = (darr[i] < dlimit) ? -darr[i] : darr[i];
    }
    
    /* Compute checksums to prevent dead code elimination */
    long long checksum = 0;
    double dchecksum = 0.0;
    float fchecksum = 0.0f;
    
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i];
        fchecksum += fout1[i] + fout2[i];
        dchecksum += dout1[i] + dout2[i];
    }
    
    /* Print checksums (volatile to prevent optimization) */
    volatile long long v_checksum = checksum;
    volatile float v_fchecksum = fchecksum;
    volatile double v_dchecksum = dchecksum;
    
    printf("Integer checksum: %lld\n", v_checksum);
    printf("Float checksum: %f\n", v_fchecksum);
    printf("Double checksum: %f\n", v_dchecksum);
    
    return 0;
}
