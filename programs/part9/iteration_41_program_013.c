/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Function to prevent optimization */
static void escape(void *p) {
    asm volatile("" : : "g"(p) : "memory");
}

/* Test GE_EXPR case - create mask from >= comparison */
void test_ge_vectorize(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        /* This creates a mask: -1 for true, 0 for false */
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
}

/* Test GT_EXPR case - create mask from > comparison */
void test_gt_vectorize(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? -1 : 0;
    }
}

/* Test LT_EXPR case - create mask from < comparison */
void test_lt_vectorize(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? -1 : 0;
    }
}

/* Test LE_EXPR case - create mask from <= comparison */
void test_le_vectorize(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? -1 : 0;
    }
}

/* Conditional reduction using >= */
int test_ge_reduction(int *restrict a, int *restrict b) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

/* Conditional select using >= */
void test_ge_select(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
}

int main() {
    /* Use aligned arrays to help vectorization */
    ALIGNED int a[N], b[N], c[N];
    int i, result;
    
    /* Initialize with pattern that will trigger comparisons */
    for (i = 0; i < N; i++) {
        a[i] = i;               /* 0, 1, 2, ..., N-1 */
        b[i] = N/2;             /* All N/2 */
    }
    
    /* Prevent compiler from optimizing away arrays */
    escape(a);
    escape(b);
    escape(c);
    
    printf("Testing vector comparison expansions...\n");
    
    /* Test GE_EXPR - primary target */
    test_ge_vectorize(a, b, c);
    result = 0;
    for (i = 0; i < N; i++) result += c[i];
    printf("GE mask sum: %d (expected: %d)\n", result, -(N - N/2 - 1));
    
    /* Test GT_EXPR */
    test_gt_vectorize(a, b, c);
    result = 0;
    for (i = 0; i < N; i++) result += c[i];
    printf("GT mask sum: %d (expected: %d)\n", result, -(N - N/2 - 1));
    
    /* Test LT_EXPR */
    test_lt_vectorize(a, b, c);
    result = 0;
    for (i = 0; i < N; i++) result += c[i];
    printf("LT mask sum: %d (expected: %d)\n", result, -(N/2));
    
    /* Test LE_EXPR */
    test_le_vectorize(a, b, c);
    result = 0;
    for (i = 0; i < N; i++) result += c[i];
    printf("LE mask sum: %d (expected: %d)\n", result, -(N/2 + 1));
    
    /* Test conditional reduction */
    result = test_ge_reduction(a, b);
    printf("GE reduction sum: %d\n", result);
    
    /* Test conditional select */
    test_ge_select(a, b, c);
    result = 0;
    for (i = 0; i < N; i++) result += c[i];
    printf("GE select sum: %d\n", result);
    
    return 0;
}
