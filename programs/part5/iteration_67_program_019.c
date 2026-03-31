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

/* Test function for GT_EXPR (>) */
void test_gt_loop(int *dst, int *src1, int *src2, int *src3, int n) {
    volatile int bound = n; /* Prevent optimization */
    for (int i = 0; i < bound; i++) {
        /* Pattern: if (src1[i] > src2[i]) dst[i] = src1[i] + src3[i]; else dst[i] = src2[i]; */
        if (src1[i] > src2[i]) {
            dst[i] = src1[i] + src3[i];
        } else {
            dst[i] = src2[i];
        }
    }
}

/* Test function for GE_EXPR (>=) */
void test_ge_loop(short *dst, short *src1, short *src2, short *src3, int n) {
    volatile int bound = n;
    for (int i = 0; i < bound; i++) {
        /* Different data type (short) and pattern */
        if (src1[i] >= src2[i]) {
            dst[i] = src1[i] - src3[i];
        } else {
            dst[i] = src2[i] + src3[i];
        }
    }
}

/* Test function for LT_EXPR (<) */
void test_lt_loop(int *dst, int *src1, int *src2, int *src3, int n) {
    volatile int bound = n;
    for (int i = 0; i < bound; i++) {
        /* Pattern with swapped operands in computation */
        if (src1[i] < src2[i]) {
            dst[i] = src3[i] * src1[i];
        } else {
            dst[i] = src2[i] - src3[i];
        }
    }
}

/* Test function for LE_EXPR (<=) */
void test_le_loop(short *dst, short *src1, short *src2, short *src3, int n) {
    volatile int bound = n;
    for (int i = 0; i < bound; i++) {
        /* Different computation pattern */
        if (src1[i] <= src2[i]) {
            dst[i] = src1[i] | src3[i];
        } else {
            dst[i] = src2[i] & src3[i];
        }
    }
}

/* Additional test with mixed comparisons in same loop */
void test_mixed_comparisons(int *dst1, int *dst2, int *src1, int *src2, int n) {
    volatile int bound = n;
    for (int i = 0; i < bound; i++) {
        /* Multiple comparisons in same loop body */
        dst1[i] = (src1[i] > src2[i]) ? src1[i] : src2[i];
        dst2[i] = (src1[i] <= src2[i]) ? src1[i] + src2[i] : src1[i] - src2[i];
    }
}

/* Test with different loop length */
void test_variable_length(int *dst, int *src1, int *src2, int n) {
    volatile int bound = n;
    /* Loop with > comparison */
    for (int i = 0; i < bound; i++) {
        if (src1[i] > src2[i]) {
            dst[i] = src1[i] << 2;
        } else {
            dst[i] = src2[i] >> 1;
        }
    }
}

int main() {
    /* Declare arrays with different sizes */
    int src1_int[SIZE], src2_int[SIZE], src3_int[SIZE];
    short src1_short[SIZE2], src2_short[SIZE2], src3_short[SIZE2];
    
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    short dst_short1[SIZE2], dst_short2[SIZE2];
    
    int dst_mixed1[SIZE], dst_mixed2[SIZE];
    int dst_var[SIZE];
    
    /* Initialize with semi-random data */
    init_arrays(src1_int, src2_int, src3_int, SIZE);
    
    /* Initialize short arrays */
    int state = global_seed + 1;
    for (int i = 0; i < SIZE2; i++) {
        src1_short[i] = lcg_rand(&state) % 1000;
        src2_short[i] = lcg_rand(&state) % 1000;
        src3_short[i] = lcg_rand(&state) % 1000;
    }
    
    /* Test all comparison operators */
    test_gt_loop(dst1, src1_int, src2_int, src3_int, SIZE);
    test_ge_loop(dst_short1, src1_short, src2_short, src3_short, SIZE2);
    test_lt_loop(dst2, src1_int, src2_int, src3_int, SIZE);
    test_le_loop(dst_short2, src1_short, src2_short, src3_short, SIZE2);
    
    /* Additional tests */
    test_mixed_comparisons(dst_mixed1, dst_mixed2, src1_int, src2_int, SIZE);
    test_variable_length(dst_var, src1_int, src2_int, SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst_mixed1[i] + dst_mixed2[i] + dst_var[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst_short1[i] + dst_short2[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
