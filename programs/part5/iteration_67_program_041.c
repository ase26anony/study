#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 12345;
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

/* Test functions for each comparison operator */

/* GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR */
void test_gt_loop(int *dst, const int *a, const int *b, const int *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR */
void test_ge_loop(int *dst, const int *a, const int *b, const int *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            dst[i] = a[i] * 2 + c[i];
        } else {
            dst[i] = b[i] * 3 - c[i];
        }
    }
}

/* LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
void test_lt_loop(int *dst, const int *a, const int *b, const int *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            dst[i] = a[i] - c[i];
        } else {
            dst[i] = b[i] + c[i];
        }
    }
}

/* LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
void test_le_loop(int *dst, const int *a, const int *b, const int *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            dst[i] = a[i] + b[i] + c[i];
        } else {
            dst[i] = a[i] - b[i] - c[i];
        }
    }
}

/* Mixed data types: short integers */
void test_gt_short(short *dst, const short *a, const short *b, const short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

void test_le_short(short *dst, const short *a, const short *b, const short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* Different loop length */
void test_ge_small(int *dst, const int *a, const int *b, const int *c) {
    for (int i = 0; i < SIZE2; i++) {
        if (a[i] >= b[i]) {
            dst[i] = a[i] + c[i] * 2;
        } else {
            dst[i] = b[i] - c[i] * 2;
        }
    }
}

/* Main function with checksum computation */
int main() {
    /* Declare arrays */
    int src1[SIZE], src2[SIZE], src3[SIZE];
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    short src1_short[SIZE], src2_short[SIZE], src3_short[SIZE];
    short dst_short1[SIZE], dst_short2[SIZE];
    int dst_small[SIZE2];
    
    /* Initialize arrays */
    init_arrays(src1, src2, src3, SIZE);
    
    /* Initialize short arrays */
    int state = global_seed;
    for (int i = 0; i < SIZE; i++) {
        src1_short[i] = lcg_rand(&state) % 1000;
        src2_short[i] = lcg_rand(&state) % 1000;
        src3_short[i] = lcg_rand(&state) % 1000;
    }
    
    /* Call test functions for each comparison operator */
    test_gt_loop(dst1, src1, src2, src3, SIZE);
    test_ge_loop(dst2, src1, src2, src3, SIZE);
    test_lt_loop(dst3, src1, src2, src3, SIZE);
    test_le_loop(dst4, src1, src2, src3, SIZE);
    
    /* Test with short data type */
    test_gt_short(dst_short1, src1_short, src2_short, src3_short, SIZE);
    test_le_short(dst_short2, src1_short, src2_short, src3_short, SIZE);
    
    /* Test with different loop length */
    test_ge_small(dst_small, src1, src2, src3);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i];
        checksum += dst_short1[i] + dst_short2[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst_small[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
