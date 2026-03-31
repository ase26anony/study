#include <stdio.h>
#include <stdlib.h>

#define N 1024

/* Initialize arrays with deterministic pseudo-random values */
static void init_arrays(int *a, unsigned int *b) {
    for (int i = 0; i < N; ++i) {
        a[i] = (i * 37) % 1001;                /* int values */
        b[i] = (unsigned int)((i * 53) % 1003); /* unsigned int values */
    }
}

/* Test cases for each comparison operator */

/* GT_EXPR: a[i] > b[i] ? a[i] : b[i] */
static void test_gt(int *res, int *a, unsigned int *b) {
    for (int i = 0; i < N; ++i) {
        /* Mixed signed/unsigned comparison prevents MIN/MAX folding */
        res[i] = (a[i] > (int)b[i]) ? a[i] : (int)b[i];
    }
}

/* GE_EXPR: a[i] >= b[i] ? a[i] : b[i] */
static void test_ge(int *res, int *a, unsigned int *b) {
    for (int i = 0; i < N; ++i) {
        /* Add mild complexity to prevent canonicalization */
        res[i] = (a[i] >= (int)(b[i] & 0x7fffffff)) ? a[i] : (int)(b[i] & 0x7fffffff);
    }
}

/* LT_EXPR: a[i] < b[i] ? a[i] : b[i] */
static void test_lt(int *res, int *a, unsigned int *b) {
    for (int i = 0; i < N; ++i) {
        /* First operand is a[i], will be swapped internally */
        res[i] = (a[i] < (int)b[i]) ? a[i] : (int)b[i];
    }
}

/* LE_EXPR: a[i] <= b[i] ? a[i] : b[i] */
static void test_le(int *res, int *a, unsigned int *b) {
    for (int i = 0; i < N; ++i) {
        /* First operand is a[i], will be swapped internally */
        res[i] = (a[i] <= (int)(b[i] & 0x7fffffff)) ? a[i] : (int)(b[i] & 0x7fffffff);
    }
}

int main(void) {
    /* Source arrays with mixed signedness */
    int a[N];
    unsigned int b[N];
    
    /* Result arrays for each test */
    int res_gt[N], res_ge[N], res_lt[N], res_le[N];
    
    /* Initialize with deterministic values */
    init_arrays(a, b);
    
    /* Execute all test cases */
    test_gt(res_gt, a, b);
    test_ge(res_ge, a, b);
    test_lt(res_lt, a, b);
    test_le(res_le, a, b);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += res_gt[i] + res_ge[i] + res_lt[i] + res_le[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
