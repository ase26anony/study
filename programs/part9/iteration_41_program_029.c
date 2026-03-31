/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Prevent optimization of computations */
static volatile int sink;

/* Test GE_EXPR (>=) comparison */
void test_ge_vectorize(ALIGNED int *restrict a, ALIGNED int *restrict b, 
                       ALIGNED int *restrict c, ALIGNED int *restrict mask) {
    int i;
    
    /* Pattern 1: Create mask from comparison (likely to use bitwise expansion) */
    for (i = 0; i < N; i++) {
        mask[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    /* Pattern 2: Conditional sum using comparison */
    int sum = 0;
    for (i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    sink = sum;
    
    /* Pattern 3: Blend operation using comparison */
    for (i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
}

/* Test GT_EXPR (>) comparison */
void test_gt_vectorize(ALIGNED int *restrict a, ALIGNED int *restrict b,
                       ALIGNED int *restrict mask) {
    int i;
    
    for (i = 0; i < N; i++) {
        mask[i] = (a[i] > b[i]) ? -1 : 0;
    }
    
    int count = 0;
    for (i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            count++;
        }
    }
    sink = count;
}

/* Test LT_EXPR (<) comparison */
void test_lt_vectorize(ALIGNED int *restrict a, ALIGNED int *restrict b,
                       ALIGNED int *restrict mask) {
    int i;
    
    for (i = 0; i < N; i++) {
        mask[i] = (a[i] < b[i]) ? -1 : 0;
    }
    
    int sum = 0;
    for (i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            sum += b[i];
        }
    }
    sink = sum;
}

/* Test LE_EXPR (<=) comparison */
void test_le_vectorize(ALIGNED int *restrict a, ALIGNED int *restrict b,
                       ALIGNED int *restrict c) {
    int i;
    
    for (i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? a[i] : b[i];
    }
    
    int count = 0;
    for (i = 0; i < N; i++) {
        count += (a[i] <= b[i]);
    }
    sink = count;
}

/* Test with smaller data type (short) to potentially trigger different paths */
void test_ge_vectorize_short(ALIGNED short *restrict a, ALIGNED short *restrict b,
                             ALIGNED short *restrict mask) {
    int i;
    
    for (i = 0; i < N; i++) {
        mask[i] = (a[i] >= b[i]) ? -1 : 0;
    }
}

int main() {
    int i;
    
    /* Allocate aligned arrays */
    ALIGNED int *a = malloc(N * sizeof(int));
    ALIGNED int *b = malloc(N * sizeof(int));
    ALIGNED int *c = malloc(N * sizeof(int));
    ALIGNED int *mask = malloc(N * sizeof(int));
    
    ALIGNED short *a_short = malloc(N * sizeof(short));
    ALIGNED short *b_short = malloc(N * sizeof(short));
    ALIGNED short *mask_short = malloc(N * sizeof(short));
    
    /* Initialize with pattern that creates mix of true/false comparisons */
    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N/2;  /* Half will be >= N/2, half < */
        c[i] = 0;
        mask[i] = 0;
        
        a_short[i] = (short)(i % 1000);
        b_short[i] = 500;
        mask_short[i] = 0;
    }
    
    /* Test all comparison types */
    test_ge_vectorize(a, b, c, mask);
    test_gt_vectorize(a, b, mask);
    test_lt_vectorize(a, b, mask);
    test_le_vectorize(a, b, c);
    test_ge_vectorize_short(a_short, b_short, mask_short);
    
    /* Simple checksum to ensure computations happen */
    int checksum = 0;
    for (i = 0; i < N; i++) {
        checksum += mask[i] + c[i] + mask_short[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Sink value: %d\n", sink);
    
    free(a);
    free(b);
    free(c);
    free(mask);
    free(a_short);
    free(b_short);
    free(mask_short);
    
    return 0;
}
