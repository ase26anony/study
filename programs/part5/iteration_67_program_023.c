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

/* Test greater-than (GT_EXPR) with int */
void test_gt_int(int *dst, int *src1, int *src2, int *src3, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR */
        if (src1[i] > src2[i]) {
            dst[i] = src1[i] + src3[i];
        } else {
            dst[i] = src2[i] - src3[i];
        }
    }
}

/* Test greater-than-or-equal (GE_EXPR) with short */
void test_ge_short(short *dst, short *src1, short *src2, short *src3, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR */
        if (src1[i] >= src2[i]) {
            dst[i] = src1[i] + src3[i];
        } else {
            dst[i] = src2[i] - src3[i];
        }
    }
}

/* Test less-than (LT_EXPR) with int, mixed pattern */
void test_lt_int(int *dst, int *src1, int *src2, int *src3, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
        if (src1[i] < src2[i]) {
            dst[i] = src3[i] * 2;
        } else {
            dst[i] = src1[i] + src2[i];
        }
    }
}

/* Test less-than-or-equal (LE_EXPR) with short */
void test_le_short(short *dst, short *src1, short *src2, short *src3, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
        if (src1[i] <= src2[i]) {
            dst[i] = src1[i] | src3[i];
        } else {
            dst[i] = src2[i] & src3[i];
        }
    }
}

/* Additional test with GT_EXPR on different size */
void test_gt_mixed(int *dst, short *src1, int *src2, int *src3, int n) {
    for (int i = 0; i < n; i++) {
        /* Mixed types to trigger different mode handling */
        if ((int)src1[i] > src2[i]) {
            dst[i] = src1[i] * 3;
        } else {
            dst[i] = src2[i] / 2;
        }
    }
}

/* Test GE_EXPR with volatile to prevent optimization */
void test_ge_volatile(int *dst, volatile int *src1, volatile int *src2, int *src3, int n) {
    for (int i = 0; i < n; i++) {
        /* Volatile reads prevent constant folding */
        int a = src1[i];
        int b = src2[i];
        if (a >= b) {
            dst[i] = a + src3[i];
        } else {
            dst[i] = b - src3[i];
        }
    }
}

/* Test LT_EXPR with stride-1 but unaligned start */
void test_lt_unaligned(int *dst, int *src1, int *src2, int *src3, int n) {
    /* Start from 1 to create unaligned access */
    for (int i = 1; i < n - 1; i++) {
        if (src1[i] < src2[i]) {
            dst[i] = src1[i] + src3[i];
        } else {
            dst[i] = src2[i] - src3[i];
        }
    }
}

/* Test LE_EXPR with multiple uses of comparison */
void test_le_multiple_use(int *dst1, int *dst2, int *src1, int *src2, int *src3, int n) {
    for (int i = 0; i < n; i++) {
        int cmp = src1[i] <= src2[i];
        dst1[i] = cmp ? src1[i] + src3[i] : src2[i];
        dst2[i] = cmp ? src3[i] * 2 : src1[i] - src2[i];
    }
}

int main(void) {
    /* Allocate aligned memory for better vectorization */
    int *src1_int = __builtin_assume_aligned(malloc(SIZE * sizeof(int)), 32);
    int *src2_int = __builtin_assume_aligned(malloc(SIZE * sizeof(int)), 32);
    int *src3_int = __builtin_assume_aligned(malloc(SIZE * sizeof(int)), 32);
    int *dst1_int = __builtin_assume_aligned(malloc(SIZE * sizeof(int)), 32);
    int *dst2_int = __builtin_assume_aligned(malloc(SIZE * sizeof(int)), 32);
    int *dst3_int = __builtin_assume_aligned(malloc(SIZE * sizeof(int)), 32);
    int *dst4_int = __builtin_assume_aligned(malloc(SIZE * sizeof(int)), 32);
    
    short *src1_short = __builtin_assume_aligned(malloc(SIZE2 * sizeof(short)), 32);
    short *src2_short = __builtin_assume_aligned(malloc(SIZE2 * sizeof(short)), 32);
    short *src3_short = __builtin_assume_aligned(malloc(SIZE2 * sizeof(short)), 32);
    short *dst1_short = __builtin_assume_aligned(malloc(SIZE2 * sizeof(short)), 32);
    short *dst2_short = __builtin_assume_aligned(malloc(SIZE2 * sizeof(short)), 32);
    
    volatile int *vol_src1 = __builtin_assume_aligned(malloc(SIZE * sizeof(int)), 32);
    volatile int *vol_src2 = __builtin_assume_aligned(malloc(SIZE * sizeof(int)), 32);
    
    if (!src1_int || !src2_int || !src3_int || !dst1_int || !dst2_int || 
        !dst3_int || !dst4_int || !src1_short || !src2_short || !src3_short ||
        !dst1_short || !dst2_short || !vol_src1 || !vol_src2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    init_arrays(src1_int, src2_int, src3_int, SIZE);
    init_arrays((int*)vol_src1, (int*)vol_src2, src3_int, SIZE);
    
    /* Initialize short arrays */
    int state = global_seed;
    for (int i = 0; i < SIZE2; i++) {
        src1_short[i] = lcg_rand(&state) % 1000;
        src2_short[i] = lcg_rand(&state) % 1000;
        src3_short[i] = lcg_rand(&state) % 1000;
    }
    
    /* Run all test functions */
    test_gt_int(dst1_int, src1_int, src2_int, src3_int, SIZE);
    test_ge_short(dst1_short, src1_short, src2_short, src3_short, SIZE2);
    test_lt_int(dst2_int, src1_int, src2_int, src3_int, SIZE);
    test_le_short(dst2_short, src1_short, src2_short, src3_short, SIZE2);
    test_gt_mixed(dst3_int, src1_short, src2_int, src3_int, SIZE2);
    test_ge_volatile(dst4_int, vol_src1, vol_src2, src3_int, SIZE);
    test_lt_unaligned(dst1_int, src1_int, src2_int, src3_int, SIZE);
    
    /* Test with multiple destinations */
    int *dst5_int = __builtin_assume_aligned(malloc(SIZE * sizeof(int)), 32);
    int *dst6_int = __builtin_assume_aligned(malloc(SIZE * sizeof(int)), 32);
    if (dst5_int && dst6_int) {
        test_le_multiple_use(dst5_int, dst6_int, src1_int, src2_int, src3_int, SIZE);
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1_int[i] + dst2_int[i] + dst3_int[i] + dst4_int[i];
        if (dst5_int && dst6_int) {
            checksum += dst5_int[i] + dst6_int[i];
        }
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst1_short[i] + dst2_short[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(src1_int); free(src2_int); free(src3_int);
    free(dst1_int); free(dst2_int); free(dst3_int); free(dst4_int);
    free(src1_short); free(src2_short); free(src3_short);
    free(dst1_short); free(dst2_short);
    free((void*)vol_src1); free((void*)vol_src2);
    if (dst5_int) free(dst5_int);
    if (dst6_int) free(dst6_int);
    
    return 0;
}
