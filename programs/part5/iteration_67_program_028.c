/* Program to trigger vectorization of comparison operations
   targeting uncovered lines in tree-vect-stmts.cc (12216-12233) */

#include <stdio.h>
#include <stdint.h>

#define SIZE 256
#define SIZE2 128

/* Global arrays to prevent constant propagation */
volatile int global_seed = 42;

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

/* GT_EXPR case (lines 12216-12218) */
void test_gt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Generate mask from > comparison, use for conditional assignment */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i];
        }
    }
}

/* GE_EXPR case (lines 12219-12221) */
void test_ge_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Generate mask from >= comparison */
        if (a[i] >= b[i]) {
            dst[i] = a[i] - c[i];
        } else {
            dst[i] = c[i];
        }
    }
}

/* LT_EXPR case (lines 12222-12226) */
void test_lt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Generate mask from < comparison */
        if (a[i] < b[i]) {
            dst[i] = b[i] + c[i];
        } else {
            dst[i] = a[i];
        }
    }
}

/* LE_EXPR case (lines 12227-12233) */
void test_le_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Generate mask from <= comparison */
        if (a[i] <= b[i]) {
            dst[i] = a[i] * 2;
        } else {
            dst[i] = b[i] * 3;
        }
    }
}

/* Mixed data types: short integer loops */

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
            dst[i] = a[i] * 2;
        } else {
            dst[i] = b[i] / 2;
        }
    }
}

/* Additional test with different loop lengths and patterns */

/* Mixed comparisons in same function */
void test_mixed_comparisons(int *dst1, int *dst2, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Use both > and < in same loop */
        if (a[i] > b[i]) {
            dst1[i] = a[i] + b[i];
        } else {
            dst1[i] = c[i];
        }
        
        if (a[i] < c[i]) {
            dst2[i] = b[i] - a[i];
        } else {
            dst2[i] = c[i] + b[i];
        }
    }
}

/* Compute checksum to prevent dead code elimination */
unsigned long long compute_checksum(int *arr, int size) {
    unsigned long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

unsigned long long compute_checksum_short(short *arr, int size) {
    unsigned long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

int main() {
    /* Declare arrays with different sizes */
    int src1[SIZE], src2[SIZE], src3[SIZE];
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    int dst5[SIZE2], dst6[SIZE2];
    
    short src1_short[SIZE], src2_short[SIZE], src3_short[SIZE];
    short dst1_short[SIZE], dst2_short[SIZE];
    
    /* Initialize arrays */
    init_arrays(src1, src2, src3, SIZE);
    init_arrays(dst5, dst6, src1, SIZE2); /* Reuse for initialization */
    
    /* Initialize short arrays */
    int state = global_seed;
    for (int i = 0; i < SIZE; i++) {
        src1_short[i] = lcg(&state) % 1000;
        src2_short[i] = lcg(&state) % 1000;
        src3_short[i] = lcg(&state) % 1000;
    }
    
    /* Test all comparison operators with int */
    test_gt_loop(dst1, src1, src2, src3, SIZE);
    test_ge_loop(dst2, src1, src2, src3, SIZE);
    test_lt_loop(dst3, src1, src2, src3, SIZE);
    test_le_loop(dst4, src1, src2, src3, SIZE);
    
    /* Test with short */
    test_gt_loop_short(dst1_short, src1_short, src2_short, src3_short, SIZE);
    test_ge_loop_short(dst2_short, src1_short, src2_short, src3_short, SIZE);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst5, dst6, src1, src2, src3, SIZE2);
    
    /* Compute checksums to prevent optimization */
    unsigned long long checksum = 0;
    checksum += compute_checksum(dst1, SIZE);
    checksum += compute_checksum(dst2, SIZE);
    checksum += compute_checksum(dst3, SIZE);
    checksum += compute_checksum(dst4, SIZE);
    checksum += compute_checksum_short(dst1_short, SIZE);
    checksum += compute_checksum_short(dst2_short, SIZE);
    checksum += compute_checksum(dst5, SIZE2);
    checksum += compute_checksum(dst6, SIZE2);
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
