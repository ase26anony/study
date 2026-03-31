/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc */
/* Specifically targets GE_EXPR, GT_EXPR, LT_EXPR, LE_EXPR cases */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

/* Function to test GE_EXPR vectorization */
int test_ge_vectorize(ALIGNED int a[N], ALIGNED int b[N], ALIGNED int c[N]) {
    int sum = 0;
    
    /* This should trigger GE_EXPR expansion */
    for (int i = 0; i < N; i++) {
        /* Create mask from comparison: -1 for true, 0 for false */
        int mask = (a[i] >= b[i]) ? -1 : 0;
        /* Use mask in computation to prevent optimization */
        c[i] = mask & a[i];
        sum += c[i];
    }
    return sum;
}

/* Function to test GT_EXPR vectorization */
int test_gt_vectorize(ALIGNED int a[N], ALIGNED int b[N], ALIGNED int c[N]) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] > b[i]) ? -1 : 0;
        c[i] = mask & b[i];
        sum += c[i];
    }
    return sum;
}

/* Function to test LT_EXPR vectorization */
int test_lt_vectorize(ALIGNED int a[N], ALIGNED int b[N], ALIGNED int c[N]) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] < b[i]) ? -1 : 0;
        c[i] = mask | a[i];
        sum += c[i];
    }
    return sum;
}

/* Function to test LE_EXPR vectorization */
int test_le_vectorize(ALIGNED int a[N], ALIGNED int b[N], ALIGNED int c[N]) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] <= b[i]) ? -1 : 0;
        c[i] = mask | b[i];
        sum += c[i];
    }
    return sum;
}

/* Alternative test with conditional reduction */
int test_ge_reduction(ALIGNED short a[N], ALIGNED short b[N]) {
    int count = 0;
    
    /* Count elements where a[i] >= b[i] */
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            count++;
        }
    }
    return count;
}

/* Test with mixed types to explore different paths */
int test_mixed_comparisons(ALIGNED int a[N], ALIGNED int b[N], ALIGNED int c[N]) {
    int sum = 0;
    
    /* Mix different comparisons in one loop */
    for (int i = 0; i < N; i += 4) {
        /* Use different comparisons to potentially trigger different expansions */
        c[i] = (a[i] >= b[i]) ? a[i] : b[i];
        c[i+1] = (a[i+1] > b[i+1]) ? a[i+1] : b[i+1];
        c[i+2] = (a[i+2] < b[i+2]) ? a[i+2] : b[i+2];
        c[i+3] = (a[i+3] <= b[i+3]) ? a[i+3] : b[i+3];
        
        sum += c[i] + c[i+1] + c[i+2] + c[i+3];
    }
    return sum;
}

int main() {
    ALIGNED int a[N], b[N], c[N];
    ALIGNED short a_short[N], b_short[N];
    int result = 0;
    
    /* Initialize arrays with pattern that will trigger comparisons */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N/2;
        a_short[i] = (short)(i % 1000);
        b_short[i] = 500;
    }
    
    printf("Testing vector comparison expansions...\n");
    
    /* Test GE_EXPR */
    result = test_ge_vectorize(a, b, c);
    printf("GE_EXPR test result: %d\n", result);
    
    /* Test GT_EXPR */
    result = test_gt_vectorize(a, b, c);
    printf("GT_EXPR test result: %d\n", result);
    
    /* Test LT_EXPR */
    result = test_lt_vectorize(a, b, c);
    printf("LT_EXPR test result: %d\n", result);
    
    /* Test LE_EXPR */
    result = test_le_vectorize(a, b, c);
    printf("LE_EXPR test result: %d\n", result);
    
    /* Test conditional reduction with GE_EXPR */
    result = test_ge_reduction(a_short, b_short);
    printf("GE_EXPR reduction result: %d\n", result);
    
    /* Test mixed comparisons */
    result = test_mixed_comparisons(a, b, c);
    printf("Mixed comparisons result: %d\n", result);
    
    /* Use volatile to prevent optimization */
    volatile int final_check = result;
    
    return 0;
}
