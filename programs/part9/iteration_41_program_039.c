/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Function to test GE_EXPR vectorization */
int test_ge_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* Pattern 1: Conditional sum with >= comparison */
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    /* Pattern 2: Mask creation with >= comparison */
    for (int i = 0; i < N; i++) {
        /* Create mask: -1 for true, 0 for false */
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    /* Pattern 3: Conditional reduction with mask */
    int masked_sum = 0;
    for (int i = 0; i < N; i++) {
        masked_sum += (a[i] >= b[i]) ? a[i] : 0;
    }
    
    return sum + masked_sum + c[N/2]; /* Combine results to prevent optimization */
}

/* Function to test GT_EXPR vectorization */
int test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? -1 : 0;
    }
    
    return sum + c[N/2];
}

/* Function to test LT_EXPR vectorization */
int test_lt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            sum += b[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? -1 : 0;
    }
    
    return sum + c[N/2];
}

/* Function to test LE_EXPR vectorization */
int test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            sum += b[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? -1 : 0;
    }
    
    return sum + c[N/2];
}

/* Also test with short type to potentially trigger different paths */
short test_ge_short_vectorize(ALIGNED short *a, ALIGNED short *b, ALIGNED short *c) {
    short sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    return sum + c[N/2];
}

int main() {
    /* Declare aligned arrays */
    ALIGNED int a[N], b[N], c[N];
    ALIGNED short as[N], bs[N], cs[N];
    
    /* Initialize arrays with pattern that creates mixed comparison results */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N/2;  /* Half will be >=, half < */
        c[i] = 0;
        
        as[i] = (short)i;
        bs[i] = (short)(N/2);
        cs[i] = 0;
    }
    
    /* Test all comparison operators */
    int result_ge = test_ge_vectorize(a, b, c);
    int result_gt = test_gt_vectorize(a, b, c);
    int result_lt = test_lt_vectorize(a, b, c);
    int result_le = test_le_vectorize(a, b, c);
    short result_ge_short = test_ge_short_vectorize(as, bs, cs);
    
    /* Use volatile to prevent optimization */
    volatile int final_result = result_ge + result_gt + result_lt + result_le + result_ge_short;
    
    printf("Test results: %d (int GE) + %d (int GT) + %d (int LT) + %d (int LE) + %d (short GE) = %d\n",
           result_ge, result_gt, result_lt, result_le, result_ge_short, final_result);
    
    /* Simple checksum verification */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += c[i];
    }
    printf("Checksum of mask array: %d\n", checksum);
    
    return 0;
}
