/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc
 * Specifically targets the GE_EXPR, GT_EXPR, LT_EXPR, and LE_EXPR cases
 * in the switch statement at lines 12216-12233
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Force compiler to keep computations */
static volatile int sink;

/* Test GE_EXPR (greater than or equal) */
void test_ge_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    for (int i = 0; i < N; i++) {
        /* Create mask from comparison: -1 for true, 0 for false
         * This pattern often triggers the specific expansion path
         */
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
}

/* Test GT_EXPR (greater than) */
void test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? -1 : 0;
    }
}

/* Test LT_EXPR (less than) */
void test_lt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? -1 : 0;
    }
}

/* Test LE_EXPR (less than or equal) */
void test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? -1 : 0;
    }
}

/* Alternative pattern: conditional reduction */
int test_ge_reduction(ALIGNED int *a, ALIGNED int *b) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

/* Another pattern: conditional select */
void test_ge_select(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
}

int main() {
    /* Aligned arrays to help vectorization */
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED int c[N];
    
    /* Initialize with pattern that will trigger comparisons */
    for (int i = 0; i < N; i++) {
        a[i] = i;               /* 0, 1, 2, ..., N-1 */
        b[i] = N/2;             /* All N/2 */
    }
    
    /* Test all comparison operators */
    test_ge_vectorize(a, b, c);
    test_gt_vectorize(a, b, c);
    test_lt_vectorize(a, b, c);
    test_le_vectorize(a, b, c);
    
    /* Test reduction pattern */
    int sum = test_ge_reduction(a, b);
    
    /* Test select pattern */
    test_ge_select(a, b, c);
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += c[i];
    }
    checksum += sum;
    
    sink = checksum;  /* Force compiler to compute everything */
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed - if vectorized, should have hit target lines\n");
    
    return 0;
}
