#include <stdio.h>
#include <stdlib.h>

#define N 1024

/* Simple deterministic pseudo-random generator */
static unsigned seed = 123456789;
static unsigned lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

int main(void) {
    /* Declare arrays with different types */
    int arr1[N], arr2[N], out1[N], out2[N];
    unsigned int uarr[N];
    float farr[N], fout[N];
    double darr[N], dout[N];
    char carr[N];
    int cout[N];
    
    /* Initialize arrays with deterministic values */
    for (int i = 0; i < N; i++) {
        arr1[i] = (int)(lcg_rand() % 2000) - 1000;  /* Signed int: -1000 to 999 */
        arr2[i] = (int)(lcg_rand() % 2000) - 1000;
        uarr[i] = lcg_rand() % 2000;                /* Unsigned int: 0 to 1999 */
        farr[i] = (float)(lcg_rand() % 2000) / 10.0f - 100.0f;  /* Float: -100.0 to 99.9 */
        darr[i] = (double)(lcg_rand() % 2000) / 10.0 - 100.0;   /* Double: -100.0 to 99.9 */
        carr[i] = (char)(lcg_rand() % 256) - 128;   /* Signed char: -128 to 127 */
    }
    
    long long checksum = 0;
    
    /* Loop 1: GT_EXPR (>) with integer mask pattern */
    int threshold1 = 100;
    for (int i = 0; i < N; i++) {
        /* Ternary operator creating mask pattern for > */
        out1[i] = (arr1[i] > threshold1) ? arr1[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with floating-point mask pattern */
    float limit = 50.0f;
    for (int i = 0; i < N; i++) {
        /* if statement with >= comparison */
        if (farr[i] >= limit) {
            fout[i] = farr[i] * 2.0f;
        } else {
            fout[i] = farr[i];
        }
    }
    
    /* Loop 3: LT_EXPR (<) with unsigned integer and nested condition */
    unsigned int bound = 1000;
    for (int i = 0; i < N; i++) {
        /* Complex predicate with < operator */
        if (uarr[i] < bound && uarr[i] > 100) {
            out2[i] = (int)uarr[i] * 2;
        } else if (uarr[i] < 50) {
            out2[i] = (int)uarr[i];
        } else {
            out2[i] = 0;
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with double precision */
    double cap = -25.0;
    for (int i = 0; i < N; i++) {
        /* Ternary with <= comparison */
        dout[i] = (darr[i] <= cap) ? darr[i] * 3.0 : darr[i];
    }
    
    /* Additional loops to ensure all operators are exercised */
    
    /* Loop 5: GT_EXPR with signed char and logical OR */
    char low = -50;
    for (int i = 0; i < N; i++) {
        /* Logical OR combining > comparisons */
        if (carr[i] > low || carr[i] > 75) {
            cout[i] = carr[i] + 100;
        } else {
            cout[i] = carr[i];
        }
    }
    
    /* Loop 6: GE_EXPR with integer accumulation */
    int sum_ge = 0;
    int limit2 = -500;
    for (int i = 0; i < N; i++) {
        /* Accumulation with mask from >= comparison */
        sum_ge += (arr2[i] >= limit2) ? arr2[i] : 1;
    }
    
    /* Loop 7: LT_EXPR with floating-point in while-style loop */
    float fsum = 0.0f;
    float upper = 75.5f;
    for (int i = 0; i < N; i++) {
        /* Assignment controlled by < comparison */
        float val = farr[i];
        while (val < upper) {  /* This creates a LT_EXPR in loop condition */
            val += 0.5f;
        }
        fsum += val;
    }
    
    /* Loop 8: LE_EXPR with mixed types in complex expression */
    for (int i = 0; i < N; i++) {
        /* Nested ternary with <= as innermost condition */
        int x = arr1[i];
        int y = arr2[i];
        out1[i] = (x <= y) ? 
                 ((x <= 0) ? x : y) : 
                 ((y <= x - 100) ? x + y : x - y);
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + (int)fout[i] + (int)dout[i] + cout[i];
    }
    checksum += sum_ge + (long long)fsum;
    
    printf("Checksum: %lld\n", checksum);
    
    /* Additional test with vectorizable reduction */
    int red_sum = 0;
    int threshold3 = 250;
    for (int i = 0; i < N; i++) {
        /* Reduction with > comparison */
        red_sum += (arr1[i] > threshold3) ? arr1[i] : arr2[i];
    }
    printf("Reduction sum: %d\n", red_sum);
    
    /* Test with all four comparisons in one loop */
    int count_gt = 0, count_ge = 0, count_lt = 0, count_le = 0;
    int mid = 0;
    for (int i = 0; i < N; i++) {
        count_gt += (arr1[i] > mid) ? 1 : 0;
        count_ge += (arr1[i] >= mid) ? 1 : 0;
        count_lt += (arr1[i] < mid) ? 1 : 0;
        count_le += (arr1[i] <= mid) ? 1 : 0;
    }
    printf("Counts: GT=%d, GE=%d, LT=%d, LE=%d\n", 
           count_gt, count_ge, count_lt, count_le);
    
    return 0;
}
