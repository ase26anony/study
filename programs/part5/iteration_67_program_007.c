#include <stdio.h>
#include <stdint.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 42;
volatile int use_volatile = 1;

/* Simple LCG for semi-random data */
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

/* Test functions for each comparison operator */

/* GT_EXPR case: > comparison */
void test_gt_loop(int *dst, const int *a, const int *b, const int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GT_EXPR */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* GE_EXPR case: >= comparison */
void test_ge_loop(short *dst, const short *a, const short *b, const short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GE_EXPR */
        if (a[i] >= b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* LT_EXPR case: < comparison */
void test_lt_loop(int *dst, const int *a, const int *b, const int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LT_EXPR */
        if (a[i] < b[i]) {
            dst[i] = a[i] * c[i];
        } else {
            dst[i] = b[i] + c[i];
        }
    }
}

/* LE_EXPR case: <= comparison */
void test_le_loop(short *dst, const short *a, const short *b, const short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LE_EXPR */
        if (a[i] <= b[i]) {
            dst[i] = a[i] - c[i];
        } else {
            dst[i] = b[i] * c[i];
        }
    }
}

/* Mixed type test with GT_EXPR */
void test_mixed_gt(int *dst, const short *a, const int *b, const short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Mixed types, still GT_EXPR */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - a[i];
        }
    }
}

/* Another LT_EXPR variant with different computation */
void test_lt_variant(int *dst, const int *a, const int *b, const int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Different computation to avoid pattern recognition */
        dst[i] = (a[i] < b[i]) ? (a[i] << 2) : (b[i] >> 1);
    }
}

/* GE_EXPR with volatile to prevent optimization */
void test_ge_volatile(int *dst, volatile int *a, volatile int *b, int n) {
    for (int i = 0; i < n; i++) {
        /* Volatile reads force re-evaluation */
        if (a[i] >= b[i]) {
            dst[i] = a[i] + 1;
        } else {
            dst[i] = b[i] - 1;
        }
    }
}

int main() {
    /* Declare arrays with different sizes */
    int src1_int[SIZE];
    int src2_int[SIZE];
    int src3_int[SIZE];
    short src1_short[SIZE2];
    short src2_short[SIZE2];
    short src3_short[SIZE2];
    
    /* Destination arrays */
    int dst1[SIZE];
    int dst2[SIZE];
    short dst3[SIZE2];
    short dst4[SIZE2];
    int dst5[SIZE];
    int dst6[SIZE];
    int dst7[SIZE];
    
    /* Initialize source arrays */
    init_arrays(src1_int, src2_int, src3_int, SIZE);
    
    /* Initialize short arrays */
    int state = global_seed + 1;
    for (int i = 0; i < SIZE2; i++) {
        src1_short[i] = lcg_next(&state) % 1000;
        src2_short[i] = lcg_next(&state) % 1000;
        src3_short[i] = lcg_next(&state) % 1000;
    }
    
    /* Create volatile versions */
    volatile int volatile_src1[SIZE];
    volatile int volatile_src2[SIZE];
    for (int i = 0; i < SIZE; i++) {
        volatile_src1[i] = src1_int[i];
        volatile_src2[i] = src2_int[i];
    }
    
    /* Test all comparison operators */
    
    /* GT_EXPR tests */
    test_gt_loop(dst1, src1_int, src2_int, src3_int, SIZE);
    test_mixed_gt(dst5, src1_short, src2_int, src3_short, SIZE2);
    
    /* GE_EXPR tests */
    test_ge_loop(dst3, src1_short, src2_short, src3_short, SIZE2);
    if (use_volatile) {
        test_ge_volatile(dst6, volatile_src1, volatile_src2, SIZE);
    }
    
    /* LT_EXPR tests */
    test_lt_loop(dst2, src1_int, src2_int, src3_int, SIZE);
    test_lt_variant(dst7, src1_int, src2_int, src3_int, SIZE);
    
    /* LE_EXPR test */
    test_le_loop(dst4, src1_short, src2_short, src3_short, SIZE2);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i];
        if (i < SIZE) checksum += dst6[i];
        if (i < SIZE) checksum += dst7[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst3[i] + dst4[i];
        if (i < SIZE2) checksum += dst5[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
