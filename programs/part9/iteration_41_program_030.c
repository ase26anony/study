/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc
 * Specifically targets the GE_EXPR, GT_EXPR, LT_EXPR, and LE_EXPR cases
 * in the expand_vec_cmp_expr function (lines around 12216-12233)
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Force results to be used to prevent optimization */
static volatile int result_ge = 0;
static volatile int result_gt = 0;
static volatile int result_lt = 0;
static volatile int result_le = 0;

/* Test GE_EXPR (>=) - creates mask pattern that should use BIT_NOT_EXPR + BIT_IOR_EXPR */
void test_ge_vectorize(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        /* Create mask: -1 if a[i] >= b[i], 0 otherwise
         * This pattern often triggers the specific expansion path
         */
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
}

/* Test GT_EXPR (>) - similar pattern for greater-than */
void test_gt_vectorize(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? -1 : 0;
    }
}

/* Test LT_EXPR (<) - should trigger swap of operands */
void test_lt_vectorize(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? -1 : 0;
    }
}

/* Test LE_EXPR (<=) - should trigger swap of operands */
void test_le_vectorize(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? -1 : 0;
    }
}

/* Alternative test using conditional reduction - another pattern that might
 * trigger the expansion */
int test_ge_reduction(int *restrict a, int *restrict b) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

/* Test with short type to potentially trigger different vectorization */
void test_ge_short(short *restrict a, short *restrict b, short *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? (short)-1 : (short)0;
    }
}

int main() {
    /* Aligned arrays to help vectorization */
    ALIGNED int a[N], b[N], c[N];
    ALIGNED short as[N], bs[N], cs[N];
    
    /* Initialize with pattern that creates mix of true/false comparisons */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N/2;  /* Half will be >=, half < */
        as[i] = (short)i;
        bs[i] = (short)(N/2);
    }
    
    /* Test all comparison operators */
    test_ge_vectorize(a, b, c);
    test_gt_vectorize(a, b, c);
    test_lt_vectorize(a, b, c);
    test_le_vectorize(a, b, c);
    
    /* Test with short type */
    test_ge_short(as, bs, cs);
    
    /* Test reduction pattern */
    int sum = test_ge_reduction(a, b);
    
    /* Use results to prevent optimization */
    result_ge = c[0];
    result_gt = c[1];
    result_lt = c[2];
    result_le = c[3];
    
    /* Simple checksum to verify execution */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += c[i] + cs[i];
    }
    checksum += sum;
    
    printf("Checksum: %d\n", checksum);
    printf("Results: ge=%d, gt=%d, lt=%d, le=%d\n", 
           result_ge, result_gt, result_lt, result_le);
    
    return 0;
}
