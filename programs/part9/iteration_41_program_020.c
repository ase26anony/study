/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Prevent optimizations from removing computations */
static volatile int sink;

/* Test GE_EXPR case: create mask from a[i] >= b[i] */
void test_ge_vectorize(int *restrict a, int *restrict b, int *restrict out) {
    for (int i = 0; i < N; i++) {
        /* This creates a mask: -1 if true, 0 if false */
        out[i] = (a[i] >= b[i]) ? -1 : 0;
    }
}

/* Test GT_EXPR case: create mask from a[i] > b[i] */
void test_gt_vectorize(int *restrict a, int *restrict b, int *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] > b[i]) ? -1 : 0;
    }
}

/* Test LT_EXPR case: create mask from a[i] < b[i] */
void test_lt_vectorize(int *restrict a, int *restrict b, int *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] < b[i]) ? -1 : 0;
    }
}

/* Test LE_EXPR case: create mask from a[i] <= b[i] */
void test_le_vectorize(int *restrict a, int *restrict b, int *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] <= b[i]) ? -1 : 0;
    }
}

/* Alternative test using conditional reduction (sum where condition is true) */
int test_ge_reduction(int *restrict a, int *restrict b) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

/* Test with short type to potentially trigger different code paths */
void test_ge_short(short *restrict a, short *restrict b, short *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] >= b[i]) ? -1 : 0;
    }
}

int main() {
    /* Use aligned arrays to help vectorization */
    ALIGNED int a[N], b[N], out[N];
    ALIGNED short as[N], bs[N], outs[N];
    
    /* Initialize with pattern that ensures some true and some false comparisons */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N/2;  /* Half will be >= N/2 */
        as[i] = (short)i;
        bs[i] = (short)(N/2);
    }
    
    /* Test all comparison operators */
    test_ge_vectorize(a, b, out);
    test_gt_vectorize(a, b, out);
    test_lt_vectorize(a, b, out);
    test_le_vectorize(a, b, out);
    
    /* Test reduction pattern */
    int sum = test_ge_reduction(a, b);
    
    /* Test with short type */
    test_ge_short(as, bs, outs);
    
    /* Use results to prevent optimization */
    sink = out[0] + out[N-1] + sum + outs[0];
    
    /* Simple checksum to verify execution */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Reduction sum: %d\n", sum);
    printf("Test completed successfully.\n");
    
    return 0;
}
