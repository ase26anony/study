/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Function to test GE_EXPR vectorization */
int test_ge_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* Loop with GE_EXPR comparison creating mask pattern */
    for (int i = 0; i < N; i++) {
        /* Create mask: -1 if a[i] >= b[i], 0 otherwise */
        int mask = (a[i] >= b[i]) ? -1 : 0;
        /* Use mask to conditionally accumulate values */
        c[i] = mask & a[i];
        sum += c[i];
    }
    
    return sum;
}

/* Function to test GT_EXPR vectorization */
int test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] > b[i]) ? -1 : 0;
        c[i] = mask & b[i];
        sum += c[i];
    }
    
    return sum;
}

/* Function to test LT_EXPR vectorization */
int test_lt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] < b[i]) ? -1 : 0;
        c[i] = mask & a[i];
        sum += c[i];
    }
    
    return sum;
}

/* Function to test LE_EXPR vectorization */
int test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] <= b[i]) ? -1 : 0;
        c[i] = mask & b[i];
        sum += c[i];
    }
    
    return sum;
}

/* Alternative test using conditional reduction */
unsigned test_ge_count(ALIGNED short *a, ALIGNED short *b) {
    unsigned count = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            count++;
        }
    }
    
    return count;
}

int main() {
    /* Aligned arrays to enable vectorization */
    ALIGNED int a_int[N], b_int[N], c_int[N];
    ALIGNED short a_short[N], b_short[N];
    
    /* Initialize with pattern that creates mixed true/false comparisons */
    for (int i = 0; i < N; i++) {
        a_int[i] = i;
        b_int[i] = N/2;  /* Half will be >=, half < */
        
        a_short[i] = (short)(i % 256);
        b_short[i] = 128;
    }
    
    printf("Testing vector comparison expansions...\n");
    
    /* Test GE_EXPR - should trigger the specific uncovered case */
    int sum_ge = test_ge_vectorize(a_int, b_int, c_int);
    printf("GE_EXPR result: %d\n", sum_ge);
    
    /* Test GT_EXPR */
    int sum_gt = test_gt_vectorize(a_int, b_int, c_int);
    printf("GT_EXPR result: %d\n", sum_gt);
    
    /* Test LT_EXPR */
    int sum_lt = test_lt_vectorize(a_int, b_int, c_int);
    printf("LT_EXPR result: %d\n", sum_lt);
    
    /* Test LE_EXPR */
    int sum_le = test_le_vectorize(a_int, b_int, c_int);
    printf("LE_EXPR result: %d\n", sum_le);
    
    /* Test with different type (short) */
    unsigned count = test_ge_count(a_short, b_short);
    printf("GE_EXPR count (short): %u\n", count);
    
    /* Use volatile to prevent optimization */
    volatile int check = sum_ge + sum_gt + sum_lt + sum_le + count;
    printf("Final check value: %d\n", (int)check);
    
    return 0;
}
