/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Force the compiler to keep computations */
static volatile int sink;

/* Test GE_EXPR (>=) comparison */
void test_ge_vectorize(ALIGNED int *restrict a, ALIGNED int *restrict b, 
                       ALIGNED int *restrict c, ALIGNED int *restrict mask) {
    for (int i = 0; i < N; i++) {
        /* Create mask from GE comparison - this should trigger the 
           BIT_NOT_EXPR + BIT_IOR_EXPR expansion */
        mask[i] = (a[i] >= b[i]) ? -1 : 0;
        
        /* Use the mask in a conditional operation */
        c[i] = (a[i] >= b[i]) ? a[i] + b[i] : a[i] - b[i];
    }
}

/* Test GT_EXPR (>) comparison */
void test_gt_vectorize(ALIGNED int *restrict a, ALIGNED int *restrict b,
                       ALIGNED int *restrict c) {
    for (int i = 0; i < N; i++) {
        /* Conditional reduction using > comparison */
        if (a[i] > b[i]) {
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] / 2;
        }
    }
}

/* Test LT_EXPR (<) comparison */
void test_lt_vectorize(ALIGNED short *restrict a, ALIGNED short *restrict b,
                       ALIGNED short *restrict c) {
    for (int i = 0; i < N; i++) {
        /* Different data type to test different paths */
        c[i] = (a[i] < b[i]) ? a[i] : b[i];
    }
}

/* Test LE_EXPR (<=) comparison with reduction */
int test_le_reduction(ALIGNED int *restrict a, ALIGNED int *restrict b) {
    int count = 0;
    for (int i = 0; i < N; i++) {
        /* Count elements where a[i] <= b[i] */
        if (a[i] <= b[i]) {
            count++;
        }
    }
    return count;
}

/* Test mixed comparisons in same loop */
void test_mixed_comparisons(ALIGNED int *restrict a, ALIGNED int *restrict b,
                            ALIGNED int *restrict c) {
    for (int i = 0; i < N; i++) {
        /* Use multiple comparison types */
        int ge_mask = (a[i] >= b[i]) ? -1 : 0;
        int gt_mask = (a[i] > b[i]) ? -1 : 0;
        int lt_mask = (a[i] < b[i]) ? -1 : 0;
        int le_mask = (a[i] <= b[i]) ? -1 : 0;
        
        /* Combine masks in a way that can't be easily optimized away */
        c[i] = (ge_mask & a[i]) | (gt_mask & b[i]) | 
               (lt_mask & (a[i] + b[i])) | (le_mask & (a[i] - b[i]));
    }
}

int main() {
    /* Aligned arrays to help vectorization */
    ALIGNED int a_int[N], b_int[N], c_int[N], mask[N];
    ALIGNED short a_short[N], b_short[N], c_short[N];
    
    /* Initialize with pattern that creates mixed comparison results */
    for (int i = 0; i < N; i++) {
        a_int[i] = i;
        b_int[i] = N/2;  /* Half will be >=, half < */
        
        a_short[i] = (short)(i * 3);
        b_short[i] = (short)(i * 2 + 100);
    }
    
    /* Test GE_EXPR - primary target */
    test_ge_vectorize(a_int, b_int, c_int, mask);
    
    /* Test GT_EXPR */
    test_gt_vectorize(a_int, b_int, c_int);
    
    /* Test LT_EXPR with different data type */
    test_lt_vectorize(a_short, b_short, c_short);
    
    /* Test LE_EXPR with reduction */
    int le_count = test_le_reduction(a_int, b_int);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(a_int, b_int, c_int);
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += c_int[i] + mask[i] + c_short[i];
    }
    checksum += le_count;
    
    sink = checksum;  /* Force computation */
    
    printf("Checksum: %d\n", checksum);
    printf("LE count: %d\n", le_count);
    
    return 0;
}
