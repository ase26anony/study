/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc
 * Targets the switch case for GE_EXPR (and GT_EXPR, LT_EXPR, LE_EXPR)
 * Compile with: gcc -O3 -ftree-vectorize -fno-vect-cost-model -fdump-tree-vect-details
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Function to test GE_EXPR vectorization */
int test_ge_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* Pattern 1: Conditional sum using >= */
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    /* Pattern 2: Create mask from >= comparison */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? -1 : 0;  /* Creates all-ones or all-zeros mask */
    }
    
    /* Pattern 3: Conditional assignment using mask */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
    
    return sum;
}

/* Function to test GT_EXPR vectorization */
int test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    
    /* Create mask from > comparison */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? -1 : 0;
    }
    
    return sum;
}

/* Function to test LT_EXPR vectorization */
int test_lt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    
    /* Create mask from < comparison */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? -1 : 0;
    }
    
    return sum;
}

/* Function to test LE_EXPR vectorization */
int test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            sum += a[i];
        }
    }
    
    /* Create mask from <= comparison */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? -1 : 0;
    }
    
    return sum;
}

/* Test with short type to potentially trigger different paths */
short test_short_ge_vectorize(ALIGNED short *a, ALIGNED short *b, ALIGNED short *c) {
    short sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? (short)-1 : (short)0;
    }
    
    return sum;
}

int main() {
    /* Aligned arrays to help vectorization */
    ALIGNED int a_int[N], b_int[N], c_int[N];
    ALIGNED short a_short[N], b_short[N], c_short[N];
    
    /* Initialize with pattern that creates mixed comparison results */
    for (int i = 0; i < N; i++) {
        a_int[i] = i;
        b_int[i] = N/2;  /* Half will be >=, half < */
        
        a_short[i] = (short)(i % 100);
        b_short[i] = 50;  /* Mixed comparisons */
    }
    
    /* Volatile variables to prevent optimization */
    volatile int result_ge, result_gt, result_lt, result_le;
    volatile short result_short_ge;
    
    /* Test all comparison types */
    result_ge = test_ge_vectorize(a_int, b_int, c_int);
    result_gt = test_gt_vectorize(a_int, b_int, c_int);
    result_lt = test_lt_vectorize(a_int, b_int, c_int);
    result_le = test_le_vectorize(a_int, b_int, c_int);
    result_short_ge = test_short_ge_vectorize(a_short, b_short, c_short);
    
    /* Simple checksum to ensure code executed */
    int checksum = result_ge + result_gt + result_lt + result_le + result_short_ge;
    
    printf("Results: GE=%d, GT=%d, LT=%d, LE=%d, Short_GE=%d\n", 
           result_ge, result_gt, result_lt, result_le, result_short_ge);
    printf("Checksum: %d\n", checksum);
    
    /* Use results to prevent dead code elimination */
    if (checksum != 0) {
        printf("Test completed successfully.\n");
    }
    
    return 0;
}
