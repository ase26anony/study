#include <stdio.h>
#include <stdlib.h>

#define N 1024

/* Mixed signed/unsigned types to prevent canonicalization */
static void test_gt(int *restrict res, int *restrict a, unsigned int *restrict b)
{
    for (int i = 0; i < N; ++i) {
        /* Pattern: (a > b) ? a : b */
        res[i] = (a[i] > (int)b[i]) ? a[i] : (int)b[i];
    }
}

static void test_ge(int *restrict res, int *restrict a, unsigned int *restrict b)
{
    for (int i = 0; i < N; ++i) {
        /* Pattern: (a >= b) ? a : b */
        res[i] = (a[i] >= (int)b[i]) ? a[i] : (int)b[i];
    }
}

static void test_lt(int *restrict res, int *restrict a, unsigned int *restrict b)
{
    for (int i = 0; i < N; ++i) {
        /* Pattern: (a < b) ? a : b - should trigger operand swap */
        res[i] = (a[i] < (int)b[i]) ? a[i] : (int)b[i];
    }
}

static void test_le(int *restrict res, int *restrict a, unsigned int *restrict b)
{
    for (int i = 0; i < N; ++i) {
        /* Pattern: (a <= b) ? a : b - should trigger operand swap */
        res[i] = (a[i] <= (int)b[i]) ? a[i] : (int)b[i];
    }
}

int main(void)
{
    /* Declare and initialize source arrays with deterministic values */
    int a[N];
    unsigned int b[N];
    
    /* Result arrays for each test */
    int res_gt[N], res_ge[N], res_lt[N], res_le[N];
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < N; ++i) {
        a[i] = (i * 37) % 1001;              /* Range: 0-1000 */
        b[i] = (i * 73) % 1001;              /* Same range, different distribution */
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
    
    /* Expected checksum for verification */
    /* With N=1024, a[i]=(i*37)%1001, b[i]=(i*73)%1001 */
    /* Expected checksum: 2045952 */
    if (checksum == 2045952LL) {
        printf("Result verification passed\n");
    } else {
        printf("Result verification failed\n");
    }
    
    return 0;
}
