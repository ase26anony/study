/* Test program to trigger specific vector comparison expansion in tree-vect-stmts.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Function to test GE_EXPR vectorization with mask creation */
int test_ge_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* Pattern 1: Create mask from GE comparison */
    for (int i = 0; i < N; i++) {
        /* This creates a mask: -1 for true, 0 for false */
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    /* Pattern 2: Conditional sum using GE - may also trigger the path */
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    return sum;
}

/* Function to test GT_EXPR vectorization */
int test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? -1 : 0;
    }
    
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    
    return sum;
}

/* Function to test LT_EXPR vectorization */
int test_lt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? -1 : 0;
    }
    
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            sum += b[i];
        }
    }
    
    return sum;
}

/* Function to test LE_EXPR vectorization */
int test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? -1 : 0;
    }
    
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            sum += a[i] + b[i];
        }
    }
    
    return sum;
}

/* Alternative: Using short types might trigger different paths */
short test_ge_short_vectorize(ALIGNED short *a, ALIGNED short *b, ALIGNED short *c) {
    short sum = 0;
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? -1 : 0;
        sum += c[i];
    }
    
    return sum;
}

int main() {
    /* Use aligned arrays to help vectorization */
    ALIGNED int a[N], b[N], c[N];
    ALIGNED short as[N], bs[N], cs[N];
    
    /* Initialize with pattern that will trigger both true and false comparisons */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N/2;  /* Half will be >=, half < */
        c[i] = 0;
        
        as[i] = (short)(i % 1000);
        bs[i] = 500;  /* Half will be >=, half < */
        cs[i] = 0;
    }
    
    /* Test all comparison operators to cover the entire switch block */
    int sum_ge = test_ge_vectorize(a, b, c);
    int sum_gt = test_gt_vectorize(a, b, c);
    int sum_lt = test_lt_vectorize(a, b, c);
    int sum_le = test_le_vectorize(a, b, c);
    short sum_ge_short = test_ge_short_vectorize(as, bs, cs);
    
    /* Use results to prevent optimization */
    printf("Results:\n");
    printf("GE sum: %d\n", sum_ge);
    printf("GT sum: %d\n", sum_gt);
    printf("LT sum: %d\n", sum_lt);
    printf("LE sum: %d\n", sum_le);
    printf("GE short sum: %d\n", (int)sum_ge_short);
    
    /* Quick verification */
    int check = c[0] + c[N-1] + cs[0] + cs[N-1];
    printf("Check value: %d\n", check);
    
    return 0;
}
