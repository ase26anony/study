/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc
 * Specifically targets the GE_EXPR case (lines 12216-12233)
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Function to test GE_EXPR vectorization */
int test_ge_vectorize(ALIGNED int *restrict a, ALIGNED int *restrict b, ALIGNED int *restrict c) {
    int sum = 0;
    
    /* Pattern 1: Conditional sum using >= comparison */
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    /* Pattern 2: Mask creation using >= comparison */
    for (int i = 0; i < N; i++) {
        // Create mask: -1 if true, 0 if false
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    /* Pattern 3: Conditional assignment using mask */
    for (int i = 0; i < N; i++) {
        // Use the mask to select between values
        c[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
    
    return sum;
}

/* Function to test GT_EXPR vectorization */
int test_gt_vectorize(ALIGNED int *restrict a, ALIGNED int *restrict b, ALIGNED int *restrict c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? -1 : 0;
    }
    
    return sum;
}

/* Function to test LT_EXPR vectorization */
int test_lt_vectorize(ALIGNED int *restrict a, ALIGNED int *restrict b, ALIGNED int *restrict c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? -1 : 0;
    }
    
    return sum;
}

/* Function to test LE_EXPR vectorization */
int test_le_vectorize(ALIGNED int *restrict a, ALIGNED int *restrict b, ALIGNED int *restrict c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            sum += a[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? -1 : 0;
    }
    
    return sum;
}

/* Test with short type to potentially trigger different code paths */
short test_short_ge_vectorize(ALIGNED short *restrict a, ALIGNED short *restrict b, ALIGNED short *restrict c) {
    short sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    return sum;
}

int main() {
    /* Aligned arrays to help vectorization */
    ALIGNED int a[N], b[N], c[N];
    ALIGNED short as[N], bs[N], cs[N];
    
    /* Initialize with pattern that ensures some true and some false comparisons */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N/2;  // Half will be >=, half <
        c[i] = 0;
        
        as[i] = (short)i;
        bs[i] = (short)(N/2);
        cs[i] = 0;
    }
    
    /* Volatile variables to prevent optimization */
    volatile int result_ge, result_gt, result_lt, result_le;
    volatile short result_short_ge;
    
    /* Test all comparison operators */
    result_ge = test_ge_vectorize(a, b, c);
    result_gt = test_gt_vectorize(a, b, c);
    result_lt = test_lt_vectorize(a, b, c);
    result_le = test_le_vectorize(a, b, c);
    result_short_ge = test_short_ge_vectorize(as, bs, cs);
    
    /* Print results to prevent dead code elimination */
    printf("GE result: %d\n", result_ge);
    printf("GT result: %d\n", result_gt);
    printf("LT result: %d\n", result_lt);
    printf("LE result: %d\n", result_le);
    printf("Short GE result: %d\n", (int)result_short_ge);
    
    /* Simple checksum using the results */
    int checksum = result_ge + result_gt + result_lt + result_le + result_short_ge;
    printf("Checksum: %d\n", checksum);
    
    /* Verify some values in c array */
    printf("Sample c[0], c[N/4], c[N/2], c[3*N/4]: %d %d %d %d\n", 
           c[0], c[N/4], c[N/2], c[3*N/4]);
    
    return 0;
}
