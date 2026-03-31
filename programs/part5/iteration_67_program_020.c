/* Program to trigger vectorization of comparison operations
   targeting uncovered lines in tree-vect-stmts.cc (12216-12233) */

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

/* GT_EXPR case: > comparison */
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

/* GE_EXPR case: >= comparison */
void test_ge_loop(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GE_EXPR */
        if (a[i] >= b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* LT_EXPR case: < comparison */
void test_lt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LT_EXPR */
        if (a[i] < b[i]) {
            dst[i] = a[i] * c[i];
        } else {
            dst[i] = b[i] / (c[i] + 1);
        }
    }
}

/* LE_EXPR case: <= comparison */
void test_le_loop(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LE_EXPR */
        if (a[i] <= b[i]) {
            dst[i] = a[i] | c[i];
        } else {
            dst[i] = b[i] & c[i];
        }
    }
}

/* Additional test with mixed comparisons in same loop */
void test_mixed_comparisons(int *dst1, int *dst2, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Mix of different comparisons */
        if (a[i] > b[i]) {          /* GT_EXPR */
            dst1[i] = a[i] + 1;
        } else if (a[i] <= c[i]) {  /* LE_EXPR */
            dst1[i] = b[i] - 1;
        } else {
            dst1[i] = 0;
        }
        
        if (a[i] >= b[i]) {         /* GE_EXPR */
            dst2[i] = c[i] * 2;
        } else if (a[i] < c[i]) {   /* LT_EXPR */
            dst2[i] = b[i] / 2;
        } else {
            dst2[i] = 1;
        }
    }
}

/* Test with different loop lengths */
void test_various_lengths(int *dst, int *a, int *b, int *c) {
    /* Multiple loops with different sizes */
    for (int i = 0; i < 64; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR */
            dst[i] = a[i] + c[i];
        }
    }
    
    for (int i = 0; i < 128; i++) {
        if (a[i] >= b[i]) {  /* GE_EXPR */
            dst[i + 64] = b[i] - c[i];
        }
    }
    
    for (int i = 0; i < 192; i++) {
        if (a[i] < b[i]) {  /* LT_EXPR */
            dst[i + 192] = a[i] * c[i];
        }
    }
    
    for (int i = 0; i < 256; i++) {
        if (a[i] <= b[i]) {  /* LE_EXPR */
            dst[i + 384] = b[i] | c[i];
        }
    }
}

int main(void) {
    /* Declare arrays with different types and sizes */
    int src1_int[SIZE], src2_int[SIZE], src3_int[SIZE];
    short src1_short[SIZE2], src2_short[SIZE2], src3_short[SIZE2];
    
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    short dst_short1[SIZE2], dst_short2[SIZE2];
    int dst_mixed1[SIZE], dst_mixed2[SIZE];
    int dst_various[640];  /* For variable length tests */
    
    /* Initialize with semi-random data */
    init_arrays(src1_int, src2_int, src3_int, SIZE);
    
    /* Initialize short arrays */
    int state = global_seed;
    for (int i = 0; i < SIZE2; i++) {
        src1_short[i] = lcg_rand(&state) % 1000;
        src2_short[i] = lcg_rand(&state) % 1000;
        src3_short[i] = lcg_rand(&state) % 1000;
    }
    
    /* Test each comparison operator with different data types */
    test_gt_loop(dst1, src1_int, src2_int, src3_int, SIZE);
    test_ge_loop(dst_short1, src1_short, src2_short, src3_short, SIZE2);
    test_lt_loop(dst2, src1_int, src2_int, src3_int, SIZE);
    test_le_loop(dst_short2, src1_short, src2_short, src3_short, SIZE2);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst3, dst4, src1_int, src2_int, src3_int, SIZE);
    
    /* Test with various loop lengths */
    test_various_lengths(dst_various, src1_int, src2_int, src3_int);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst_short1[i] + dst_short2[i];
    }
    for (int i = 0; i < 640; i++) {
        checksum += dst_various[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
