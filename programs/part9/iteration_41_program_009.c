/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc
 * Specifically targets lines 12216-12233 for GE_EXPR, GT_EXPR, LT_EXPR, LE_EXPR
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Force side effects to prevent optimization */
volatile int dummy_result = 0;

/* Test GE_EXPR (>=) case - primary target */
int test_ge_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* Pattern 1: Conditional sum using >= */
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    /* Pattern 2: Create mask from >= comparison */
    for (int i = 0; i < N; i++) {
        /* This creates a mask: -1 for true, 0 for false */
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    /* Pattern 3: Reduction counting >= comparisons */
    int count = 0;
    for (int i = 0; i < N; i++) {
        count += (a[i] >= b[i]);
    }
    
    return sum + count + c[N-1];
}

/* Test GT_EXPR (>) case */
int test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? -1 : 0;
    }
    
    return sum + c[N-1];
}

/* Test LT_EXPR (<) case */
int test_lt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            sum += b[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? -1 : 0;
    }
    
    return sum + c[N-1];
}

/* Test LE_EXPR (<=) case */
int test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            sum += b[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? -1 : 0;
    }
    
    return sum + c[N-1];
}

/* Also test with short type to potentially trigger different paths */
short test_short_ge_vectorize(ALIGNED short *a, ALIGNED short *b, ALIGNED short *c) {
    short sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    return sum + c[N-1];
}

int main() {
    /* Aligned arrays to help vectorization */
    ALIGNED int a[N], b[N], c[N];
    ALIGNED short as[N], bs[N], cs[N];
    
    /* Initialize with pattern that ensures all comparison results occur */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N/2;  /* Half will be >=, half < */
        c[i] = 0;
        
        as[i] = (short)i;
        bs[i] = (short)(N/2);
        cs[i] = 0;
    }
    
    /* Test all comparison types */
    int result = 0;
    
    result += test_ge_vectorize(a, b, c);
    dummy_result = result;  /* Force side effect */
    
    result += test_gt_vectorize(a, b, c);
    dummy_result = result;
    
    result += test_lt_vectorize(a, b, c);
    dummy_result = result;
    
    result += test_le_vectorize(a, b, c);
    dummy_result = result;
    
    /* Test with short type */
    short short_result = test_short_ge_vectorize(as, bs, cs);
    dummy_result = short_result;
    
    /* Print results to prevent dead code elimination */
    printf("Result: %d (short: %d)\n", result, short_result);
    
    /* Verify some computations */
    int verify = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) verify += a[i];
    }
    printf("GE verification sum: %d\n", verify);
    
    return 0;
}
