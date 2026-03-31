/* Program to trigger vectorization of comparison operations
   targeting uncovered lines in tree-vect-stmts.cc (12216-12233) */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 12345;

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
void test_gt_loop(int *dst, int *src1, int *src2, int *src3, int size) {
    for (int i = 0; i < size; i++) {
        /* Generate mask from GT comparison, use for conditional assignment */
        if (src1[i] > src2[i]) {
            dst[i] = src1[i] + src3[i];
        } else {
            dst[i] = src2[i] - src3[i];
        }
    }
}

/* Test function for GE_EXPR (>=) */
void test_ge_loop(short *dst, short *src1, short *src2, short *src3, int size) {
    for (int i = 0; i < size; i++) {
        /* Generate mask from GE comparison */
        if (src1[i] >= src2[i]) {
            dst[i] = src1[i] + src3[i];
        } else {
            dst[i] = src2[i] - src3[i];
        }
    }
}

/* Test function for LT_EXPR (<) */
void test_lt_loop(int *dst, int *src1, int *src2, int *src3, int size) {
    for (int i = 0; i < size; i++) {
        /* Generate mask from LT comparison */
        if (src1[i] < src2[i]) {
            dst[i] = src1[i] * src3[i];
        } else {
            dst[i] = src2[i] + src3[i];
        }
    }
}

/* Test function for LE_EXPR (<=) */
void test_le_loop(short *dst, short *src1, short *src2, short *src3, int size) {
    for (int i = 0; i < size; i++) {
        /* Generate mask from LE comparison */
        if (src1[i] <= src2[i]) {
            dst[i] = src1[i] * src3[i];
        } else {
            dst[i] = src2[i] + src3[i];
        }
    }
}

/* Additional test with mixed comparisons in same loop */
void test_mixed_comparisons(int *dst1, int *dst2, int *src1, int *src2, int *src3, int size) {
    for (int i = 0; i < size; i++) {
        /* Multiple comparisons to trigger different paths */
        if (src1[i] > src2[i]) {
            dst1[i] = src1[i] + 1;
        } else {
            dst1[i] = src2[i] - 1;
        }
        
        if (src1[i] <= src3[i]) {
            dst2[i] = src1[i] * 2;
        } else {
            dst2[i] = src3[i] / 2;
        }
    }
}

/* Test with different integer widths */
void test_char_comparisons(char *dst, char *src1, char *src2, char *src3, int size) {
    for (int i = 0; i < size; i++) {
        /* Use all four comparison operators */
        if (src1[i] > src2[i]) {
            dst[i] = src1[i] + src3[i];
        } else if (src1[i] < src3[i]) {
            dst[i] = src2[i] - src3[i];
        } else if (src1[i] >= src2[i]) {
            dst[i] = src1[i] * 2;
        } else if (src1[i] <= src3[i]) {
            dst[i] = src3[i] / 2;
        } else {
            dst[i] = 0;
        }
    }
}

int main() {
    /* Declare arrays with different sizes and types */
    int src1_int[SIZE], src2_int[SIZE], src3_int[SIZE];
    short src1_short[SIZE2], src2_short[SIZE2], src3_short[SIZE2];
    char src1_char[SIZE], src2_char[SIZE], src3_char[SIZE];
    
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    short dst_short1[SIZE2], dst_short2[SIZE2];
    char dst_char[SIZE];
    
    /* Initialize with semi-random data */
    init_arrays(src1_int, src2_int, src3_int, SIZE);
    
    /* Initialize short arrays */
    int state = global_seed;
    for (int i = 0; i < SIZE2; i++) {
        src1_short[i] = lcg_rand(&state) % 1000;
        src2_short[i] = lcg_rand(&state) % 1000;
        src3_short[i] = lcg_rand(&state) % 1000;
    }
    
    /* Initialize char arrays */
    state = global_seed;
    for (int i = 0; i < SIZE; i++) {
        src1_char[i] = lcg_rand(&state) % 128;
        src2_char[i] = lcg_rand(&state) % 128;
        src3_char[i] = lcg_rand(&state) % 128;
    }
    
    /* Call all test functions to trigger different comparison operators */
    
    /* GT_EXPR - line 12216 */
    test_gt_loop(dst1, src1_int, src2_int, src3_int, SIZE);
    
    /* GE_EXPR - line 12220 */
    test_ge_loop(dst_short1, src1_short, src2_short, src3_short, SIZE2);
    
    /* LT_EXPR - line 12224 */
    test_lt_loop(dst2, src1_int, src2_int, src3_int, SIZE);
    
    /* LE_EXPR - line 12228 */
    test_le_loop(dst_short2, src1_short, src2_short, src3_short, SIZE2);
    
    /* Mixed comparisons */
    test_mixed_comparisons(dst3, dst4, src1_int, src2_int, src3_int, SIZE);
    
    /* Character type comparisons */
    test_char_comparisons(dst_char, src1_char, src2_char, src3_char, SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i] + dst_char[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst_short1[i] + dst_short2[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
