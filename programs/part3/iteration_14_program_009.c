#include <stdio.h>
#include <stdlib.h>

#define N 1024

/* Initialize arrays with deterministic pseudo-random values */
static void init_arrays(int *a, unsigned int *b) {
    for (int i = 0; i < N; i++) {
        a[i] = (i * 37) % 1001;                    /* Range: 0-1000 */
        b[i] = (unsigned int)((i * 73) % 1003);    /* Range: 0-1002 */
    }
}

/* Test cases for each comparison operator */
static void test_gt(int *res, const int *a, const unsigned int *b) {
    for (int i = 0; i < N; i++) {
        /* Pattern: (a > b) ? a : b - should trigger GT_EXPR case */
        res[i] = (a[i] > (int)(b[i] & 0x7fffffff)) ? a[i] : (int)(b[i] & 0x7fffffff);
    }
}

static void test_ge(int *res, const int *a, const unsigned int *b) {
    for (int i = 0; i < N; i++) {
        /* Pattern: (a >= b) ? a : b - should trigger GE_EXPR case */
        res[i] = (a[i] >= (int)(b[i] & 0x7fffffff)) ? a[i] : (int)(b[i] & 0x7fffffff);
    }
}

static void test_lt(int *res, const int *a, const unsigned int *b) {
    for (int i = 0; i < N; i++) {
        /* Pattern: (a < b) ? a : b - should trigger LT_EXPR case with swap */
        res[i] = (a[i] < (int)(b[i] & 0x7fffffff)) ? a[i] : (int)(b[i] & 0x7fffffff);
    }
}

static void test_le(int *res, const int *a, const unsigned int *b) {
    for (int i = 0; i < N; i++) {
        /* Pattern: (a <= b) ? a : b - should trigger LE_EXPR case with swap */
        res[i] = (a[i] <= (int)(b[i] & 0x7fffffff)) ? a[i] : (int)(b[i] & 0x7fffffff);
    }
}

int main(void) {
    /* Source arrays with mixed signedness to inhibit MIN/MAX conversion */
    int a[N];
    unsigned int b[N];
    
    /* Result arrays for each test case */
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
    for (int i = 0; i < N; i++) {
        checksum += res_gt[i] + res_ge[i] + res_lt[i] + res_le[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    return 0;
}
