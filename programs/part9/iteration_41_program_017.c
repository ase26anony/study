/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Prevent optimization of results */
static volatile int sink;

/* Initialize arrays with pattern that creates mixed comparison results */
static void init_arrays(int *a, int *b) {
    for (int i = 0; i < N; i++) {
        a[i] = i - N/2;          /* Range: [-512, 511] */
        b[i] = (i % 64) - 32;    /* Range: [-32, 31], repeats every 64 */
    }
}

/* GE_EXPR case: create mask from >= comparison */
static void test_ge_vectorize(int *a, int *b, int *mask) {
    for (int i = 0; i < N; i++) {
        /* This should generate vector comparison and mask creation */
        mask[i] = (a[i] >= b[i]) ? -1 : 0;
    }
}

/* GT_EXPR case: create mask from > comparison */
static void test_gt_vectorize(int *a, int *b, int *mask) {
    for (int i = 0; i < N; i++) {
        mask[i] = (a[i] > b[i]) ? -1 : 0;
    }
}

/* LT_EXPR case: create mask from < comparison */
static void test_lt_vectorize(int *a, int *b, int *mask) {
    for (int i = 0; i < N; i++) {
        mask[i] = (a[i] < b[i]) ? -1 : 0;
    }
}

/* LE_EXPR case: create mask from <= comparison */
static void test_le_vectorize(int *a, int *b, int *mask) {
    for (int i = 0; i < N; i++) {
        mask[i] = (a[i] <= b[i]) ? -1 : 0;
    }
}

/* Conditional reduction using >= comparison */
static int test_ge_reduction(int *a, int *b) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

/* Conditional select using >= comparison */
static void test_ge_select(int *a, int *b, int *c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
}

int main(void) {
    /* Aligned arrays for better vectorization */
    ALIGNED int a[N], b[N];
    ALIGNED int mask_ge[N], mask_gt[N], mask_lt[N], mask_le[N];
    ALIGNED int result[N];
    
    init_arrays(a, b);
    
    /* Test all four comparison operators */
    test_ge_vectorize(a, b, mask_ge);
    test_gt_vectorize(a, b, mask_gt);
    test_lt_vectorize(a, b, mask_lt);
    test_le_vectorize(a, b, mask_le);
    
    /* Test conditional reduction */
    int sum = test_ge_reduction(a, b);
    
    /* Test conditional select */
    test_ge_select(a, b, result);
    
    /* Compute checksums to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += mask_ge[i] ^ mask_gt[i] ^ mask_lt[i] ^ mask_le[i] ^ result[i];
    }
    checksum += sum;
    
    sink = checksum;  /* Volatile write to ensure computation isn't optimized away */
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed successfully\n");
    
    return 0;
}
