#include <stdio.h>
#include <stdlib.h>

#define N 1024

/* Initialize arrays with deterministic pseudo-random values */
static void init_arrays(int *a, unsigned int *b) {
    for (int i = 0; i < N; ++i) {
        a[i] = (i * 37) % 1001;                /* Range: 0-1000 */
        b[i] = (unsigned int)((i * 73) % 1003); /* Range: 0-1002 */
    }
}

/* Test cases for each comparison operator */
static void test_gt(int *a, unsigned int *b, int *res) {
    for (int i = 0; i < N; ++i) {
        /* Pattern: (a > b) ? a : b */
        res[i] = (a[i] > (int)b[i]) ? a[i] : (int)b[i];
    }
}

static void test_ge(int *a, unsigned int *b, int *res) {
    for (int i = 0; i < N; ++i) {
        /* Pattern: (a >= b) ? a : b */
        res[i] = (a[i] >= (int)(b[i] & 0x7fffffff)) ? a[i] : (int)(b[i] & 0x7fffffff);
    }
}

static void test_lt(int *a, unsigned int *b, int *res) {
    for (int i = 0; i < N; ++i) {
        /* Pattern: (a < b) ? a : b */
        res[i] = (a[i] < (int)b[i]) ? a[i] : (int)b[i];
    }
}

static void test_le(int *a, unsigned int *b, int *res) {
    for (int i = 0; i < N; ++i) {
        /* Pattern: (a <= b) ? a : b */
        res[i] = (a[i] <= (int)(b[i] & 0x7fffffff)) ? a[i] : (int)(b[i] & 0x7fffffff);
    }
}

int main(void) {
    /* Source arrays with different types to inhibit MIN/MAX folding */
    int a[N];
    unsigned int b[N];
    
    /* Result arrays for each test */
    int res_gt[N], res_ge[N], res_lt[N], res_le[N];
    
    /* Initialize with deterministic values */
    init_arrays(a, b);
    
    /* Execute all test patterns */
    test_gt(a, b, res_gt);
    test_ge(a, b, res_ge);
    test_lt(a, b, res_lt);
    test_le(a, b, res_le);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += res_gt[i] + res_ge[i] + res_lt[i] + res_le[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    return 0;
}
