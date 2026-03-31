/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Prevent optimization of computations */
static volatile int sink;

/* GE_EXPR test - target case */
void test_ge_vectorize(ALIGNED int *restrict a, ALIGNED int *restrict b, 
                       ALIGNED int *restrict c) {
    int i;
    /* Pattern 1: Create mask from comparison */
    for (i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? -1 : 0;  // Generates mask
    }
    
    /* Pattern 2: Conditional reduction using mask */
    int sum = 0;
    for (i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    sink = sum;  /* Prevent dead code elimination */
}

/* GT_EXPR test */
void test_gt_vectorize(ALIGNED short *restrict a, ALIGNED short *restrict b,
                       ALIGNED short *restrict c) {
    int i;
    for (i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? (short)-1 : 0;
    }
    
    int sum = 0;
    for (i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    sink = sum;
}

/* LT_EXPR test - will trigger std::swap */
void test_lt_vectorize(ALIGNED int *restrict a, ALIGNED int *restrict b,
                       ALIGNED int *restrict c) {
    int i;
    for (i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? -1 : 0;
    }
    
    int sum = 0;
    for (i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            sum += b[i];
        }
    }
    sink = sum;
}

/* LE_EXPR test - will trigger std::swap */
void test_le_vectorize(ALIGNED int *restrict a, ALIGNED int *restrict b,
                       ALIGNED int *restrict c) {
    int i;
    for (i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? -1 : 0;
    }
    
    int sum = 0;
    for (i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            sum += a[i] + b[i];
        }
    }
    sink = sum;
}

/* Mixed test with different data types */
void test_mixed_comparisons(ALIGNED int *restrict a, ALIGNED int *restrict b,
                            ALIGNED int *restrict c, ALIGNED int *restrict d) {
    int i;
    
    /* Multiple comparisons in one loop */
    for (i = 0; i < N; i++) {
        int ge_mask = (a[i] >= b[i]) ? -1 : 0;
        int lt_mask = (a[i] < b[i]) ? -1 : 0;
        c[i] = ge_mask & (a[i] + 5);
        d[i] = lt_mask | (b[i] - 3);
    }
    
    /* Complex reduction with GE */
    int count = 0;
    for (i = 0; i < N; i++) {
        count += (a[i] >= b[i]) ? 1 : 0;
    }
    sink = count;
}

int main() {
    int i;
    
    /* Aligned arrays for vectorization */
    ALIGNED int a[N], b[N], c[N], d[N];
    ALIGNED short sa[N], sb[N], sc[N];
    
    /* Initialize with pattern that creates mixed comparison results */
    for (i = 0; i < N; i++) {
        a[i] = i - N/2;           /* Range: [-512, 511] */
        b[i] = i % 100 - 50;      /* Range: [-50, 49] */
        sa[i] = (short)(i * 3);
        sb[i] = (short)(i * 2 + 100);
    }
    
    /* Test all comparison types */
    test_ge_vectorize(a, b, c);
    test_gt_vectorize(sa, sb, sc);
    test_lt_vectorize(a, b, c);
    test_le_vectorize(a, b, c);
    test_mixed_comparisons(a, b, c, d);
    
    /* Verify some results to ensure code executed */
    int checksum = 0;
    for (i = 0; i < 10; i++) {
        checksum += c[i] + d[i] + sc[i];
    }
    
    printf("Test completed. Checksum: %d\n", checksum);
    printf("Sink value: %d\n", sink);
    
    return 0;
}
