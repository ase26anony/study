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

/* GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR */
void test_gt_loop(int *dst, int *src1, int *src2, int *src3, int n) {
    volatile int limit = n; /* Prevent optimization */
    for (int i = 0; i < limit; i++) {
        /* Generate mask from > comparison */
        if (src1[i] > src2[i]) {
            dst[i] = src1[i] + src3[i];
        } else {
            dst[i] = src2[i] - src3[i];
        }
    }
}

/* GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR */
void test_ge_loop(short *dst, short *src1, short *src2, short *src3, int n) {
    volatile int limit = n;
    for (int i = 0; i < limit; i++) {
        /* Generate mask from >= comparison */
        if (src1[i] >= src2[i]) {
            dst[i] = src1[i] + src3[i];
        } else {
            dst[i] = src2[i] - src3[i];
        }
    }
}

/* LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
void test_lt_loop(int *dst, int *src1, int *src2, int *src3, int n) {
    volatile int limit = n;
    for (int i = 0; i < limit; i++) {
        /* Generate mask from < comparison */
        if (src1[i] < src2[i]) {
            dst[i] = src1[i] * src3[i];
        } else {
            dst[i] = src2[i] + src3[i];
        }
    }
}

/* LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
void test_le_loop(short *dst, short *src1, short *src2, short *src3, int n) {
    volatile int limit = n;
    for (int i = 0; i < limit; i++) {
        /* Generate mask from <= comparison */
        if (src1[i] <= src2[i]) {
            dst[i] = src1[i] * src3[i];
        } else {
            dst[i] = src2[i] + src3[i];
        }
    }
}

/* Additional test with mixed sizes to trigger different vectorization factors */
void test_mixed_comparisons(int *dst1, int *dst2, 
                           short *dst3, short *dst4,
                           int *src1_int, int *src2_int, int *src3_int,
                           short *src1_short, short *src2_short, short *src3_short) {
    /* Test all four operators with different data types */
    
    /* GT with int - should trigger GT_EXPR case */
    for (int i = 0; i < SIZE; i++) {
        if (src1_int[i] > src2_int[i]) {
            dst1[i] = src1_int[i] + src3_int[i];
        } else {
            dst1[i] = src2_int[i];
        }
    }
    
    /* GE with int - should trigger GE_EXPR case */
    for (int i = 0; i < SIZE; i++) {
        if (src1_int[i] >= src2_int[i]) {
            dst2[i] = src1_int[i] - src3_int[i];
        } else {
            dst2[i] = src2_int[i];
        }
    }
    
    /* LT with short - should trigger LT_EXPR case with swap */
    for (int i = 0; i < SIZE2; i++) {
        if (src1_short[i] < src2_short[i]) {
            dst3[i] = src1_short[i] + 1;
        } else {
            dst3[i] = src2_short[i] - 1;
        }
    }
    
    /* LE with short - should trigger LE_EXPR case with swap */
    for (int i = 0; i < SIZE2; i++) {
        if (src1_short[i] <= src2_short[i]) {
            dst4[i] = src1_short[i] * 2;
        } else {
            dst4[i] = src2_short[i] / 2;
        }
    }
}

int main() {
    /* Declare arrays with different sizes and types */
    int src1_int[SIZE], src2_int[SIZE], src3_int[SIZE];
    short src1_short[SIZE2], src2_short[SIZE2], src3_short[SIZE2];
    
    int dst1[SIZE], dst2[SIZE];
    short dst3[SIZE2], dst4[SIZE2];
    
    /* Additional destination arrays for individual tests */
    int dst_gt[SIZE], dst_lt[SIZE];
    short dst_ge[SIZE2], dst_le[SIZE2];
    
    /* Initialize source arrays */
    init_arrays(src1_int, src2_int, src3_int, SIZE);
    
    /* Initialize short arrays */
    int state = global_seed + 1;
    for (int i = 0; i < SIZE2; i++) {
        src1_short[i] = lcg_rand(&state) % 500;
        src2_short[i] = lcg_rand(&state) % 500;
        src3_short[i] = lcg_rand(&state) % 500;
    }
    
    /* Test each comparison operator individually */
    test_gt_loop(dst_gt, src1_int, src2_int, src3_int, SIZE);
    test_ge_loop(dst_ge, src1_short, src2_short, src3_short, SIZE2);
    test_lt_loop(dst_lt, src1_int, src2_int, src3_int, SIZE);
    test_le_loop(dst_le, src1_short, src2_short, src3_short, SIZE2);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst1, dst2, dst3, dst4,
                          src1_int, src2_int, src3_int,
                          src1_short, src2_short, src3_short);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        checksum += dst_gt[i] + dst_lt[i];
        if (i < SIZE) {
            checksum += dst1[i] + dst2[i];
        }
    }
    
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst_ge[i] + dst_le[i];
        if (i < SIZE2) {
            checksum += dst3[i] + dst4[i];
        }
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
