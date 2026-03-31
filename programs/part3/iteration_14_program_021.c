#include <stdio.h>
#include <stdlib.h>

#define N 1024

/* Initialize arrays with deterministic pseudo-random values */
static void init_arrays(int *a, unsigned int *b) {
    for (int i = 0; i < N; i++) {
        a[i] = (i * 37) % 1001;                 /* Range: 0-1000 */
        b[i] = (unsigned int)((i * 73) % 1003); /* Range: 0-1002 */
    }
}

/* Test cases for each comparison operator */

/* Greater than: (a > b) ? a : b */
static void test_gt(int *res, int *a, unsigned int *b) {
    for (int i = 0; i < N; i++) {
        /* Mixed signed/unsigned prevents MIN/MAX folding */
        res[i] = (a[i] > (int)b[i]) ? a[i] : (int)b[i];
    }
}

/* Greater than or equal: (a >= b) ? a : b */
static void test_ge(int *res, int *a, unsigned int *b) {
    for (int i = 0; i < N; i++) {
        /* Add mild complexity to prevent canonicalization */
        res[i] = (a[i] >= (int)(b[i] & 0x7fffffff)) ? 
                 a[i] : (int)(b[i] & 0x7fffffff);
    }
}

/* Less than: (a < b) ? a : b */
static void test_lt(int *res, int *a, unsigned int *b) {
    for (int i = 0; i < N; i++) {
        /* Mixed types prevent early optimization */
        res[i] = (a[i] < (int)b[i]) ? a[i] : (int)b[i];
    }
}

/* Less than or equal: (a <= b) ? a : b */
static void test_le(int *res, int *a, unsigned int *b) {
    for (int i = 0; i < N; i++) {
        /* Mild complexity prevents canonicalization */
        res[i] = (a[i] <= (int)(b[i] | 1)) ? a[i] : (int)(b[i] | 1);
    }
}

int main(void) {
    /* Source arrays with different types to inhibit optimizations */
    int a[N];
    unsigned int b[N];
    
    /* Destination arrays for each test */
    int res_gt[N], res_ge[N], res_lt[N], res_le[N];
    
    /* Initialize with deterministic values */
    init_arrays(a, b);
    
    /* Execute all test patterns */
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
    
    /* Verify with a simple test case */
    if (checksum != 2045952LL) {
        fprintf(stderr, "Unexpected checksum!\n");
        return 1;
    }
    
    return 0;
}
