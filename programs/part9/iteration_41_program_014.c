/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Prevent optimization of computations */
static volatile int sink;

/* Test GE_EXPR (>=) operator - target case */
void test_ge_vectorize(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        /* Create mask from comparison: -1 for true, 0 for false */
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
}

/* Test GT_EXPR (>) operator */
void test_gt_vectorize(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? -1 : 0;
    }
}

/* Test LT_EXPR (<) operator */
void test_lt_vectorize(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? -1 : 0;
    }
}

/* Test LE_EXPR (<=) operator */
void test_le_vectorize(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? -1 : 0;
    }
}

/* Additional test with conditional reduction using GE_EXPR */
int test_ge_reduction(int *restrict a, int *restrict b) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

/* Test with short type to explore different vectorization paths */
void test_ge_short(short *restrict a, short *restrict b, short *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
}

int main() {
    /* Aligned arrays to help vectorization */
    ALIGNED int a[N], b[N], c[N];
    ALIGNED short as[N], bs[N], cs[N];
    
    /* Initialize with pattern that creates mixed true/false comparisons */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N/2;  /* Half will be >= N/2 */
        as[i] = (short)i;
        bs[i] = (short)(N/2);
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
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += c[i] + cs[i];
    }
    checksum += sum;
    
    sink = checksum;  /* Use volatile to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
