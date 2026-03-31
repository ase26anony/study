#include <stdio.h>
#include <stdlib.h>

#define N 1024

/* Simple deterministic pseudo-random generator */
static unsigned seed = 12345;
static unsigned rand_int(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

static float rand_float(void) {
    return (float)rand_int() / (float)(1U << 31);
}

int main(void) {
    /* Declare arrays with mixed types */
    int arr1[N], arr2[N], arr3[N], arr4[N];
    float farr[N];
    unsigned uarr[N];
    char carr[N];
    
    int out1[N], out2[N], out3[N], out4[N];
    float fout[N];
    int checksum = 0;
    
    /* Initialize arrays with deterministic values */
    for (int i = 0; i < N; i++) {
        arr1[i] = (int)(rand_int() % 2000) - 1000;   /* Signed int: -1000..999 */
        arr2[i] = (int)(rand_int() % 2000) - 1000;
        arr3[i] = (int)(rand_int() % 2000) - 1000;
        arr4[i] = (int)(rand_int() % 2000) - 1000;
        farr[i] = rand_float() * 2000.0f - 1000.0f;  /* Float: -1000..1000 */
        uarr[i] = rand_int() % 2000;                 /* Unsigned int: 0..1999 */
        carr[i] = (char)(rand_int() % 256) - 128;    /* Signed char: -128..127 */
    }
    
    /* Loop 1: GT_EXPR (>) with integer mask */
    /* Pattern: result = (data > threshold) ? data : 0 */
    const int threshold1 = 100;
    for (int i = 0; i < N; i++) {
        /* This should generate GT_EXPR tree code */
        out1[i] = (arr1[i] > threshold1) ? arr1[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with floating-point mask */
    /* Pattern: result = (data >= limit) ? data : constant */
    const float limit = 250.5f;
    for (int i = 0; i < N; i++) {
        /* This should generate GE_EXPR tree code */
        fout[i] = (farr[i] >= limit) ? farr[i] : -1.0f;
    }
    
    /* Loop 3: LT_EXPR (<) with unsigned integer and nested condition */
    /* Pattern: if (data < bound) mask-based operation */
    const unsigned bound = 1500;
    for (int i = 0; i < N; i++) {
        /* This should generate LT_EXPR tree code */
        if (uarr[i] < bound) {
            out2[i] = uarr[i] * 2;
        } else {
            out2[i] = uarr[i];
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with signed char and complex predicate */
    /* Pattern: result = (data <= cap && data > min) ? data : default */
    const char cap = 50;
    const char min = -30;
    for (int i = 0; i < N; i++) {
        /* This should generate LE_EXPR tree code (and GT_EXPR for the other part) */
        out3[i] = (carr[i] <= cap && carr[i] > min) ? carr[i] : -100;
    }
    
    /* Additional loop: Mixed comparisons in same loop */
    /* Uses all four operators in different contexts */
    const int low = -500;
    const int high = 500;
    for (int i = 0; i < N; i++) {
        /* GT_EXPR */
        int val1 = (arr3[i] > 0) ? arr3[i] : 0;
        
        /* GE_EXPR */
        int val2 = (arr3[i] >= low) ? arr3[i] : low;
        
        /* LT_EXPR in logical OR */
        if (arr4[i] < -800 || arr4[i] > 800) {
            out4[i] = arr4[i] * 3;
        } else {
            /* LE_EXPR in ternary */
            out4[i] = (arr4[i] <= high) ? arr4[i] : high;
        }
    }
    
    /* Loop with floating-point comparisons using all operators */
    float float_out[N];
    const float f_low = -500.0f;
    const float f_high = 500.0f;
    for (int i = 0; i < N; i++) {
        /* GT_EXPR and LT_EXPR combined */
        if (farr[i] > f_low && farr[i] < f_high) {
            float_out[i] = farr[i];
        } else {
            /* GE_EXPR or LE_EXPR alternative */
            float_out[i] = (farr[i] >= 0.0f) ? farr[i] * 0.5f : farr[i] * 2.0f;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i] + (int)float_out[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
