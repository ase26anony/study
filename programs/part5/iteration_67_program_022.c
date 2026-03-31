#include <stdio.h>
#include <stdint.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 42;
volatile int use_volatile = 1;

/* Simple LCG for semi-random data */
static inline int lcg_next(int *state) {
    *state = *state * 1103515245 + 12345;
    return (*state >> 16) & 0x7FFF;
}

/* Initialize arrays with semi-random data */
void init_arrays(int *a, int *b, int *c, int size) {
    int state = global_seed;
    for (int i = 0; i < size; i++) {
        a[i] = lcg_next(&state);
        b[i] = lcg_next(&state);
        c[i] = lcg_next(&state);
    }
}

void init_arrays_short(short *a, short *b, short *c, int size) {
    int state = global_seed + 1;
    for (int i = 0; i < size; i++) {
        a[i] = (short)(lcg_next(&state) & 0x7FFF);
        b[i] = (short)(lcg_next(&state) & 0x7FFF);
        c[i] = (short)(lcg_next(&state) & 0x7FFF);
    }
}

/* Test functions for each comparison operator */

/* GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR */
void test_gt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
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
        if (a[i] >= b[i]) {
            dst[i] = a[i] * 2 + c[i];
        } else {
            dst[i] = b[i] * 2 - c[i];
        }
    }
}

/* LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
void test_lt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            dst[i] = a[i] + b[i] + c[i];
        } else {
            dst[i] = a[i] - b[i] - c[i];
        }
    }
}

/* LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
void test_le_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
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
            dst[i] = (short)(a[i] + c[i]);
        } else {
            dst[i] = (short)(b[i] - c[i]);
        }
    }
}

void test_ge_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            dst[i] = (short)(a[i] * 3 + c[i]);
        } else {
            dst[i] = (short)(b[i] * 3 - c[i]);
        }
    }
}

void test_lt_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            dst[i] = (short)(a[i] + c[i] * 2);
        } else {
            dst[i] = (short)(b[i] - c[i] * 2);
        }
    }
}

void test_le_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            dst[i] = (short)(a[i] + b[i] + c[i]);
        } else {
            dst[i] = (short)(a[i] - b[i] - c[i]);
        }
    }
}

/* Additional test with different array sizes */
void test_mixed_comparisons(int *dst1, int *dst2, int *a, int *b, int *c, int n) {
    /* Mix of comparisons in same function */
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            dst1[i] = a[i] + 1;
        } else {
            dst1[i] = b[i] - 1;
        }
        
        if (a[i] <= c[i]) {
            dst2[i] = b[i] + 2;
        } else {
            dst2[i] = c[i] - 2;
        }
    }
}

int main() {
    /* Declare arrays with different sizes */
    int a_int[SIZE], b_int[SIZE], c_int[SIZE];
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    int dst5[SIZE2], dst6[SIZE2];
    
    short a_short[SIZE], b_short[SIZE], c_short[SIZE];
    short dst_short1[SIZE], dst_short2[SIZE], dst_short3[SIZE], dst_short4[SIZE];
    
    /* Initialize arrays */
    init_arrays(a_int, b_int, c_int, SIZE);
    init_arrays(dst5, dst6, a_int, SIZE2); /* Reuse for initialization */
    init_arrays_short(a_short, b_short, c_short, SIZE);
    
    /* Test all comparison operators with int arrays */
    test_gt_loop(dst1, a_int, b_int, c_int, SIZE);
    test_ge_loop(dst2, a_int, b_int, c_int, SIZE);
    test_lt_loop(dst3, a_int, b_int, c_int, SIZE);
    test_le_loop(dst4, a_int, b_int, c_int, SIZE);
    
    /* Test with short arrays */
    test_gt_loop_short(dst_short1, a_short, b_short, c_short, SIZE);
    test_ge_loop_short(dst_short2, a_short, b_short, c_short, SIZE);
    test_lt_loop_short(dst_short3, a_short, b_short, c_short, SIZE);
    test_le_loop_short(dst_short4, a_short, b_short, c_short, SIZE);
    
    /* Test with different array size */
    test_mixed_comparisons(dst5, dst6, a_int, b_int, c_int, SIZE2);
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i];
        checksum += dst_short1[i] + dst_short2[i] + dst_short3[i] + dst_short4[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst5[i] + dst6[i];
    }
    
    printf("Checksum: %lu\n", (unsigned long)checksum);
    return 0;
}
