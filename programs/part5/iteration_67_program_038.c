#include <stdio.h>
#include <stdint.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 12345;
volatile int use_volatile = 1;

/* Simple LCG for semi-random data */
static inline int lcg(int *state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

/* Initialize arrays with semi-random data */
void init_arrays(int *a, int *b, int *c, int n) {
    int state = global_seed;
    for (int i = 0; i < n; i++) {
        a[i] = lcg(&state) % 1000;
        b[i] = lcg(&state) % 1000;
        c[i] = lcg(&state) % 1000;
    }
}

void init_arrays_short(short *a, short *b, short *c, int n) {
    int state = global_seed + 1;
    for (int i = 0; i < n; i++) {
        a[i] = lcg(&state) % 1000;
        b[i] = lcg(&state) % 1000;
        c[i] = lcg(&state) % 1000;
    }
}

/* Test functions for each comparison operator */

/* GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR */
void test_gt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Generate mask from comparison and use it for conditional update */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR */
void test_ge_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Different computation pattern to avoid CSE */
        if (a[i] >= b[i]) {
            dst[i] = (a[i] << 1) + c[i];
        } else {
            dst[i] = (b[i] >> 1) - c[i];
        }
    }
}

/* LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swapped operands */
void test_lt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Less-than comparison should trigger operand swap */
        if (a[i] < b[i]) {
            dst[i] = a[i] * 2 + c[i];
        } else {
            dst[i] = b[i] * 3 - c[i];
        }
    }
}

/* LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swapped operands */
void test_le_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Less-than-or-equal comparison */
        if (a[i] <= b[i]) {
            dst[i] = a[i] + (c[i] << 2);
        } else {
            dst[i] = b[i] - (c[i] >> 2);
        }
    }
}

/* Mixed data types to trigger different vector modes */

/* GT_EXPR with short (16-bit) integers */
void test_gt_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* GE_EXPR with short integers */
void test_ge_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            dst[i] = (a[i] << 1) | c[i];
        } else {
            dst[i] = (b[i] >> 1) & c[i];
        }
    }
}

/* LT_EXPR with short integers */
void test_lt_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            dst[i] = a[i] ^ c[i];
        } else {
            dst[i] = b[i] | c[i];
        }
    }
}

/* LE_EXPR with short integers */
void test_le_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            dst[i] = a[i] + c[i] * 2;
        } else {
            dst[i] = b[i] - c[i] / 2;
        }
    }
}

/* Additional test with different array sizes to ensure vectorization attempts */

void test_mixed_comparisons(int n) {
    int a[SIZE], b[SIZE], c[SIZE], dst[SIZE];
    
    /* Initialize with function parameter to prevent constant folding */
    init_arrays(a, b, c, n);
    
    /* Test all four operators in sequence */
    test_gt_loop(dst, a, b, c, n);
    test_ge_loop(dst, a, b, c, n);
    test_lt_loop(dst, a, b, c, n);
    test_le_loop(dst, a, b, c, n);
}

int main() {
    /* Arrays for int tests */
    int a_int[SIZE], b_int[SIZE], c_int[SIZE];
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    
    /* Arrays for short tests */
    short a_short[SIZE2], b_short[SIZE2], c_short[SIZE2];
    short dst1_short[SIZE2], dst2_short[SIZE2], dst3_short[SIZE2], dst4_short[SIZE2];
    
    /* Initialize arrays */
    init_arrays(a_int, b_int, c_int, SIZE);
    init_arrays_short(a_short, b_short, c_short, SIZE2);
    
    /* Test with int arrays (32-bit) */
    test_gt_loop(dst1, a_int, b_int, c_int, SIZE);
    test_ge_loop(dst2, a_int, b_int, c_int, SIZE);
    test_lt_loop(dst3, a_int, b_int, c_int, SIZE);
    test_le_loop(dst4, a_int, b_int, c_int, SIZE);
    
    /* Test with short arrays (16-bit) */
    test_gt_loop_short(dst1_short, a_short, b_short, c_short, SIZE2);
    test_ge_loop_short(dst2_short, a_short, b_short, c_short, SIZE2);
    test_lt_loop_short(dst3_short, a_short, b_short, c_short, SIZE2);
    test_le_loop_short(dst4_short, a_short, b_short, c_short, SIZE2);
    
    /* Additional test with parameterized size */
    test_mixed_comparisons(SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i];
    }
    
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst1_short[i] + dst2_short[i] + dst3_short[i] + dst4_short[i];
    }
    
    printf("Checksum: %lu\n", (unsigned long)checksum);
    
    return 0;
}
