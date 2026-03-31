/* Test program to trigger vector comparison expansion for GE_EXPR and related cases */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Force the compiler to generate vectorized comparisons */
static void test_ge_vectorize(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        /* This creates a mask: -1 for true, 0 for false */
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
}

static void test_gt_vectorize(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? -1 : 0;
    }
}

static void test_lt_vectorize(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? -1 : 0;
    }
}

static void test_le_vectorize(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? -1 : 0;
    }
}

/* Conditional reduction that might also trigger the expansion */
static int test_ge_reduction(int *restrict a, int *restrict b) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

/* Another pattern: conditional select */
static void test_ge_select(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
}

int main(void) {
    /* Use aligned arrays to help vectorization */
    ALIGNED int a[N], b[N], c[N];
    int sum = 0;
    
    /* Initialize with pattern that ensures mixed comparison results */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N/2;  /* Half will be >=, half < */
    }
    
    /* Test all comparison operators to cover the switch cases */
    test_ge_vectorize(a, b, c);
    test_gt_vectorize(a, b, c);
    test_lt_vectorize(a, b, c);
    test_le_vectorize(a, b, c);
    
    /* Test reduction pattern */
    sum = test_ge_reduction(a, b);
    
    /* Test select pattern */
    test_ge_select(a, b, c);
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += c[i];
    }
    checksum += sum;
    
    printf("Checksum: %d\n", checksum);
    
    /* Verify some results */
    printf("Sample results (first 5):\n");
    for (int i = 0; i < 5; i++) {
        printf("a[%d]=%d, b[%d]=%d, c[%d]=%d\n", 
               i, a[i], i, b[i], i, c[i]);
    }
    
    return 0;
}
