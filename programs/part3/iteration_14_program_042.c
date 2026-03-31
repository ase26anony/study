#include <stdio.h>
#include <stdlib.h>

#define N 1024

/* Initialize arrays with deterministic pseudo-random values */
static void init_arrays(int *a, unsigned int *b)
{
    for (int i = 0; i < N; i++) {
        a[i] = (i * 37) % 1001;                /* Range: 0-1000 */
        b[i] = (unsigned int)((i * 73) % 1003); /* Range: 0-1002 */
    }
}

/* Test cases for each comparison operator */

/* GT_EXPR: (a > b) ? a : b */
static void test_gt(int *res, int *a, unsigned int *b)
{
    for (int i = 0; i < N; i++) {
        /* Mixed signed/unsigned prevents MIN/MAX transformation */
        res[i] = (a[i] > (int)b[i]) ? a[i] : (int)b[i];
    }
}

/* GE_EXPR: (a >= b) ? a : b */
static void test_ge(int *res, int *a, unsigned int *b)
{
    for (int i = 0; i < N; i++) {
        /* Use bitmask to prevent operand canonicalization */
        unsigned int masked_b = b[i] & 0x7fffffff;
        res[i] = (a[i] >= (int)masked_b) ? a[i] : (int)masked_b;
    }
}

/* LT_EXPR: (a < b) ? a : b */
static void test_lt(int *res, int *a, unsigned int *b)
{
    for (int i = 0; i < N; i++) {
        /* Mixed types with simple operation */
        res[i] = (a[i] < (int)b[i]) ? a[i] : (int)b[i];
    }
}

/* LE_EXPR: (a <= b) ? a : b */
static void test_le(int *res, int *a, unsigned int *b)
{
    for (int i = 0; i < N; i++) {
        /* Use different bitmask pattern */
        unsigned int masked_b = (b[i] ^ 0x1234) & 0x7fffffff;
        res[i] = (a[i] <= (int)masked_b) ? a[i] : (int)masked_b;
    }
}

int main(void)
{
    /* Source arrays with different types to prevent aliasing */
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
    for (int i = 0; i < N; i++) {
        checksum += res_gt[i] + res_ge[i] + res_lt[i] + res_le[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Expected checksum for verification */
    /* With N=1024 and the given initialization patterns: */
    /* Expected checksum: 2050048 */
    
    return 0;
}
