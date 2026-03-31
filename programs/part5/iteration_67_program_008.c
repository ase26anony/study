#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 12345;

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

/* Test function for GT_EXPR (>) transformation */
void test_gt_loop(int *dst, int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        /* This should generate GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i];
        }
    }
}

/* Test function for GE_EXPR (>=) transformation */
void test_ge_loop(short *dst, short *a, short *b, short *c, int size) {
    for (int i = 0; i < size; i++) {
        /* This should generate GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR */
        if (a[i] >= b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i];
        }
    }
}

/* Test function for LT_EXPR (<) transformation */
void test_lt_loop(int *dst, int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        /* This should generate LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
        if (a[i] < b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i];
        }
    }
}

/* Test function for LE_EXPR (<=) transformation */
void test_le_loop(short *dst, short *a, short *b, short *c, int size) {
    for (int i = 0; i < size; i++) {
        /* This should generate LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
        if (a[i] <= b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i];
        }
    }
}

/* Additional test with mixed comparisons in same loop */
void test_mixed_comparisons(int *dst1, int *dst2, int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        /* Mix of different comparisons to trigger multiple paths */
        if (a[i] > b[i]) {
            dst1[i] = a[i] + c[i];
        } else {
            dst1[i] = b[i] - c[i];
        }
        
        if (a[i] <= b[i]) {
            dst2[i] = a[i] * 2;
        } else {
            dst2[i] = b[i] * 3;
        }
    }
}

/* Test with different loop lengths and unroll hints */
void test_variable_length(int *dst, int *a, int *b, int *c, int size) {
    /* Volatile to prevent constant folding of loop bound */
    volatile int bound = size;
    for (int i = 0; i < bound; i++) {
        /* Use all four operators in different contexts */
        int temp = 0;
        if (a[i] > b[i]) temp += 1;
        if (a[i] >= c[i]) temp += 2;
        if (b[i] < c[i]) temp += 4;
        if (b[i] <= a[i]) temp += 8;
        dst[i] = temp;
    }
}

int main() {
    /* Declare arrays with different types and sizes */
    int src1_int[SIZE], src2_int[SIZE], src3_int[SIZE];
    short src1_short[SIZE2], src2_short[SIZE2], src3_short[SIZE2];
    
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    short dst_short1[SIZE2], dst_short2[SIZE2];
    int dst_mixed1[SIZE], dst_mixed2[SIZE];
    int dst_var[SIZE];
    
    /* Initialize all arrays */
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
    test_mixed_comparisons(dst_mixed1, dst_mixed2, src1_int, src2_int, src3_int, SIZE);
    
    /* Test variable length */
    test_variable_length(dst_var, src1_int, src2_int, src3_int, SIZE);
    
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
