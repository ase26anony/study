/* Program to trigger vectorization of comparison operations
   targeting uncovered lines in tree-vect-stmts.cc (12216-12233) */

#include <stdio.h>
#include <stdint.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 12345;
volatile int use_volatile = 1;

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

/* Test functions for each comparison operator */

/* GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR */
void test_gt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Generate mask from comparison, use for conditional update */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];  /* Use comparison result */
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR */
void test_ge_loop(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Different integer width (short) */
        if (a[i] >= b[i]) {
            dst[i] = (short)(a[i] * c[i]);
        } else {
            dst[i] = (short)(b[i] / (c[i] + 1));
        }
    }
}

/* LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
void test_lt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Less-than comparison should trigger swap */
        if (a[i] < b[i]) {
            dst[i] = b[i] + a[i] * c[i];
        } else {
            dst[i] = a[i] - b[i] * c[i];
        }
    }
}

/* LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
void test_le_loop(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Less-than-or-equal with short type */
        if (a[i] <= b[i]) {
            dst[i] = (short)(a[i] | c[i]);
        } else {
            dst[i] = (short)(b[i] & c[i]);
        }
    }
}

/* Additional test with mixed comparisons in same loop */
void test_mixed_comparisons(int *dst1, int *dst2, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Multiple comparisons in same loop */
        dst1[i] = (a[i] > b[i]) ? (a[i] + c[i]) : b[i];
        dst2[i] = (a[i] <= c[i]) ? (b[i] - a[i]) : c[i];
    }
}

/* Test with volatile inputs to prevent optimization */
void test_volatile_comparisons(int *dst, volatile int *a, volatile int *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            dst[i] = 1;
        } else if (a[i] < b[i]) {
            dst[i] = -1;
        } else {
            dst[i] = 0;
        }
    }
}

int main(void) {
    /* Declare arrays with different sizes */
    int src1[SIZE], src2[SIZE], src3[SIZE];
    int dst_gt[SIZE], dst_lt[SIZE];
    short src1_short[SIZE2], src2_short[SIZE2], src3_short[SIZE2];
    short dst_ge[SIZE2], dst_le[SIZE2];
    int dst_mixed1[SIZE], dst_mixed2[SIZE];
    int dst_volatile[SIZE];
    
    volatile int volatile_src1[SIZE];
    volatile int volatile_src2[SIZE];
    
    /* Initialize with semi-random data */
    init_arrays(src1, src2, src3, SIZE);
    
    /* Initialize short arrays */
    int state = global_seed;
    for (int i = 0; i < SIZE2; i++) {
        src1_short[i] = (short)(lcg_rand(&state) % 1000);
        src2_short[i] = (short)(lcg_rand(&state) % 1000);
        src3_short[i] = (short)(lcg_rand(&state) % 1000);
    }
    
    /* Initialize volatile arrays */
    state = global_seed + 1;
    for (int i = 0; i < SIZE; i++) {
        volatile_src1[i] = lcg_rand(&state) % 1000;
        volatile_src2[i] = lcg_rand(&state) % 1000;
    }
    
    /* Execute all test functions */
    test_gt_loop(dst_gt, src1, src2, src3, SIZE);
    test_ge_loop(dst_ge, src1_short, src2_short, src3_short, SIZE2);
    test_lt_loop(dst_lt, src1, src2, src3, SIZE);
    test_le_loop(dst_le, src1_short, src2_short, src3_short, SIZE2);
    test_mixed_comparisons(dst_mixed1, dst_mixed2, src1, src2, src3, SIZE);
    
    if (use_volatile) {
        test_volatile_comparisons(dst_volatile, volatile_src1, volatile_src2, SIZE);
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst_gt[i] + dst_lt[i] + dst_mixed1[i] + dst_mixed2[i];
        if (i < SIZE) checksum += dst_volatile[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst_ge[i] + dst_le[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
