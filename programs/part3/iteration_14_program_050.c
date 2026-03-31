#include <stdio.h>
#include <stdlib.h>

#define N 1024

/* Test functions for each comparison operator */
static void test_gt(int *restrict res, const int *restrict a, const unsigned int *restrict b)
{
    for (int i = 0; i < N; ++i) {
        /* Pattern: (a > b) ? a : b 
           Mixed signedness prevents MIN/MAX transformation */
        res[i] = (a[i] > (int)(b[i] & 0x7fffffff)) ? a[i] : (int)(b[i] & 0x7fffffff);
    }
}

static void test_ge(int *restrict res, const int *restrict a, const unsigned int *restrict b)
{
    for (int i = 0; i < N; ++i) {
        /* Pattern: (a >= b) ? a : b */
        res[i] = (a[i] >= (int)(b[i] & 0x7fffffff)) ? a[i] : (int)(b[i] & 0x7fffffff);
    }
}

static void test_lt(int *restrict res, const int *restrict a, const unsigned int *restrict b)
{
    for (int i = 0; i < N; ++i) {
        /* Pattern: (a < b) ? a : b 
           First operand is compared, will be swapped internally */
        res[i] = (a[i] < (int)(b[i] & 0x7fffffff)) ? a[i] : (int)(b[i] & 0x7fffffff);
    }
}

static void test_le(int *restrict res, const int *restrict a, const unsigned int *restrict b)
{
    for (int i = 0; i < N; ++i) {
        /* Pattern: (a <= b) ? a : b */
        res[i] = (a[i] <= (int)(b[i] & 0x7fffffff)) ? a[i] : (int)(b[i] & 0x7fffffff);
    }
}

int main(void)
{
    /* Declare and initialize source arrays */
    int a[N];
    unsigned int b[N];
    
    /* Declare result arrays */
    int res_gt[N], res_ge[N], res_lt[N], res_le[N];
    
    /* Initialize with deterministic pseudo-random values */
    for (int i = 0; i < N; ++i) {
        a[i] = (i * 37) % 1001;                /* Range: 0-1000 */
        b[i] = (i * 73) % 1001;                /* Range: 0-1000 */
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
    
    /* Verify with simple test case */
    if (checksum == 0) {
        fprintf(stderr, "Error: All results are zero\n");
        return 1;
    }
    
    return 0;
}
