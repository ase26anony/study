#include <stdio.h>
#include <stdlib.h>

#define N 1024

/* Initialize arrays with deterministic pseudo-random values */
static void init_arrays(int *a, unsigned int *b)
{
    for (int i = 0; i < N; i++) {
        a[i] = (i * 37) % 1001;                /* Range: 0-1000 */
        b[i] = (unsigned int)((i * 73) % 1001); /* Same range but unsigned */
    }
}

/* GT_EXPR pattern: (a > b) ? a : b */
static void test_gt(int *res, const int *a, const unsigned int *b)
{
    for (int i = 0; i < N; i++) {
        /* Mixed signed/unsigned prevents MIN/MAX transformation */
        res[i] = (a[i] > (int)b[i]) ? a[i] : (int)b[i];
    }
}

/* GE_EXPR pattern: (a >= b) ? a : b */
static void test_ge(int *res, const int *a, const unsigned int *b)
{
    for (int i = 0; i < N; i++) {
        /* Add mild complexity to prevent canonicalization */
        res[i] = (a[i] >= (int)(b[i] & 0x7fffffff)) ? a[i] : (int)(b[i] & 0x7fffffff);
    }
}

/* LT_EXPR pattern: (a < b) ? a : b */
static void test_lt(int *res, const int *a, const unsigned int *b)
{
    for (int i = 0; i < N; i++) {
        /* First operand is compared, will be swapped internally */
        res[i] = (a[i] < (int)b[i]) ? a[i] : (int)b[i];
    }
}

/* LE_EXPR pattern: (a <= b) ? a : b */
static void test_le(int *res, const int *a, const unsigned int *b)
{
    for (int i = 0; i < N; i++) {
        /* Mixed types prevent early optimization */
        res[i] = (a[i] <= (int)(b[i] | 1)) ? a[i] : (int)(b[i] | 1);
    }
}

int main(void)
{
    int a[N];
    unsigned int b[N];
    int res_gt[N], res_ge[N], res_lt[N], res_le[N];
    long long checksum = 0;
    
    /* Initialize source arrays */
    init_arrays(a, b);
    
    /* Execute all test patterns */
    test_gt(res_gt, a, b);
    test_ge(res_ge, a, b);
    test_lt(res_lt, a, b);
    test_le(res_le, a, b);
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        checksum += res_gt[i] + res_ge[i] + res_lt[i] + res_le[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    return 0;
}
