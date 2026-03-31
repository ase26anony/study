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

void init_arrays_short(short *a, short *b, short *c, int size) {
    int state = global_seed + 1;
    for (int i = 0; i < size; i++) {
        a[i] = lcg_rand(&state) % 1000;
        b[i] = lcg_rand(&state) % 1000;
        c[i] = lcg_rand(&state) % 1000;
    }
}

/* Test functions for each comparison operator */

/* GT_EXPR case - lines 12216-12219 */
void test_gt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Generate mask from comparison, use for conditional assignment */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* GE_EXPR case - lines 12220-12223 */
void test_ge_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Greater-than-or-equal comparison */
        if (a[i] >= b[i]) {
            dst[i] = a[i] * 2 + c[i];
        } else {
            dst[i] = b[i] * 3 - c[i];
        }
    }
}

/* LT_EXPR case - lines 12224-12228 */
void test_lt_loop(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Less-than comparison with different data type */
        if (a[i] < b[i]) {
            dst[i] = a[i] + c[i] * 2;
        } else {
            dst[i] = b[i] - c[i] / 2;
        }
    }
}

/* LE_EXPR case - lines 12229-12233 */
void test_le_loop(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Less-than-or-equal comparison */
        if (a[i] <= b[i]) {
            dst[i] = a[i] * 3 + c[i];
        } else {
            dst[i] = b[i] * 2 - c[i];
        }
    }
}

/* Additional test with mixed comparisons in same loop */
void test_mixed_comparisons(int *dst1, int *dst2, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Multiple comparisons to trigger different paths */
        if (a[i] > b[i]) {
            dst1[i] = a[i] + 1;
        } else {
            dst1[i] = b[i] - 1;
        }
        
        if (a[i] < c[i]) {
            dst2[i] = a[i] * 2;
        } else {
            dst2[i] = c[i] * 3;
        }
    }
}

/* Test with volatile inputs to prevent optimization */
void test_volatile_comparisons(int *dst, volatile int *a, volatile int *b, int n) {
    for (int i = 0; i < n; i++) {
        /* Force compiler to generate actual comparison */
        if (a[i] >= b[i]) {
            dst[i] = 1;
        } else {
            dst[i] = 0;
        }
    }
}

int main(void) {
    /* Declare arrays with different sizes */
    int src1_int[SIZE], src2_int[SIZE], src3_int[SIZE];
    int dst1_int[SIZE], dst2_int[SIZE], dst3_int[SIZE], dst4_int[SIZE];
    
    short src1_short[SIZE2], src2_short[SIZE2], src3_short[SIZE2];
    short dst1_short[SIZE2], dst2_short[SIZE2];
    
    int dst_mixed1[SIZE], dst_mixed2[SIZE];
    
    /* Initialize with semi-random data */
    init_arrays(src1_int, src2_int, src3_int, SIZE);
    init_arrays_short(src1_short, src2_short, src3_short, SIZE2);
    
    /* Test each comparison operator */
    test_gt_loop(dst1_int, src1_int, src2_int, src3_int, SIZE);
    test_ge_loop(dst2_int, src1_int, src2_int, src3_int, SIZE);
    test_lt_loop(dst1_short, src1_short, src2_short, src3_short, SIZE2);
    test_le_loop(dst2_short, src1_short, src2_short, src3_short, SIZE2);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst_mixed1, dst_mixed2, src1_int, src2_int, src3_int, SIZE);
    
    /* Test with volatile if enabled */
    if (use_volatile) {
        volatile int volatile_a[SIZE], volatile_b[SIZE];
        for (int i = 0; i < SIZE; i++) {
            volatile_a[i] = src1_int[i];
            volatile_b[i] = src2_int[i];
        }
        test_volatile_comparisons(dst3_int, volatile_a, volatile_b, SIZE);
    }
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1_int[i] + dst2_int[i] + dst_mixed1[i] + dst_mixed2[i];
        if (i < SIZE2) {
            checksum += dst1_short[i] + dst2_short[i];
        }
    }
    
    printf("Checksum: %lu\n", (unsigned long)checksum);
    return 0;
}
