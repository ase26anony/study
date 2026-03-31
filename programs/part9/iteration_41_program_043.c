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
ALIGNED int d[N];

/* Initialize arrays with pattern that creates mixed comparison results */
void init_arrays(void) {
    for (int i = 0; i < N; i++) {
        a[i] = i;                     /* 0, 1, 2, ..., N-1 */
        b[i] = N/2;                   /* All N/2 (512) */
        c[i] = (i % 3 == 0) ? i : -i; /* Mixed positive/negative */
        d[i] = i * 2;                 /* Always larger than a[i] */
    }
}

/* Test GE_EXPR (>=) - target case */
int test_ge_vectorize(void) {
    int sum = 0;
    
    /* Pattern 1: Conditional sum with >= comparison */
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    /* Pattern 2: Mask creation - likely to trigger the specific expansion */
    /* This creates a mask of -1 (true) or 0 (false) */
    ALIGNED int mask[N];
    for (int i = 0; i < N; i++) {
        mask[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    /* Use the mask in computation to prevent optimization */
    int masked_sum = 0;
    for (int i = 0; i < N; i++) {
        masked_sum += mask[i] & c[i];
    }
    
    return sum + masked_sum;
}

/* Test GT_EXPR (>) - adjacent case */
int test_gt_vectorize(void) {
    int sum = 0;
    
    /* Conditional reduction with > */
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    
    /* Mask creation */
    ALIGNED int mask[N];
    for (int i = 0; i < N; i++) {
        mask[i] = (a[i] > b[i]) ? -1 : 0;
    }
    
    int masked_sum = 0;
    for (int i = 0; i < N; i++) {
        masked_sum += mask[i] & c[i];
    }
    
    return sum + masked_sum;
}

/* Test LT_EXPR (<) - adjacent case with swap */
int test_lt_vectorize(void) {
    int sum = 0;
    
    /* Conditional reduction with < */
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    
    /* Mask creation */
    ALIGNED int mask[N];
    for (int i = 0; i < N; i++) {
        mask[i] = (a[i] < b[i]) ? -1 : 0;
    }
    
    int masked_sum = 0;
    for (int i = 0; i < N; i++) {
        masked_sum += mask[i] | c[i];  /* Use OR to vary pattern */
    }
    
    return sum + masked_sum;
}

/* Test LE_EXPR (<=) - adjacent case with swap */
int test_le_vectorize(void) {
    int sum = 0;
    
    /* Conditional reduction with <= */
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            sum += a[i];
        }
    }
    
    /* Mask creation - using different arrays to vary pattern */
    ALIGNED int mask[N];
    for (int i = 0; i < N; i++) {
        mask[i] = (c[i] <= d[i]) ? -1 : 0;
    }
    
    int masked_sum = 0;
    for (int i = 0; i < N; i++) {
        masked_sum += mask[i] | a[i];  /* Use OR to vary pattern */
    }
    
    return sum + masked_sum;
}

/* Alternative test focusing specifically on mask creation patterns */
void test_mask_patterns(void) {
    ALIGNED int result1[N], result2[N], result3[N], result4[N];
    
    /* Direct mask creation patterns that might trigger the expansion */
    for (int i = 0; i < N; i++) {
        /* GE mask */
        result1[i] = (a[i] >= b[i]) ? -1 : 0;
        /* GT mask */
        result2[i] = (a[i] > b[i]) ? -1 : 0;
        /* LT mask */
        result3[i] = (a[i] < b[i]) ? -1 : 0;
        /* LE mask */
        result4[i] = (c[i] <= d[i]) ? -1 : 0;
    }
    
    /* Use results to prevent optimization */
    volatile int sink = 0;
    for (int i = 0; i < N; i++) {
        sink += result1[i] + result2[i] + result3[i] + result4[i];
    }
}

int main(void) {
    init_arrays();
    
    printf("Testing vector comparison expansions...\n");
    
    /* Call all test functions to trigger different comparison expansions */
    int result_ge = test_ge_vectorize();
    int result_gt = test_gt_vectorize();
    int result_lt = test_lt_vectorize();
    int result_le = test_le_vectorize();
    
    /* Also test mask patterns */
    test_mask_patterns();
    
    /* Combine results to create observable output */
    int total = result_ge + result_gt + result_lt + result_le;
    printf("Total result: %d\n", total);
    printf("GE: %d, GT: %d, LT: %d, LE: %d\n", 
           result_ge, result_gt, result_lt, result_le);
    
    return 0;
}
