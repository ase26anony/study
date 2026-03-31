/* test_vector_comparisons.c
 * Designed to trigger GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR transformations
 * in tree-vect-stmts.cc lines 12216-12233
 */

#include <stdio.h>
#include <stdint.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 42;
volatile int use_volatile = 1;

/* Simple LCG for semi-random data */
static inline int lcg_next(int *state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

/* Initialize arrays with semi-random data */
void init_arrays(int *a, int *b, int *c, int size) {
    int state = global_seed;
    for (int i = 0; i < size; i++) {
        a[i] = lcg_next(&state) % 1000;
        b[i] = lcg_next(&state) % 1000;
        c[i] = lcg_next(&state) % 1000;
    }
}

void init_arrays_short(short *a, short *b, short *c, int size) {
    int state = global_seed + 1;
    for (int i = 0; i < size; i++) {
        a[i] = lcg_next(&state) % 1000;
        b[i] = lcg_next(&state) % 1000;
        c[i] = lcg_next(&state) % 1000;
    }
}

/* Test functions for each comparison operator */

/* GT_EXPR case - greater than */
void test_gt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* GE_EXPR case - greater than or equal */
void test_ge_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR */
        if (a[i] >= b[i]) {
            dst[i] = a[i] * 2 + c[i];
        } else {
            dst[i] = b[i] * 2 - c[i];
        }
    }
}

/* LT_EXPR case - less than */
void test_lt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
        if (a[i] < b[i]) {
            dst[i] = c[i] + a[i] * 3;
        } else {
            dst[i] = c[i] - b[i] * 3;
        }
    }
}

/* LE_EXPR case - less than or equal */
void test_le_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
        if (a[i] <= b[i]) {
            dst[i] = a[i] + b[i] + c[i];
        } else {
            dst[i] = a[i] - b[i] - c[i];
        }
    }
}

/* Mixed data types - using short to trigger different vector modes */
void test_gt_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

void test_ge_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            dst[i] = a[i] * 2 + c[i];
        } else {
            dst[i] = b[i] * 2 - c[i];
        }
    }
}

/* Different loop lengths to give vectorizer multiple chances */
void test_lt_loop_small(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            dst[i] = (a[i] << 2) + c[i];
        } else {
            dst[i] = (b[i] << 2) - c[i];
        }
    }
}

void test_le_loop_small(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            dst[i] = a[i] | b[i] | c[i];
        } else {
            dst[i] = a[i] & b[i] & c[i];
        }
    }
}

/* Main function with checksum to prevent dead code elimination */
int main() {
    /* Declare arrays with different sizes */
    int src1_int[SIZE], src2_int[SIZE], src3_int[SIZE];
    int dst1_int[SIZE], dst2_int[SIZE], dst3_int[SIZE], dst4_int[SIZE];
    
    short src1_short[SIZE2], src2_short[SIZE2], src3_short[SIZE2];
    short dst1_short[SIZE2], dst2_short[SIZE2];
    
    int src_small1[SIZE2], src_small2[SIZE2], src_small3[SIZE2];
    int dst_small1[SIZE2], dst_small2[SIZE2];
    
    /* Initialize all arrays */
    init_arrays(src1_int, src2_int, src3_int, SIZE);
    init_arrays_short(src1_short, src2_short, src3_short, SIZE2);
    init_arrays(src_small1, src_small2, src_small3, SIZE2);
    
    /* Execute all test loops */
    test_gt_loop(dst1_int, src1_int, src2_int, src3_int, SIZE);
    test_ge_loop(dst2_int, src1_int, src2_int, src3_int, SIZE);
    test_lt_loop(dst3_int, src1_int, src2_int, src3_int, SIZE);
    test_le_loop(dst4_int, src1_int, src2_int, src3_int, SIZE);
    
    test_gt_loop_short(dst1_short, src1_short, src2_short, src3_short, SIZE2);
    test_ge_loop_short(dst2_short, src1_short, src2_short, src3_short, SIZE2);
    
    test_lt_loop_small(dst_small1, src_small1, src_small2, src_small3, SIZE2);
    test_le_loop_small(dst_small2, src_small1, src_small2, src_small3, SIZE2);
    
    /* Compute checksum to prevent optimization */
    uint64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1_int[i] + dst2_int[i] + dst3_int[i] + dst4_int[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst1_short[i] + dst2_short[i];
        checksum += dst_small1[i] + dst_small2[i];
    }
    
    /* Use volatile to prevent constant folding of checksum */
    if (use_volatile) {
        printf("Checksum: %lu\n", (unsigned long)checksum);
    }
    
    return 0;
}
