/* Program to trigger vectorization of comparison operations
   targeting uncovered lines in tree-vect-stmts.cc (12216-12233) */

#include <stdio.h>
#include <stdint.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 42;
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

/* GT_EXPR case (lines 12216-12218) */
void test_gt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Generate mask from GT comparison, use for conditional assignment */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];  /* Use comparison result */
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* GE_EXPR case (lines 12219-12221) */
void test_ge_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Generate mask from GE comparison */
        if (a[i] >= b[i]) {
            dst[i] = a[i] * 2 + c[i];
        } else {
            dst[i] = b[i] * 2 - c[i];
        }
    }
}

/* LT_EXPR case (lines 12222-12226) */
void test_lt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Generate mask from LT comparison */
        if (a[i] < b[i]) {
            dst[i] = a[i] + b[i] + c[i];
        } else {
            dst[i] = a[i] - b[i] - c[i];
        }
    }
}

/* LE_EXPR case (lines 12227-12233) */
void test_le_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Generate mask from LE comparison */
        if (a[i] <= b[i]) {
            dst[i] = a[i] * 3 + c[i];
        } else {
            dst[i] = b[i] * 3 - c[i];
        }
    }
}

/* Same tests with short type for different vector modes */
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

void test_lt_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            dst[i] = a[i] + b[i] + c[i];
        } else {
            dst[i] = a[i] - b[i] - c[i];
        }
    }
}

void test_le_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            dst[i] = a[i] * 3 + c[i];
        } else {
            dst[i] = b[i] * 3 - c[i];
        }
    }
}

/* Additional test with mixed comparisons in same loop */
void test_mixed_comparisons(int *dst1, int *dst2, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Multiple comparisons in same loop */
        if (a[i] > b[i]) {
            dst1[i] = a[i] + c[i];
        } else {
            dst1[i] = b[i];
        }
        
        if (a[i] <= c[i]) {
            dst2[i] = b[i] + a[i];
        } else {
            dst2[i] = c[i];
        }
    }
}

int main() {
    /* Declare arrays with different sizes */
    int src1_int[SIZE], src2_int[SIZE], src3_int[SIZE];
    int dst1_int[SIZE], dst2_int[SIZE], dst3_int[SIZE], dst4_int[SIZE];
    int dst5_int[SIZE], dst6_int[SIZE];
    
    short src1_short[SIZE2], src2_short[SIZE2], src3_short[SIZE2];
    short dst1_short[SIZE2], dst2_short[SIZE2], dst3_short[SIZE2], dst4_short[SIZE2];
    
    /* Initialize with semi-random data */
    init_arrays(src1_int, src2_int, src3_int, SIZE);
    init_arrays_short(src1_short, src2_short, src3_short, SIZE2);
    
    /* Force compiler to consider all loops by using volatile */
    int n = use_volatile ? SIZE : 0;
    int n2 = use_volatile ? SIZE2 : 0;
    
    /* Test all comparison operators with int type */
    test_gt_loop(dst1_int, src1_int, src2_int, src3_int, n);
    test_ge_loop(dst2_int, src1_int, src2_int, src3_int, n);
    test_lt_loop(dst3_int, src1_int, src2_int, src3_int, n);
    test_le_loop(dst4_int, src1_int, src2_int, src3_int, n);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst5_int, dst6_int, src1_int, src2_int, src3_int, n);
    
    /* Test all comparison operators with short type */
    test_gt_loop_short(dst1_short, src1_short, src2_short, src3_short, n2);
    test_ge_loop_short(dst2_short, src1_short, src2_short, src3_short, n2);
    test_lt_loop_short(dst3_short, src1_short, src2_short, src3_short, n2);
    test_le_loop_short(dst4_short, src1_short, src2_short, src3_short, n2);
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1_int[i] + dst2_int[i] + dst3_int[i] + dst4_int[i];
        checksum += dst5_int[i] + dst6_int[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst1_short[i] + dst2_short[i] + dst3_short[i] + dst4_short[i];
    }
    
    printf("Checksum: %lu\n", (unsigned long)checksum);
    return 0;
}
