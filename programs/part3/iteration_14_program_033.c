#include <stdio.h>
#include <stdlib.h>

#define N 1024

/* Test functions for each comparison operator */
static void test_gt(int *restrict res, const int *restrict a, const unsigned int *restrict b)
{
    for (int i = 0; i < N; ++i) {
        /* GT_EXPR pattern: (a > b) ? a : b */
        /* Mixed signed/unsigned to prevent MIN/MAX transformation */
        res[i] = (a[i] > (int)(b[i] & 0x7fffffff)) ? a[i] : (int)(b[i] & 0x7fffffff);
    }
}

static void test_ge(int *restrict res, const int *restrict a, const unsigned int *restrict b)
{
    for (int i = 0; i < N; ++i) {
        /* GE_EXPR pattern: (a >= b) ? a : b */
        res[i] = (a[i] >= (int)(b[i] & 0x7fffffff)) ? a[i] : (int)(b[i] & 0x7fffffff);
    }
}

static void test_lt(int *restrict res, const int *restrict a, const unsigned int *restrict b)
{
    for (int i = 0; i < N; ++i) {
        /* LT_EXPR pattern: (a < b) ? a : b */
        /* First operand is compared, will be swapped internally */
        res[i] = (a[i] < (int)(b[i] & 0x7fffffff)) ? a[i] : (int)(b[i] & 0x7fffffff);
    }
}

static void test_le(int *restrict res, const int *restrict a, const unsigned int *restrict b)
{
    for (int i = 0; i < N; ++i) {
        /* LE_EXPR pattern: (a <= b) ? a : b */
        res[i] = (a[i] <= (int)(b[i] & 0x7fffffff)) ? a[i] : (int)(b[i] & 0x7fffffff);
    }
}

int main(void)
{
    /* Declare and initialize source arrays with deterministic values */
    int a[N];
    unsigned int b[N];
    
    /* Declare result arrays for each test */
    int res_gt[N], res_ge[N], res_lt[N], res_le[N];
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < N; ++i) {
        a[i] = (i * 37) % 1001;                    /* Values in range 0-1000 */
        b[i] = (i * 73) % 1001;                    /* Different sequence */
    }
    
    /* Execute all test functions */
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
    
    /* Expected checksum for verification: 2045952 */
    /* Uncomment for debugging:
    printf("Expected: 2045952\n");
    */
    
    return 0;
}
