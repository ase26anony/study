/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Prevent optimization of results */
static volatile int sink;

/* Test GE_EXPR (>=) case - target for uncovered lines */
void test_ge_vectorize(ALIGNED int *restrict a, ALIGNED int *restrict b, 
                       ALIGNED int *restrict c) {
    for (int i = 0; i < N; i++) {
        /* Create mask from comparison: -1 for true, 0 for false */
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
}

/* Test GT_EXPR (>) case */
void test_gt_vectorize(ALIGNED int *restrict a, ALIGNED int *restrict b,
                       ALIGNED int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? -1 : 0;
    }
}

/* Test LT_EXPR (<) case */
void test_lt_vectorize(ALIGNED int *restrict a, ALIGNED int *restrict b,
                       ALIGNED int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? -1 : 0;
    }
}

/* Test LE_EXPR (<=) case */
void test_le_vectorize(ALIGNED int *restrict a, ALIGNED int *restrict b,
                       ALIGNED int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? -1 : 0;
    }
}

/* Alternative test using conditional reduction - another path to same expansion */
int test_ge_reduction(ALIGNED int *restrict a, ALIGNED int *restrict b) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

/* Test with short type to potentially trigger different vectorization */
void test_ge_short(ALIGNED short *restrict a, ALIGNED short *restrict b,
                   ALIGNED short *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
}

int main() {
    /* Aligned arrays for vectorization */
    ALIGNED int a[N], b[N], c[N];
    ALIGNED short as[N], bs[N], cs[N];
    
    /* Initialize with pattern that ensures mixed true/false comparisons */
    for (int i = 0; i < N; i++) {
        a[i] = i - N/2;          /* Range: -512 to 511 */
        b[i] = i % 100;          /* Range: 0 to 99 */
        as[i] = (short)(i - N/2);
        bs[i] = (short)(i % 100);
    }
    
    /* Test all comparison operators */
    test_ge_vectorize(a, b, c);
    test_gt_vectorize(a, b, c);
    test_lt_vectorize(a, b, c);
    test_le_vectorize(a, b, c);
    
    /* Test reduction pattern */
    int sum = test_ge_reduction(a, b);
    
    /* Test with short type */
    test_ge_short(as, bs, cs);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += c[i] + cs[i];
    }
    checksum += sum;
    
    sink = checksum;  /* Volatile write to ensure computation isn't optimized away */
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed - if vectorized, should have hit target lines in tree-vect-stmts.cc\n");
    
    return 0;
}
