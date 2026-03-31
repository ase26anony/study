/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Prevent optimization of results */
static volatile int result_ge = 0;
static volatile int result_gt = 0;
static volatile int result_le = 0;
static volatile int result_lt = 0;

/* Function targeting GE_EXPR case */
void test_ge_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int i;
    /* Pattern: create mask from comparison result */
    for (i = 0; i < N; i++) {
        /* This should generate a mask: -1 if true, 0 if false */
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    /* Also do a conditional sum to ensure the comparison is used */
    int sum = 0;
    for (i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    result_ge = sum;
}

/* Function targeting GT_EXPR case */
void test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int i;
    for (i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? -1 : 0;
    }
    
    int sum = 0;
    for (i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    result_gt = sum;
}

/* Function targeting LE_EXPR case (note: std::swap in the expansion) */
void test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int i;
    for (i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? -1 : 0;
    }
    
    int sum = 0;
    for (i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            sum += a[i];
        }
    }
    result_le = sum;
}

/* Function targeting LT_EXPR case (note: std::swap in the expansion) */
void test_lt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int i;
    for (i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? -1 : 0;
    }
    
    int sum = 0;
    for (i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    result_lt = sum;
}

/* Additional test with short type to potentially trigger different paths */
void test_ge_vectorize_short(ALIGNED short *a, ALIGNED short *b, ALIGNED short *c) {
    int i;
    for (i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
}

int main() {
    int i;
    
    /* Allocate aligned arrays */
    ALIGNED int *a = malloc(N * sizeof(int));
    ALIGNED int *b = malloc(N * sizeof(int));
    ALIGNED int *c = malloc(N * sizeof(int));
    
    ALIGNED short *as = malloc(N * sizeof(short));
    ALIGNED short *bs = malloc(N * sizeof(short));
    ALIGNED short *cs = malloc(N * sizeof(short));
    
    if (!a || !b || !c || !as || !bs || !cs) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern that will trigger comparisons */
    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N/2;  /* Half will be >=, half < */
        
        as[i] = (short)i;
        bs[i] = (short)(N/2);
    }
    
    /* Test all comparison types */
    test_ge_vectorize(a, b, c);
    test_gt_vectorize(a, b, c);
    test_le_vectorize(a, b, c);
    test_lt_vectorize(a, b, c);
    test_ge_vectorize_short(as, bs, cs);
    
    /* Verify some results to prevent dead code elimination */
    int checksum = 0;
    for (i = 0; i < 16; i++) {
        checksum += c[i];
    }
    
    printf("Results: GE=%d, GT=%d, LE=%d, LT=%d, checksum=%d\n",
           result_ge, result_gt, result_le, result_lt, checksum);
    
    /* Clean up */
    free(a);
    free(b);
    free(c);
    free(as);
    free(bs);
    free(cs);
    
    return 0;
}
