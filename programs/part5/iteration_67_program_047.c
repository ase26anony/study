#include <stdio.h>
#include <stdlib.h>

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
void init_arrays(int *a, int *b, int *c, int n) {
    int state = global_seed;
    for (int i = 0; i < n; i++) {
        a[i] = lcg_rand(&state) % 1000;
        b[i] = lcg_rand(&state) % 1000;
        c[i] = lcg_rand(&state) % 1000;
    }
}

/* Test functions for each comparison operator */

/* GT_EXPR - greater than */
void test_gt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GT_EXPR */
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
        /* This should generate GE_EXPR */
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
        /* This should generate LT_EXPR */
        if (a[i] < b[i]) {
            dst[i] = a[i] + b[i] + c[i];
        } else {
            dst[i] = a[i] - b[i] - c[i];
        }
    }
}

/* LE_EXPR - less than or equal */
void test_le_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LE_EXPR */
        if (a[i] <= b[i]) {
            dst[i] = (a[i] << 1) + c[i];
        } else {
            dst[i] = (b[i] << 1) - c[i];
        }
    }
}

/* Mixed data type tests - using short to trigger different vector modes */

/* GT_EXPR with short */
void test_gt_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* GE_EXPR with short */
void test_ge_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            dst[i] = a[i] * 3 + c[i];
        } else {
            dst[i] = b[i] * 3 - c[i];
        }
    }
}

/* LT_EXPR with short */
void test_lt_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            dst[i] = a[i] + c[i] * 2;
        } else {
            dst[i] = b[i] - c[i] * 2;
        }
    }
}

/* LE_EXPR with short */
void test_le_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            dst[i] = a[i] + b[i] + c[i];
        } else {
            dst[i] = a[i] - b[i] - c[i];
        }
    }
}

/* Additional tests with different loop lengths */

/* GT_EXPR with different length */
void test_gt_loop_mixed(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i += 2) {
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i] * 3;
        } else {
            dst[i] = b[i] - c[i] * 3;
        }
    }
}

/* GE_EXPR with different stride */
void test_ge_loop_stride(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n - 1; i++) {
        if (a[i] >= b[i + 1]) {
            dst[i] = a[i] + b[i + 1];
        } else {
            dst[i] = a[i] - b[i + 1];
        }
    }
}

int main() {
    /* Declare arrays */
    int src1_int[SIZE], src2_int[SIZE], src3_int[SIZE];
    int dst1_int[SIZE], dst2_int[SIZE], dst3_int[SIZE], dst4_int[SIZE];
    
    short src1_short[SIZE2], src2_short[SIZE2], src3_short[SIZE2];
    short dst1_short[SIZE2], dst2_short[SIZE2], dst3_short[SIZE2], dst4_short[SIZE2];
    
    int src1_mixed[SIZE], src2_mixed[SIZE], src3_mixed[SIZE];
    int dst_mixed[SIZE];
    
    /* Initialize integer arrays */
    init_arrays(src1_int, src2_int, src3_int, SIZE);
    
    /* Initialize short arrays */
    int state = global_seed;
    for (int i = 0; i < SIZE2; i++) {
        src1_short[i] = lcg_rand(&state) % 1000;
        src2_short[i] = lcg_rand(&state) % 1000;
        src3_short[i] = lcg_rand(&state) % 1000;
    }
    
    /* Initialize mixed arrays */
    init_arrays(src1_mixed, src2_mixed, src3_mixed, SIZE);
    
    /* Test all comparison operators with int */
    test_gt_loop(dst1_int, src1_int, src2_int, src3_int, SIZE);
    test_ge_loop(dst2_int, src1_int, src2_int, src3_int, SIZE);
    test_lt_loop(dst3_int, src1_int, src2_int, src3_int, SIZE);
    test_le_loop(dst4_int, src1_int, src2_int, src3_int, SIZE);
    
    /* Test all comparison operators with short */
    test_gt_loop_short(dst1_short, src1_short, src2_short, src3_short, SIZE2);
    test_ge_loop_short(dst2_short, src1_short, src2_short, src3_short, SIZE2);
    test_lt_loop_short(dst3_short, src1_short, src2_short, src3_short, SIZE2);
    test_le_loop_short(dst4_short, src1_short, src2_short, src3_short, SIZE2);
    
    /* Test with different loop patterns */
    test_gt_loop_mixed(dst_mixed, src1_mixed, src2_mixed, src3_mixed, SIZE);
    test_ge_loop_stride(dst_mixed, src1_mixed, src2_mixed, src3_mixed, SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1_int[i] + dst2_int[i] + dst3_int[i] + dst4_int[i];
        if (i < SIZE) checksum += dst_mixed[i];
    }
    
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst1_short[i] + dst2_short[i] + dst3_short[i] + dst4_short[i];
    }
    
    /* Use volatile to prevent optimization */
    if (use_volatile) {
        printf("Checksum: %lld\n", checksum);
    }
    
    return 0;
}
