/* Program to trigger vectorization of comparison operations for
   GT_EXPR, GE_EXPR, LT_EXPR, and LE_EXPR transformations in tree-vect-stmts.cc */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define SIZE2 128

/* Use volatile for some arrays to prevent premature optimization */
volatile int global_seed = 12345;

/* Simple LCG for semi-random data */
static inline int lcg(int *state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

/* Initialize arrays with semi-random data */
void init_arrays(int *a, int *b, int *c, int size) {
    int state = global_seed;
    for (int i = 0; i < size; i++) {
        a[i] = lcg(&state) % 1000;
        b[i] = lcg(&state) % 1000;
        c[i] = lcg(&state) % 1000;
    }
}

/* Test functions for each comparison operator */

/* GT_EXPR - greater than */
void test_gt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GT_EXPR in the vectorizer */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* GE_EXPR - greater than or equal */
void test_ge_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GE_EXPR in the vectorizer */
        if (a[i] >= b[i]) {
            dst[i] = a[i] * 2 + c[i];
        } else {
            dst[i] = b[i] * 2 - c[i];
        }
    }
}

/* LT_EXPR - less than */
void test_lt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LT_EXPR in the vectorizer */
        if (a[i] < b[i]) {
            dst[i] = a[i] - c[i];
        } else {
            dst[i] = b[i] + c[i];
        }
    }
}

/* LE_EXPR - less than or equal */
void test_le_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LE_EXPR in the vectorizer */
        if (a[i] <= b[i]) {
            dst[i] = a[i] + b[i] + c[i];
        } else {
            dst[i] = a[i] - b[i] - c[i];
        }
    }
}

/* Test with different integer types (short) */
void test_gt_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* GT_EXPR with short type */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

void test_lt_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* LT_EXPR with short type */
        if (a[i] < b[i]) {
            dst[i] = a[i] - c[i];
        } else {
            dst[i] = b[i] + c[i];
        }
    }
}

/* Test with mixed comparisons in same loop */
void test_mixed_comparisons(int *dst1, int *dst2, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Mix GT and LT in same loop */
        if (a[i] > b[i]) {
            dst1[i] = a[i] + c[i];
        } else {
            dst1[i] = b[i];
        }
        
        if (a[i] <= c[i]) {
            dst2[i] = b[i] - a[i];
        } else {
            dst2[i] = c[i];
        }
    }
}

/* Main function with checksum computation */
int main() {
    /* Declare arrays with different sizes */
    int a[SIZE], b[SIZE], c[SIZE];
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    int dst5[SIZE2], dst6[SIZE2];
    
    short sa[SIZE], sb[SIZE], sc[SIZE];
    short sdst1[SIZE], sdst2[SIZE];
    
    /* Initialize with semi-random data */
    init_arrays(a, b, c, SIZE);
    init_arrays(dst5, dst6, a, SIZE2); /* Reuse a for initialization */
    
    /* Initialize short arrays */
    int state = global_seed;
    for (int i = 0; i < SIZE; i++) {
        sa[i] = lcg(&state) % 1000;
        sb[i] = lcg(&state) % 1000;
        sc[i] = lcg(&state) % 1000;
    }
    
    /* Call test functions for each comparison operator */
    test_gt_loop(dst1, a, b, c, SIZE);
    test_ge_loop(dst2, a, b, c, SIZE);
    test_lt_loop(dst3, a, b, c, SIZE);
    test_le_loop(dst4, a, b, c, SIZE);
    
    /* Test with different size */
    test_gt_loop(dst5, a, b, c, SIZE2);
    test_lt_loop(dst6, a, b, c, SIZE2);
    
    /* Test with short type */
    test_gt_loop_short(sdst1, sa, sb, sc, SIZE);
    test_lt_loop_short(sdst2, sa, sb, sc, SIZE);
    
    /* Test mixed comparisons */
    int mixed1[SIZE], mixed2[SIZE];
    test_mixed_comparisons(mixed1, mixed2, a, b, c, SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i];
        checksum += sdst1[i] + sdst2[i];
        checksum += mixed1[i] + mixed2[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst5[i] + dst6[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
