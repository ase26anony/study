/* test_vector_comparisons.c
 * Designed to trigger GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR transformations
 * in tree-vect-stmts.cc lines 12216-12233
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 42;
volatile int use_volatile = 1;

/* Simple LCG for semi-random values */
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

/* Test GT_EXPR transformation */
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

/* Test GE_EXPR transformation */
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

/* Test LT_EXPR transformation */
void test_lt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
        if (a[i] < b[i]) {
            dst[i] = a[i] + b[i] + c[i];
        } else {
            dst[i] = a[i] - b[i] - c[i];
        }
    }
}

/* Test LE_EXPR transformation */
void test_le_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
        if (a[i] <= b[i]) {
            dst[i] = a[i] * 3 + c[i];
        } else {
            dst[i] = b[i] * 3 - c[i];
        }
    }
}

/* Test with short integers to trigger different modes */
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

/* Test with mixed lengths */
void test_ge_loop_mixed(int *dst, int *a, int *b, int *c, int n) {
    /* Use volatile to prevent optimization */
    volatile int limit = n;
    for (int i = 0; i < limit; i++) {
        /* GE_EXPR with volatile boundary */
        if (a[i] >= b[i]) {
            dst[i] = a[i] + b[i] + c[i];
        } else {
            dst[i] = a[i] - b[i] - c[i];
        }
    }
}

/* Test LT_EXPR with different pattern */
void test_lt_loop_pattern(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* LT_EXPR with more complex computation */
        int temp = (a[i] < b[i]) ? (a[i] * c[i]) : (b[i] / (c[i] + 1));
        dst[i] = temp + i;
    }
}

/* Test LE_EXPR with stride-1 but different access */
void test_le_loop_stride(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n - 1; i++) {
        /* LE_EXPR with simple stride */
        if (a[i] <= b[i + 1]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
    /* Handle last element */
    if (n > 0) {
        dst[n - 1] = a[n - 1];
    }
}

int main() {
    /* Declare arrays with different sizes */
    int a[SIZE], b[SIZE], c[SIZE];
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    
    short a_short[SIZE2], b_short[SIZE2], c_short[SIZE2];
    short dst_short[SIZE2];
    
    int a_mixed[128], b_mixed[128], c_mixed[128];
    int dst_mixed[128];
    
    /* Initialize all arrays */
    init_arrays(a, b, c, SIZE);
    init_arrays(a_mixed, b_mixed, c_mixed, 128);
    
    /* Initialize short arrays */
    int state = global_seed;
    for (int i = 0; i < SIZE2; i++) {
        a_short[i] = (short)(lcg_rand(&state) % 1000);
        b_short[i] = (short)(lcg_rand(&state) % 1000);
        c_short[i] = (short)(lcg_rand(&state) % 1000);
    }
    
    /* Execute all test loops */
    test_gt_loop(dst1, a, b, c, SIZE);
    test_ge_loop(dst2, a, b, c, SIZE);
    test_lt_loop(dst3, a, b, c, SIZE);
    test_le_loop(dst4, a, b, c, SIZE);
    
    test_gt_loop_short(dst_short, a_short, b_short, c_short, SIZE2);
    test_ge_loop_mixed(dst_mixed, a_mixed, b_mixed, c_mixed, 128);
    
    /* Additional tests with different patterns */
    test_lt_loop_pattern(dst1, a, b, c, SIZE);
    test_le_loop_stride(dst2, a, b, c, SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst_short[i];
    }
    for (int i = 0; i < 128; i++) {
        checksum += dst_mixed[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
