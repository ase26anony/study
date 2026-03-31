/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Force compiler to keep computations */
static volatile int sink;

/* Test GE_EXPR (>=) case - primary target */
void test_ge_vectorize(ALIGNED int *restrict a, ALIGNED int *restrict b, 
                       ALIGNED int *restrict c, ALIGNED int *restrict mask) {
    int i;
    
    /* Pattern 1: Conditional mask creation (likely to use bitwise expansion) */
    for (i = 0; i < N; i++) {
        /* Create mask: -1 if a[i] >= b[i], 0 otherwise */
        mask[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    /* Pattern 2: Conditional reduction using mask */
    int sum = 0;
    for (i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    /* Pattern 3: Blend operation using comparison */
    for (i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
    
    sink = sum; /* Prevent optimization */
}

/* Test GT_EXPR (>) case */
void test_gt_vectorize(ALIGNED int *restrict a, ALIGNED int *restrict b, 
                       ALIGNED int *restrict mask) {
    int i;
    
    for (i = 0; i < N; i++) {
        mask[i] = (a[i] > b[i]) ? -1 : 0;
    }
    
    int count = 0;
    for (i = 0; i < N; i++) {
        count += (a[i] > b[i]) ? 1 : 0;
    }
    
    sink = count;
}

/* Test LT_EXPR (<) case */
void test_lt_vectorize(ALIGNED int *restrict a, ALIGNED int *restrict b, 
                       ALIGNED int *restrict mask) {
    int i;
    
    for (i = 0; i < N; i++) {
        mask[i] = (a[i] < b[i]) ? -1 : 0;
    }
    
    int sum = 0;
    for (i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            sum += b[i];
        }
    }
    
    sink = sum;
}

/* Test LE_EXPR (<=) case */
void test_le_vectorize(ALIGNED int *restrict a, ALIGNED int *restrict b, 
                       ALIGNED int *restrict c) {
    int i;
    
    for (i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? a[i] : b[i];
    }
    
    int count = 0;
    for (i = 0; i < N; i++) {
        count += (a[i] <= b[i]);
    }
    
    sink = count;
}

/* Also test with short type to potentially trigger different paths */
void test_ge_vectorize_short(ALIGNED short *restrict a, ALIGNED short *restrict b,
                             ALIGNED short *restrict mask) {
    int i;
    
    for (i = 0; i < N; i++) {
        mask[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    short sum = 0;
    for (i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    sink = sum;
}

int main() {
    /* Aligned arrays for vectorization */
    ALIGNED int a[N], b[N], c[N], mask[N];
    ALIGNED short as[N], bs[N], masks[N];
    
    /* Initialize with pattern that creates mix of true/false comparisons */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N/2;  /* Half will be >=, half < */
        c[i] = 0;
        mask[i] = 0;
        
        as[i] = (short)i;
        bs[i] = (short)(N/4);
        masks[i] = 0;
    }
    
    /* Test all comparison types */
    test_ge_vectorize(a, b, c, mask);
    test_gt_vectorize(a, b, mask);
    test_lt_vectorize(a, b, mask);
    test_le_vectorize(a, b, c);
    test_ge_vectorize_short(as, bs, masks);
    
    /* Simple checksum to ensure computations happen */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += mask[i] + c[i] + masks[i];
    }
    
    printf("Checksum: %d (sink: %d)\n", checksum, sink);
    printf("Test completed - vectorization should have been attempted\n");
    
    return 0;
}
