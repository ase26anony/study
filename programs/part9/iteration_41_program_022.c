/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc
 * Specifically targets the switch case for GE_EXPR (and other comparisons)
 * that expands to BIT_NOT_EXPR and BIT_IOR_EXPR sequences.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Force the compiler to consider all code paths */
static int volatile result;

/* Test GE_EXPR (>=) comparison - primary target */
void test_ge_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int i;
    /* Pattern 1: Create mask from comparison (likely to use bitwise expansion) */
    for (i = 0; i < N; i++) {
        /* This creates a mask: -1 for true, 0 for false */
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    /* Pattern 2: Conditional reduction using comparison */
    int sum = 0;
    for (i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    result = sum;
}

/* Test GT_EXPR (>) comparison */
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
    result = sum;
}

/* Test LT_EXPR (<) comparison */
void test_lt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
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
    result = sum;
}

/* Test LE_EXPR (<=) comparison */
void test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
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
    result = sum;
}

/* Test with short type (different vector size might trigger different paths) */
void test_ge_vectorize_short(ALIGNED short *a, ALIGNED short *b, ALIGNED short *c) {
    int i;
    for (i = 0; i < N * 2; i++) {  /* More iterations for shorts */
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    short sum = 0;
    for (i = 0; i < N * 2; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    result = sum;
}

int main() {
    int i;
    
    /* Allocate aligned arrays */
    ALIGNED int *a_int = (ALIGNED int*)malloc(N * sizeof(int));
    ALIGNED int *b_int = (ALIGNED int*)malloc(N * sizeof(int));
    ALIGNED int *c_int = (ALIGNED int*)malloc(N * sizeof(int));
    
    ALIGNED short *a_short = (ALIGNED short*)malloc(N * 2 * sizeof(short));
    ALIGNED short *b_short = (ALIGNED short*)malloc(N * 2 * sizeof(short));
    ALIGNED short *c_short = (ALIGNED short*)malloc(N * 2 * sizeof(short));
    
    /* Initialize with pattern that creates mix of true/false comparisons */
    for (i = 0; i < N; i++) {
        a_int[i] = i;
        b_int[i] = N/2;  /* Half will be >=, half < */
        
        if (i < N/4) {
            a_int[i] = N;  /* Force some > comparisons */
        } else if (i < N/2) {
            a_int[i] = N/2; /* Force some == comparisons */
        }
    }
    
    for (i = 0; i < N * 2; i++) {
        a_short[i] = i % 256;
        b_short[i] = 128;
    }
    
    /* Run all tests to cover different comparison operators */
    test_ge_vectorize(a_int, b_int, c_int);
    printf("GE test result (mask): %d\n", c_int[N/4]);
    
    test_gt_vectorize(a_int, b_int, c_int);
    printf("GT test result (mask): %d\n", c_int[N/4]);
    
    test_lt_vectorize(a_int, b_int, c_int);
    printf("LT test result (mask): %d\n", c_int[3*N/4]);
    
    test_le_vectorize(a_int, b_int, c_int);
    printf("LE test result (mask): %d\n", c_int[N/2]);
    
    test_ge_vectorize_short(a_short, b_short, c_short);
    printf("GE short test result (mask): %d\n", c_short[64]);
    
    /* Verify some results */
    int check = 0;
    for (i = 0; i < 16; i++) {
        check += c_int[i];
    }
    printf("Check sum: %d\n", check);
    
    free(a_int);
    free(b_int);
    free(c_int);
    free(a_short);
    free(b_short);
    free(c_short);
    
    return 0;
}
