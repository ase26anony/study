/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Function to prevent optimization */
static void escape(void *p) {
    asm volatile("" : : "g"(p) : "memory");
}

/* Test GE_EXPR case - greater than or equal */
int test_ge_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* Loop that should vectorize with GE_EXPR comparison */
    for (int i = 0; i < N; i++) {
        /* Create mask from comparison: -1 if a[i] >= b[i], 0 otherwise */
        int mask = (a[i] >= b[i]) ? -1 : 0;
        
        /* Use mask to conditionally add to sum */
        sum += mask & a[i];
        
        /* Also store mask to array to ensure it's used */
        c[i] = mask;
    }
    
    return sum;
}

/* Test GT_EXPR case - greater than */
int test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] > b[i]) ? -1 : 0;
        sum += mask & a[i];
        c[i] = mask;
    }
    
    return sum;
}

/* Test LT_EXPR case - less than */
int test_lt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] < b[i]) ? -1 : 0;
        sum += mask & a[i];
        c[i] = mask;
    }
    
    return sum;
}

/* Test LE_EXPR case - less than or equal */
int test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] <= b[i]) ? -1 : 0;
        sum += mask & a[i];
        c[i] = mask;
    }
    
    return sum;
}

/* Alternative test using conditional reduction pattern */
unsigned int test_ge_count(ALIGNED short *a, ALIGNED short *b) {
    unsigned int count = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            count++;
        }
    }
    
    return count;
}

int main() {
    /* Allocate aligned arrays */
    ALIGNED int a_int[N], b_int[N], c_int[N];
    ALIGNED short a_short[N], b_short[N];
    
    /* Initialize with pattern that creates mixed comparison results */
    for (int i = 0; i < N; i++) {
        a_int[i] = i;
        b_int[i] = N/2;  /* Half will be >=, half < */
        
        a_short[i] = i % 256;
        b_short[i] = 128;  /* Mixed results */
    }
    
    /* Prevent optimization of arrays */
    escape(a_int);
    escape(b_int);
    escape(c_int);
    escape(a_short);
    escape(b_short);
    
    /* Test all comparison operators */
    int sum_ge = test_ge_vectorize(a_int, b_int, c_int);
    int sum_gt = test_gt_vectorize(a_int, b_int, c_int);
    int sum_lt = test_lt_vectorize(a_int, b_int, c_int);
    int sum_le = test_le_vectorize(a_int, b_int, c_int);
    unsigned int count_ge = test_ge_count(a_short, b_short);
    
    /* Use results to prevent dead code elimination */
    printf("Results:\n");
    printf("GE sum: %d\n", sum_ge);
    printf("GT sum: %d\n", sum_gt);
    printf("LT sum: %d\n", sum_lt);
    printf("LE sum: %d\n", sum_le);
    printf("GE count: %u\n", count_ge);
    
    /* Simple checksum to verify execution */
    int checksum = sum_ge + sum_gt + sum_lt + sum_le + count_ge;
    printf("Checksum: %d\n", checksum);
    
    /* Verify some mask values */
    printf("Sample masks (indices 0, %d, %d): %d, %d, %d\n", 
           N/4, N-1, c_int[0], c_int[N/4], c_int[N-1]);
    
    return 0;
}
