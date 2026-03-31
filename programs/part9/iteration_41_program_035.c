/* Test program to trigger specific vector comparison expansion in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Function prototypes */
int test_ge_vectorize(void);
int test_gt_vectorize(void);
int test_lt_vectorize(void);
int test_le_vectorize(void);

/* Global aligned arrays to ensure vectorization */
ALIGNED int a[N];
ALIGNED int b[N];
ALIGNED int c[N];

/* Initialize arrays with pattern that creates mixed comparison results */
void init_arrays(void) {
    for (int i = 0; i < N; i++) {
        a[i] = i;                     /* 0, 1, 2, ..., N-1 */
        b[i] = N/2;                   /* All N/2 (512) */
        c[i] = 0;
    }
}

/* Test GE_EXPR case: count elements where a[i] >= b[i] */
int test_ge_vectorize(void) {
    int count = 0;
    
    /* This loop should vectorize with GE_EXPR comparison */
    for (int i = 0; i < N; i++) {
        /* Create mask from comparison result */
        int mask = (a[i] >= b[i]) ? -1 : 0;
        /* Use mask to conditionally increment count */
        count += mask & 1;
    }
    
    return count;
}

/* Test GT_EXPR case: count elements where a[i] > b[i] */
int test_gt_vectorize(void) {
    int count = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] > b[i]) ? -1 : 0;
        count += mask & 1;
    }
    
    return count;
}

/* Test LT_EXPR case: count elements where a[i] < b[i] */
int test_lt_vectorize(void) {
    int count = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] < b[i]) ? -1 : 0;
        count += mask & 1;
    }
    
    return count;
}

/* Test LE_EXPR case: count elements where a[i] <= b[i] */
int test_le_vectorize(void) {
    int count = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] <= b[i]) ? -1 : 0;
        count += mask & 1;
    }
    
    return count;
}

/* Alternative test: mask-based array operation */
void test_mask_operation(void) {
    /* This creates a mask pattern that might trigger the specific expansion */
    for (int i = 0; i < N; i++) {
        /* Create mask using comparison, then use in bitwise operation */
        int mask = (a[i] >= b[i]) ? -1 : 0;
        c[i] = (a[i] & mask) | (b[i] & ~mask);
    }
}

/* Another pattern: conditional reduction with comparison */
int test_conditional_reduction(void) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    return sum;
}

int main(void) {
    int result;
    
    /* Initialize test data */
    init_arrays();
    
    printf("Testing vector comparison expansions in GCC...\n");
    
    /* Test GE_EXPR case (primary target) */
    result = test_ge_vectorize();
    printf("GE_EXPR test result (a[i] >= b[i] count): %d (expected: %d)\n", 
           result, N - N/2);
    
    /* Test GT_EXPR case */
    result = test_gt_vectorize();
    printf("GT_EXPR test result (a[i] > b[i] count): %d (expected: %d)\n", 
           result, N - N/2 - 1);
    
    /* Test LT_EXPR case */
    result = test_lt_vectorize();
    printf("LT_EXPR test result (a[i] < b[i] count): %d (expected: %d)\n", 
           result, N/2);
    
    /* Test LE_EXPR case */
    result = test_le_vectorize();
    printf("LE_EXPR test result (a[i] <= b[i] count): %d (expected: %d)\n", 
           result, N/2 + 1);
    
    /* Test mask operation */
    test_mask_operation();
    
    /* Verify mask operation produced correct results */
    int verify = 0;
    for (int i = 0; i < N; i++) {
        int expected = (a[i] >= b[i]) ? a[i] : b[i];
        verify += (c[i] == expected) ? 0 : 1;
    }
    printf("Mask operation verification errors: %d\n", verify);
    
    /* Test conditional reduction */
    result = test_conditional_reduction();
    printf("Conditional reduction sum (a[i] >= b[i]): %d\n", result);
    
    return 0;
}
