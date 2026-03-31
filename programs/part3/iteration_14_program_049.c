#include <stdio.h>
#include <stdlib.h>

#define N 1024

/* Test functions for each comparison operator */
static void test_gt(int *restrict a, unsigned int *restrict b, int *restrict res)
{
    for (int i = 0; i < N; ++i)
        res[i] = (a[i] > b[i]) ? a[i] : b[i];
}

static void test_ge(int *restrict a, unsigned int *restrict b, int *restrict res)
{
    for (int i = 0; i < N; ++i)
        res[i] = (a[i] >= (b[i] & 0x7fffffff)) ? a[i] : (b[i] & 0x7fffffff);
}

static void test_lt(int *restrict a, unsigned int *restrict b, int *restrict res)
{
    for (int i = 0; i < N; ++i)
        res[i] = (a[i] < b[i]) ? a[i] : b[i];
}

static void test_le(int *restrict a, unsigned int *restrict b, int *restrict res)
{
    for (int i = 0; i < N; ++i)
        res[i] = (a[i] <= (b[i] | 1)) ? a[i] : (b[i] | 1);
}

int main(void)
{
    /* Source arrays with mixed signed/unsigned types */
    int a[N];
    unsigned int b[N];
    
    /* Result arrays for each test */
    int res_gt[N], res_ge[N], res_lt[N], res_le[N];
    
    /* Initialize with deterministic pseudo-random values */
    for (int i = 0; i < N; ++i) {
        a[i] = (i * 37) % 1001;
        b[i] = (i * 73) % 1003;
    }
    
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
