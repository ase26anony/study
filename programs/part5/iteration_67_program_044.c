/* Compile with: gcc -O3 -ftree-vectorize -fdump-tree-vect-details -march=native -o test_vector test_vector.c */

#include <stdio.h>
#include <stdint.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 42;
volatile int use_volatile = 1;

/* Simple LCG for semi-random values */
static inline int lcg_next(int *state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

/* Initialize arrays with semi-random data */
void init_arrays(int *a, int *b, int *c, int size) {
    int state = global_seed;
    for (int i = 0; i < size; i++) {
        a[i] = lcg_next(&state) % 1000;
        b[i] = lcg_next(&state) % 1000;
        c[i] = lcg_next(&state) % 1000;
    }
}

/* Test function for GT_EXPR (greater-than) */
void test_gt_loop(int *dst, int *src1, int *src2, int *src3, int n) {
    volatile int bound = n; /* Prevent loop unrolling before vectorization */
    for (int i = 0; i < bound; i++) {
        /* This should generate GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR */
        if (src1[i] > src2[i]) {
            dst[i] = src1[i] + src3[i];
        } else {
            dst[i] = src2[i] - src3[i];
        }
    }
}

/* Test function for GE_EXPR (greater-than-or-equal) */
void test_ge_loop(short *dst, short *src1, short *src2, short *src3, int n) {
    volatile int bound = n;
    for (int i = 0; i < bound; i++) {
        /* This should generate GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR */
        if (src1[i] >= src2[i]) {
            dst[i] = src1[i] + src3[i];
        } else {
            dst[i] = src2[i] - src3[i];
        }
    }
}

/* Test function for LT_EXPR (less-than) */
void test_lt_loop(int *dst, int *src1, int *src2, int *src3, int n) {
    volatile int bound = n;
    for (int i = 0; i < bound; i++) {
        /* This should generate LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
        if (src1[i] < src2[i]) {
            dst[i] = src1[i] * src3[i];
        } else {
            dst[i] = src2[i] / (src3[i] + 1); /* Avoid division by zero */
        }
    }
}

/* Test function for LE_EXPR (less-than-or-equal) */
void test_le_loop(short *dst, short *src1, short *src2, short *src3, int n) {
    volatile int bound = n;
    for (int i = 0; i < bound; i++) {
        /* This should generate LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
        if (src1[i] <= src2[i]) {
            dst[i] = src1[i] | src3[i];
        } else {
            dst[i] = src2[i] & src3[i];
        }
    }
}

/* Additional test with mixed comparisons in same loop */
void test_mixed_comparisons(int *dst1, int *dst2, int *src1, int *src2, int *src3, int n) {
    volatile int bound = n;
    for (int i = 0; i < bound; i++) {
        /* Mix of comparisons to ensure all paths are exercised */
        if (src1[i] > src2[i]) {
            dst1[i] = src1[i] + 1;
        } else {
            dst1[i] = src2[i] - 1;
        }
        
        if (src1[i] >= src3[i]) {
            dst2[i] = src1[i] * 2;
        } else {
            dst2[i] = src3[i] / 2;
        }
    }
}

/* Another test with different data type (char/short) */
void test_char_comparisons(char *dst, char *src1, char *src2, char *src3, int n) {
    volatile int bound = n;
    for (int i = 0; i < bound; i++) {
        /* Using < and <= with char type */
        if (src1[i] < src2[i]) {
            dst[i] = src1[i] + src3[i];
        } else if (src1[i] <= src3[i]) {
            dst[i] = src2[i] - src3[i];
        } else {
            dst[i] = src1[i];
        }
    }
}

int main() {
    /* Declare arrays with different sizes and types */
    int src1_int[SIZE], src2_int[SIZE], src3_int[SIZE];
    short src1_short[SIZE2], src2_short[SIZE2], src3_short[SIZE2];
    char src1_char[SIZE2], src2_char[SIZE2], src3_char[SIZE2];
    
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    short dst_short1[SIZE2], dst_short2[SIZE2];
    char dst_char[SIZE2];
    
    /* Initialize all arrays */
    init_arrays(src1_int, src2_int, src3_int, SIZE);
    
    /* Initialize short arrays */
    int state = global_seed;
    for (int i = 0; i < SIZE2; i++) {
        src1_short[i] = lcg_next(&state) % 1000;
        src2_short[i] = lcg_next(&state) % 1000;
        src3_short[i] = lcg_next(&state) % 1000;
        src1_char[i] = lcg_next(&state) % 128;
        src2_char[i] = lcg_next(&state) % 128;
        src3_char[i] = lcg_next(&state) % 128;
    }
    
    /* Call all test functions to exercise different comparison operators */
    test_gt_loop(dst1, src1_int, src2_int, src3_int, SIZE);
    test_ge_loop(dst_short1, src1_short, src2_short, src3_short, SIZE2);
    test_lt_loop(dst2, src1_int, src2_int, src3_int, SIZE);
    test_le_loop(dst_short2, src1_short, src2_short, src3_short, SIZE2);
    test_mixed_comparisons(dst3, dst4, src1_int, src2_int, src3_int, SIZE);
    test_char_comparisons(dst_char, src1_char, src2_char, src3_char, SIZE2);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst_short1[i] + dst_short2[i] + dst_char[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
