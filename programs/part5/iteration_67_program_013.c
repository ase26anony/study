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

void init_arrays_short(short *a, short *b, short *c, int size) {
    int state = global_seed + 1;
    for (int i = 0; i < size; i++) {
        a[i] = lcg(&state) % 1000;
        b[i] = lcg(&state) % 1000;
        c[i] = lcg(&state) % 1000;
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

/* LT_EXPR - less than */
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

/* LE_EXPR - less than or equal */
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

/* Mixed data types - short comparisons */

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
            dst[i] = a[i] * 3 + c[i];
        } else {
            dst[i] = b[i] * 3 - c[i];
        }
    }
}

void test_lt_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            dst[i] = a[i] + c[i] * 2;
        } else {
            dst[i] = b[i] - c[i] * 2;
        }
    }
}

void test_le_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            dst[i] = a[i] + b[i] + c[i];
        } else {
            dst[i] = a[i] - b[i] - c[i];
        }
    }
}

/* Additional test with different loop lengths and patterns */

void test_mixed_comparisons(int *dst1, int *dst2, int *dst3, int *dst4,
                           int *a, int *b, int *c, int n) {
    /* Multiple comparisons in sequence */
    for (int i = 0; i < n; i++) {
        /* GT comparison */
        if (a[i] > b[i]) {
            dst1[i] = a[i] + 1;
        } else {
            dst1[i] = b[i] - 1;
        }
        
        /* GE comparison */
        if (a[i] >= c[i]) {
            dst2[i] = a[i] * 2;
        } else {
            dst2[i] = c[i] * 2;
        }
        
        /* LT comparison */
        if (b[i] < c[i]) {
            dst3[i] = b[i] + c[i];
        } else {
            dst3[i] = b[i] - c[i];
        }
        
        /* LE comparison */
        if (c[i] <= a[i]) {
            dst4[i] = c[i] << 2;
        } else {
            dst4[i] = a[i] << 2;
        }
    }
}

/* Main function with checksum computation */
int main() {
    /* Declare arrays */
    int a[SIZE], b[SIZE], c[SIZE];
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    int dst5[SIZE2], dst6[SIZE2], dst7[SIZE2], dst8[SIZE2];
    
    short sa[SIZE], sb[SIZE], sc[SIZE];
    short sdst1[SIZE], sdst2[SIZE], sdst3[SIZE], sdst4[SIZE];
    
    /* Initialize arrays */
    init_arrays(a, b, c, SIZE);
    init_arrays(dst5, dst6, dst7, dst8, SIZE2); /* Reuse init for different arrays */
    init_arrays_short(sa, sb, sc, SIZE);
    
    /* Test all comparison operators with int type */
    test_gt_loop(dst1, a, b, c, SIZE);
    test_ge_loop(dst2, a, b, c, SIZE);
    test_lt_loop(dst3, a, b, c, SIZE);
    test_le_loop(dst4, a, b, c, SIZE);
    
    /* Test with short type */
    test_gt_loop_short(sdst1, sa, sb, sc, SIZE);
    test_ge_loop_short(sdst2, sa, sb, sc, SIZE);
    test_lt_loop_short(sdst3, sa, sb, sc, SIZE);
    test_le_loop_short(sdst4, sa, sb, sc, SIZE);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst5, dst6, dst7, dst8, a, b, c, SIZE2);
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i];
        checksum += sdst1[i] + sdst2[i] + sdst3[i] + sdst4[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst5[i] + dst6[i] + dst7[i] + dst8[i];
    }
    
    /* Use volatile to prevent optimization */
    if (use_volatile) {
        printf("Checksum: %lu\n", (unsigned long)checksum);
    }
    
    return 0;
}
