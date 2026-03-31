/* Program to trigger vectorization of comparison operations for
   GT_EXPR, GE_EXPR, LT_EXPR, and LE_EXPR transformations in tree-vect-stmts.cc
   Compile with: gcc -O3 -ftree-vectorize -fdump-tree-vect-details -march=native */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 42;

/* Simple LCG for semi-random data */
static inline int lcg_rand(int *state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

/* Initialize arrays with semi-random data */
void init_arrays(int *a, int *b, int *c, int size) {
    int state = global_seed;
    for (int i = 0; i < size; i++) {
        a[i] = lcg_rand(&state) % 1000;
        b[i] = lcg_rand(&state) % 1000;
        c[i] = lcg_rand(&state) % 1000;
    }
}

/* Test function for GT_EXPR (greater-than) */
void test_gt_loop(int *dst, const int *a, const int *b, const int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* Test function for GE_EXPR (greater-than-or-equal) */
void test_ge_loop(short *dst, const short *a, const short *b, const short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR */
        if (a[i] >= b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* Test function for LT_EXPR (less-than) */
void test_lt_loop(int *dst, const int *a, const int *b, const int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
        if (a[i] < b[i]) {
            dst[i] = a[i] * c[i];
        } else {
            dst[i] = b[i] + c[i];
        }
    }
}

/* Test function for LE_EXPR (less-than-or-equal) */
void test_le_loop(short *dst, const short *a, const short *b, const short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
        if (a[i] <= b[i]) {
            dst[i] = a[i] - c[i];
        } else {
            dst[i] = b[i] * c[i];
        }
    }
}

/* Additional test with mixed comparisons in same loop */
void test_mixed_comparisons(int *dst1, int *dst2, int *dst3, int *dst4,
                           const int *a, const int *b, const int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Multiple comparisons to ensure all cases are exercised */
        dst1[i] = (a[i] > b[i]) ? (a[i] + c[i]) : b[i];
        dst2[i] = (a[i] >= b[i]) ? (a[i] - c[i]) : b[i];
        dst3[i] = (a[i] < b[i]) ? (a[i] * c[i]) : b[i];
        dst4[i] = (a[i] <= b[i]) ? (a[i] / (c[i] + 1)) : b[i];
    }
}

/* Test with different loop lengths */
void test_various_lengths(int *dst, const int *a, const int *b, int n) {
    /* Multiple loops with different comparison operators */
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) dst[i] = 1;
    }
    
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) dst[i + n] = 2;
    }
    
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) dst[i + 2*n] = 3;
    }
    
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) dst[i + 3*n] = 4;
    }
}

int main(void) {
    /* Declare arrays with different sizes */
    int src1[SIZE], src2[SIZE], src3[SIZE];
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    
    short src1_short[SIZE2], src2_short[SIZE2], src3_short[SIZE2];
    short dst1_short[SIZE2], dst2_short[SIZE2];
    
    int dst_mixed[SIZE * 4];
    
    /* Initialize with semi-random data */
    init_arrays(src1, src2, src3, SIZE);
    
    /* Initialize short arrays */
    int state = global_seed;
    for (int i = 0; i < SIZE2; i++) {
        src1_short[i] = lcg_rand(&state) % 1000;
        src2_short[i] = lcg_rand(&state) % 1000;
        src3_short[i] = lcg_rand(&state) % 1000;
    }
    
    /* Test all comparison operators */
    test_gt_loop(dst1, src1, src2, src3, SIZE);
    test_lt_loop(dst2, src1, src2, src3, SIZE);
    test_ge_loop(dst1_short, src1_short, src2_short, src3_short, SIZE2);
    test_le_loop(dst2_short, src1_short, src2_short, src3_short, SIZE2);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst1, dst2, dst3, dst4, src1, src2, src3, SIZE);
    
    /* Test with various lengths */
    test_various_lengths(dst_mixed, src1, src2, SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst1_short[i] + dst2_short[i];
    }
    for (int i = 0; i < SIZE * 4; i++) {
        checksum += dst_mixed[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
