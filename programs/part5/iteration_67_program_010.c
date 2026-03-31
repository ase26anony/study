/* Vectorizable comparison loops targeting tree-vect-stmts.cc lines 12216-12233 */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define SIZE_SHORT 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 42;
volatile int global_mod = 100;

/* Simple LCG for semi-random data */
static inline int lcg_next(int *state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

/* Initialize arrays with semi-random data */
void init_arrays(int *a, int *b, int *c, int size) {
    int state = global_seed;
    for (int i = 0; i < size; i++) {
        a[i] = lcg_next(&state) % global_mod;
        b[i] = lcg_next(&state) % global_mod;
        c[i] = (lcg_next(&state) % 50) + 1; /* Non-zero for division safety */
    }
}

void init_arrays_short(short *a, short *b, short *c, int size) {
    int state = global_seed + 1;
    for (int i = 0; i < size; i++) {
        a[i] = (short)(lcg_next(&state) % 100);
        b[i] = (short)(lcg_next(&state) % 100);
        c[i] = (short)((lcg_next(&state) % 40) + 1);
    }
}

/* Test functions for each comparison operator */

/* GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR */
void test_gt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Generate mask from comparison, use for conditional update */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR */
void test_ge_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Different computation to avoid CSE */
        if (a[i] >= b[i]) {
            dst[i] = (a[i] * 2) + c[i];
        } else {
            dst[i] = (b[i] * 3) - c[i];
        }
    }
}

/* LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
void test_lt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Less-than comparison */
        if (a[i] < b[i]) {
            dst[i] = a[i] + (c[i] * 2);
        } else {
            dst[i] = b[i] - (c[i] / 2);
        }
    }
}

/* LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
void test_le_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Less-than-or-equal comparison */
        if (a[i] <= b[i]) {
            dst[i] = (a[i] << 1) | c[i];
        } else {
            dst[i] = (b[i] >> 1) & c[i];
        }
    }
}

/* Same operators with short type for different vector modes */
void test_gt_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            dst[i] = (short)(a[i] + c[i]);
        } else {
            dst[i] = (short)(b[i] - c[i]);
        }
    }
}

void test_ge_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            dst[i] = (short)(a[i] * c[i]);
        } else {
            dst[i] = (short)(b[i] / c[i]);
        }
    }
}

void test_lt_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            dst[i] = (short)(a[i] | c[i]);
        } else {
            dst[i] = (short)(b[i] & c[i]);
        }
    }
}

void test_le_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            dst[i] = (short)(a[i] ^ c[i]);
        } else {
            dst[i] = (short)(b[i] + c[i]);
        }
    }
}

/* Additional test with mixed comparisons in same loop */
void test_mixed_comparisons(int *dst1, int *dst2, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Multiple comparisons in same loop body */
        if (a[i] > b[i]) {
            dst1[i] = a[i] + 1;
        } else {
            dst1[i] = b[i] - 1;
        }
        
        if (a[i] <= c[i]) {
            dst2[i] = a[i] * 2;
        } else {
            dst2[i] = c[i] * 3;
        }
    }
}

int main() {
    /* Allocate aligned arrays for better vectorization */
    int __attribute__((aligned(32))) src1[SIZE], src2[SIZE], src3[SIZE];
    int __attribute__((aligned(32))) dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    int __attribute__((aligned(32))) dst5[SIZE], dst6[SIZE];
    
    short __attribute__((aligned(16))) src1_short[SIZE_SHORT], src2_short[SIZE_SHORT], src3_short[SIZE_SHORT];
    short __attribute__((aligned(16))) dst1_short[SIZE_SHORT], dst2_short[SIZE_SHORT];
    short __attribute__((aligned(16))) dst3_short[SIZE_SHORT], dst4_short[SIZE_SHORT];
    
    /* Initialize with semi-random data */
    init_arrays(src1, src2, src3, SIZE);
    init_arrays_short(src1_short, src2_short, src3_short, SIZE_SHORT);
    
    /* Test all comparison operators with int type */
    test_gt_loop(dst1, src1, src2, src3, SIZE);
    test_ge_loop(dst2, src1, src2, src3, SIZE);
    test_lt_loop(dst3, src1, src2, src3, SIZE);
    test_le_loop(dst4, src1, src2, src3, SIZE);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst5, dst6, src1, src2, src3, SIZE);
    
    /* Test all comparison operators with short type */
    test_gt_loop_short(dst1_short, src1_short, src2_short, src3_short, SIZE_SHORT);
    test_ge_loop_short(dst2_short, src1_short, src2_short, src3_short, SIZE_SHORT);
    test_lt_loop_short(dst3_short, src1_short, src2_short, src3_short, SIZE_SHORT);
    test_le_loop_short(dst4_short, src1_short, src2_short, src3_short, SIZE_SHORT);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i] + dst5[i] + dst6[i];
    }
    for (int i = 0; i < SIZE_SHORT; i++) {
        checksum += dst1_short[i] + dst2_short[i] + dst3_short[i] + dst4_short[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
