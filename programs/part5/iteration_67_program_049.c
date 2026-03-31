/* Program to trigger vectorization of comparison operations for GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR */
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
void init_arrays(int *a, int *b, int *c, int size) {
    int state = global_seed;
    for (int i = 0; i < size; i++) {
        a[i] = lcg_rand(&state) % 1000;
        b[i] = lcg_rand(&state) % 1000;
        c[i] = lcg_rand(&state) % 1000;
    }
}

/* Test functions for each comparison operator */

/* GT_EXPR - greater than */
void test_gt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GT_EXPR in vectorized form */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* GE_EXPR - greater than or equal */
void test_ge_loop(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GE_EXPR in vectorized form */
        if (a[i] >= b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* LT_EXPR - less than */
void test_lt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LT_EXPR in vectorized form */
        if (a[i] < b[i]) {
            dst[i] = a[i] * c[i];
        } else {
            dst[i] = b[i] + c[i];
        }
    }
}

/* LE_EXPR - less than or equal */
void test_le_loop(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LE_EXPR in vectorized form */
        if (a[i] <= b[i]) {
            dst[i] = a[i] - c[i];
        } else {
            dst[i] = b[i] * c[i];
        }
    }
}

/* Mixed data types test with GT_EXPR */
void test_mixed_gt(int *dst, short *a, int *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Mixed types to trigger different mode conditions */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - a[i];
        }
    }
}

/* Another variant with GE_EXPR and different stride */
void test_ge_loop_stride2(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i += 2) {
        if (a[i] >= b[i]) {
            dst[i] = a[i] + c[i];
        }
        if (i + 1 < n && a[i + 1] >= b[i + 1]) {
            dst[i + 1] = a[i + 1] + c[i + 1];
        }
    }
}

/* Main function with checksum computation */
int main() {
    /* Declare arrays with different sizes */
    int src1_int[SIZE], src2_int[SIZE], src3_int[SIZE];
    short src1_short[SIZE2], src2_short[SIZE2], src3_short[SIZE2];
    
    int dst1[SIZE], dst2[SIZE];
    short dst3[SIZE2], dst4[SIZE2];
    
    /* Initialize with semi-random data */
    init_arrays(src1_int, src2_int, src3_int, SIZE);
    
    /* Initialize short arrays */
    int state = global_seed + 1;
    for (int i = 0; i < SIZE2; i++) {
        src1_short[i] = lcg_rand(&state) % 1000;
        src2_short[i] = lcg_rand(&state) % 1000;
        src3_short[i] = lcg_rand(&state) % 1000;
    }
    
    /* Call test functions with different comparison operators */
    
    /* GT_EXPR test with int arrays */
    test_gt_loop(dst1, src1_int, src2_int, src3_int, SIZE);
    
    /* GE_EXPR test with short arrays */
    test_ge_loop(dst3, src1_short, src2_short, src3_short, SIZE2);
    
    /* LT_EXPR test with int arrays */
    test_lt_loop(dst2, src1_int, src2_int, src3_int, SIZE);
    
    /* LE_EXPR test with short arrays */
    test_le_loop(dst4, src1_short, src2_short, src3_short, SIZE2);
    
    /* Additional tests with mixed types */
    int dst5[SIZE2];
    test_mixed_gt(dst5, src1_short, src2_int, src3_short, SIZE2);
    
    int dst6[SIZE];
    test_ge_loop_stride2(dst6, src1_int, src2_int, src3_int, SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i];
        if (i < SIZE) checksum += dst6[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst3[i] + dst4[i] + dst5[i];
    }
    
    /* Use volatile to prevent optimization */
    if (use_volatile) {
        printf("Checksum: %llu\n", checksum);
    }
    
    return 0;
}
