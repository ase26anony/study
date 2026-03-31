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
void test_gt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GT_EXPR comparison */
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
        /* This should generate GE_EXPR comparison */
        if (a[i] >= b[i]) {
            dst[i] = a[i] * 2 + c[i];
        } else {
            dst[i] = b[i] * 3 - c[i];
        }
    }
}

/* LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
void test_lt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LT_EXPR comparison */
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
        /* This should generate LE_EXPR comparison */
        if (a[i] <= b[i]) {
            dst[i] = (a[i] << 1) + c[i];
        } else {
            dst[i] = (b[i] << 2) - c[i];
        }
    }
}

/* Mixed data types: short integers */
void test_gt_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* GT_EXPR with short type */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

void test_le_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* LE_EXPR with short type */
        if (a[i] <= b[i]) {
            dst[i] = a[i] * 3 + c[i];
        } else {
            dst[i] = b[i] * 2 - c[i];
        }
    }
}

/* Additional test with different array sizes */
void test_ge_loop_mixed(int *dst, int *a, int *b, int *c, int n) {
    /* Use volatile to prevent optimization */
    volatile int limit = n;
    for (int i = 0; i < limit; i++) {
        /* GE_EXPR with volatile loop bound */
        if (a[i] >= b[i]) {
            dst[i] = (a[i] & 0xFF) + c[i];
        } else {
            dst[i] = (b[i] & 0xFF) - c[i];
        }
    }
}

void test_lt_loop_mixed(int *dst, int *a, int *b, int *c, int n) {
    /* Another LT_EXPR test */
    for (int i = 0; i < n; i += 1) {  // Ensure stride 1
        if (a[i] < b[i]) {
            dst[i] = a[i] | c[i];
        } else {
            dst[i] = b[i] & c[i];
        }
    }
}

int main() {
    /* Declare arrays */
    int src1[SIZE], src2[SIZE], src3[SIZE];
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    short src1_short[SIZE2], src2_short[SIZE2], src3_short[SIZE2];
    short dst1_short[SIZE2], dst2_short[SIZE2];
    int dst_mixed1[SIZE2], dst_mixed2[SIZE2];
    
    /* Initialize arrays */
    init_arrays(src1, src2, src3, SIZE);
    
    /* Initialize short arrays */
    int state = global_seed;
    for (int i = 0; i < SIZE2; i++) {
        src1_short[i] = lcg_rand(&state) % 1000;
        src2_short[i] = lcg_rand(&state) % 1000;
        src3_short[i] = lcg_rand(&state) % 1000;
    }
    
    /* Call test functions for all comparison operators */
    
    /* GT_EXPR tests */
    test_gt_loop(dst1, src1, src2, src3, SIZE);
    test_gt_loop_short(dst1_short, src1_short, src2_short, src3_short, SIZE2);
    
    /* GE_EXPR tests */
    test_ge_loop(dst2, src1, src2, src3, SIZE);
    test_ge_loop_mixed(dst_mixed1, src1, src2, src3, SIZE2);
    
    /* LT_EXPR tests */
    test_lt_loop(dst3, src1, src2, src3, SIZE);
    test_lt_loop_mixed(dst_mixed2, src1, src2, src3, SIZE2);
    
    /* LE_EXPR tests */
    test_le_loop(dst4, src1, src2, src3, SIZE);
    test_le_loop_short(dst2_short, src1_short, src2_short, src3_short, SIZE2);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst1_short[i] + dst2_short[i] + 
                   dst_mixed1[i] + dst_mixed2[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
