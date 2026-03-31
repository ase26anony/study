/* test_vector_comparisons.c
 * Designed to trigger GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR transformations
 * in tree-vect-stmts.cc lines 12216-12233
 */

#include <stdio.h>
#include <stdint.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 12345;
volatile int global_mod = 100;

/* Simple LCG for semi-random data */
static inline int pseudo_rand(int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

/* Initialize arrays with semi-random data */
void init_arrays(int *a, int *b, int *c, int n) {
    int seed = global_seed;
    for (int i = 0; i < n; i++) {
        a[i] = pseudo_rand(&seed) % global_mod;
        b[i] = pseudo_rand(&seed) % global_mod;
        c[i] = pseudo_rand(&seed) % global_mod;
    }
}

/* Test GT_EXPR transformation */
void test_gt_loop(int *dst, int *src1, int *src2, int *src3, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR */
        if (src1[i] > src2[i]) {
            dst[i] = src1[i] + src3[i];
        } else {
            dst[i] = src2[i] - src3[i];
        }
    }
}

/* Test GE_EXPR transformation */
void test_ge_loop(short *dst, short *src1, short *src2, short *src3, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR */
        if (src1[i] >= src2[i]) {
            dst[i] = src1[i] + src3[i];
        } else {
            dst[i] = src2[i] - src3[i];
        }
    }
}

/* Test LT_EXPR transformation */
void test_lt_loop(int *dst, int *src1, int *src2, int *src3, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
        if (src1[i] < src2[i]) {
            dst[i] = src1[i] * src3[i];
        } else {
            dst[i] = src2[i] / (src3[i] + 1);
        }
    }
}

/* Test LE_EXPR transformation */
void test_le_loop(short *dst, short *src1, short *src2, short *src3, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
        if (src1[i] <= src2[i]) {
            dst[i] = src1[i] | src3[i];
        } else {
            dst[i] = src2[i] & src3[i];
        }
    }
}

/* Additional test with mixed comparisons */
void test_mixed_comparisons(int *dst, int *src1, int *src2, int *src3, int n) {
    for (int i = 0; i < n; i++) {
        /* Mix of comparisons to ensure all paths are exercised */
        if (src1[i] > src2[i]) {
            dst[i] = src1[i] + 1;
        } else if (src1[i] >= src2[i]) {
            dst[i] = src2[i] - 1;
        } else if (src1[i] < src2[i]) {
            dst[i] = src3[i] * 2;
        } else if (src1[i] <= src2[i]) {
            dst[i] = src3[i] / 2;
        }
    }
}

/* Test with different loop lengths and unrolling hints */
void test_variable_length(int *dst, int *src1, int *src2, int *src3, int n) {
    /* Volatile to prevent loop unrolling before vectorization */
    volatile int vn = n;
    for (int i = 0; i < vn; i++) {
        /* Use all comparison operators */
        if (i % 4 == 0) {
            if (src1[i] > src2[i]) dst[i] = 1;
        } else if (i % 4 == 1) {
            if (src1[i] >= src2[i]) dst[i] = 2;
        } else if (i % 4 == 2) {
            if (src1[i] < src2[i]) dst[i] = 3;
        } else {
            if (src1[i] <= src2[i]) dst[i] = 4;
        }
    }
}

int main() {
    /* Declare arrays with different sizes */
    int src1_int[SIZE], src2_int[SIZE], src3_int[SIZE];
    short src1_short[SIZE2], src2_short[SIZE2], src3_short[SIZE2];
    
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    short dst_short1[SIZE2], dst_short2[SIZE2];
    
    int dst_mixed[SIZE];
    int dst_variable[SIZE];
    
    /* Initialize data */
    init_arrays(src1_int, src2_int, src3_int, SIZE);
    
    /* Initialize short arrays */
    int seed = global_seed;
    for (int i = 0; i < SIZE2; i++) {
        src1_short[i] = pseudo_rand(&seed) % 100;
        src2_short[i] = pseudo_rand(&seed) % 100;
        src3_short[i] = pseudo_rand(&seed) % 100;
    }
    
    /* Test all comparison operators */
    test_gt_loop(dst1, src1_int, src2_int, src3_int, SIZE);
    test_ge_loop(dst_short1, src1_short, src2_short, src3_short, SIZE2);
    test_lt_loop(dst2, src1_int, src2_int, src3_int, SIZE);
    test_le_loop(dst_short2, src1_short, src2_short, src3_short, SIZE2);
    
    /* Additional tests for coverage */
    test_mixed_comparisons(dst3, src1_int, src2_int, src3_int, SIZE);
    test_variable_length(dst4, src1_int, src2_int, src3_int, SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst_short1[i] + dst_short2[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
