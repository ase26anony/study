/* Program to trigger vectorization of comparison operations for GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 12345;
volatile int global_mod = 100;

/* Initialize arrays with pseudo-random data */
void init_arrays(int *a, int *b, int *c, int n) {
    int seed = global_seed;
    for (int i = 0; i < n; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        a[i] = (seed % global_mod) - (global_mod / 2);
        b[i] = ((seed * 1664525 + 1013904223) % global_mod) - (global_mod / 2);
        c[i] = ((seed * 214013 + 2531011) % global_mod) - (global_mod / 2);
    }
}

void init_arrays_short(short *a, short *b, short *c, int n) {
    int seed = global_seed + 1;
    for (int i = 0; i < n; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        a[i] = (short)(seed % 1000) - 500;
        b[i] = (short)((seed * 1664525) % 1000) - 500;
        c[i] = (short)((seed * 214013) % 1000) - 500;
    }
}

/* Test functions for each comparison operator */

/* GT_EXPR case: > comparison */
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

/* GE_EXPR case: >= comparison */
void test_ge_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GE_EXPR in vectorized form */
        if (a[i] >= b[i]) {
            dst[i] = a[i] * 2 + c[i];
        } else {
            dst[i] = b[i] * 2 - c[i];
        }
    }
}

/* LT_EXPR case: < comparison */
void test_lt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LT_EXPR in vectorized form */
        if (a[i] < b[i]) {
            dst[i] = a[i] + b[i] + c[i];
        } else {
            dst[i] = a[i] - b[i] - c[i];
        }
    }
}

/* LE_EXPR case: <= comparison */
void test_le_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LE_EXPR in vectorized form */
        if (a[i] <= b[i]) {
            dst[i] = (a[i] << 1) + c[i];
        } else {
            dst[i] = (b[i] << 1) - c[i];
        }
    }
}

/* Mixed data types: short comparisons */

void test_gt_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* GT_EXPR with short type */
        if (a[i] > b[i]) {
            dst[i] = (short)(a[i] + c[i]);
        } else {
            dst[i] = (short)(b[i] - c[i]);
        }
    }
}

void test_ge_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* GE_EXPR with short type */
        if (a[i] >= b[i]) {
            dst[i] = (short)(a[i] * 3 + c[i]);
        } else {
            dst[i] = (short)(b[i] * 3 - c[i]);
        }
    }
}

void test_lt_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* LT_EXPR with short type */
        if (a[i] < b[i]) {
            dst[i] = (short)(a[i] + c[i] * 2);
        } else {
            dst[i] = (short)(b[i] - c[i] * 2);
        }
    }
}

void test_le_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* LE_EXPR with short type */
        if (a[i] <= b[i]) {
            dst[i] = (short)(a[i] * 4 + c[i]);
        } else {
            dst[i] = (short)(b[i] * 4 - c[i]);
        }
    }
}

/* Additional test with different loop lengths and patterns */

void test_mixed_comparisons(int *dst1, int *dst2, int *a, int *b, int *c, int n) {
    /* Mix of comparisons in one loop - may trigger different optimizations */
    for (int i = 0; i < n; i++) {
        /* First comparison: GT_EXPR */
        int cond1 = (a[i] > b[i]) ? a[i] + c[i] : b[i];
        
        /* Second comparison: GE_EXPR */
        int cond2 = (a[i] >= c[i]) ? b[i] + i : c[i];
        
        /* Third comparison: LT_EXPR */
        int cond3 = (b[i] < c[i]) ? a[i] - i : b[i];
        
        /* Fourth comparison: LE_EXPR */
        int cond4 = (c[i] <= a[i]) ? b[i] * i : c[i];
        
        dst1[i] = cond1 + cond2;
        dst2[i] = cond3 + cond4;
    }
}

int main() {
    /* Allocate arrays with different sizes */
    int a_int[SIZE], b_int[SIZE], c_int[SIZE];
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    int dst5[SIZE2], dst6[SIZE2];
    
    short a_short[SIZE], b_short[SIZE], c_short[SIZE];
    short dst_short1[SIZE], dst_short2[SIZE], dst_short3[SIZE], dst_short4[SIZE];
    
    /* Initialize with volatile-dependent values */
    init_arrays(a_int, b_int, c_int, SIZE);
    init_arrays(dst5, dst6, a_int, SIZE2); /* Reuse for initialization */
    init_arrays_short(a_short, b_short, c_short, SIZE);
    
    /* Test all four comparison operators with int type */
    test_gt_loop(dst1, a_int, b_int, c_int, SIZE);
    test_ge_loop(dst2, a_int, b_int, c_int, SIZE);
    test_lt_loop(dst3, a_int, b_int, c_int, SIZE);
    test_le_loop(dst4, a_int, b_int, c_int, SIZE);
    
    /* Test all four comparison operators with short type */
    test_gt_loop_short(dst_short1, a_short, b_short, c_short, SIZE);
    test_ge_loop_short(dst_short2, a_short, b_short, c_short, SIZE);
    test_lt_loop_short(dst_short3, a_short, b_short, c_short, SIZE);
    test_le_loop_short(dst_short4, a_short, b_short, c_short, SIZE);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst5, dst6, a_int, b_int, c_int, SIZE2);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i];
        checksum += dst_short1[i] + dst_short2[i] + dst_short3[i] + dst_short4[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst5[i] + dst6[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
