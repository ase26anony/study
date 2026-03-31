/* Program to trigger vectorization of comparison operations
   targeting uncovered lines in tree-vect-stmts.cc (12216-12233) */

#include <stdio.h>
#include <stdint.h>

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

/* GT_EXPR case: > comparison */
void test_gt_loop(int *dst, const int *a, const int *b, const int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GT_EXPR in vectorized form */
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
        /* This should generate GE_EXPR in vectorized form */
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
        /* This should generate LT_EXPR in vectorized form */
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
        /* This should generate LE_EXPR in vectorized form */
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
        /* Mixed types to trigger different conversions */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* Test with volatile inputs to prevent optimization */
void test_volatile_lt(volatile int *dst, volatile int *a, volatile int *b, int n) {
    for (int i = 0; i < n; i++) {
        /* Volatile prevents constant folding */
        if (a[i] < b[i]) {
            dst[i] = a[i] + 1;
        } else {
            dst[i] = b[i] - 1;
        }
    }
}

/* Main function with checksum computation */
int main() {
    /* Declare arrays with different sizes */
    int src1_int[SIZE];
    int src2_int[SIZE];
    int src3_int[SIZE];
    int dst1_int[SIZE];
    
    short src1_short[SIZE2];
    short src2_short[SIZE2];
    short src3_short[SIZE2];
    short dst2_short[SIZE2];
    
    int dst3_int[SIZE];
    short dst4_short[SIZE2];
    
    volatile int volatile_src1[SIZE];
    volatile int volatile_src2[SIZE];
    volatile int volatile_dst[SIZE];
    
    /* Initialize arrays */
    init_arrays(src1_int, src2_int, src3_int, SIZE);
    
    /* Initialize short arrays */
    int state = global_seed + 1;
    for (int i = 0; i < SIZE2; i++) {
        src1_short[i] = lcg_rand(&state) % 1000;
        src2_short[i] = lcg_rand(&state) % 1000;
        src3_short[i] = lcg_rand(&state) % 1000;
    }
    
    /* Initialize volatile arrays */
    state = global_seed + 2;
    for (int i = 0; i < SIZE; i++) {
        volatile_src1[i] = lcg_rand(&state) % 1000;
        volatile_src2[i] = lcg_rand(&state) % 1000;
    }
    
    /* Call all test functions to trigger different comparison operators */
    
    /* GT_EXPR - int arrays, size 256 */
    test_gt_loop(dst1_int, src1_int, src2_int, src3_int, SIZE);
    
    /* GE_EXPR - short arrays, size 128 */
    test_ge_loop(dst2_short, src1_short, src2_short, src3_short, SIZE2);
    
    /* LT_EXPR - int arrays, size 256 */
    test_lt_loop(dst3_int, src1_int, src2_int, src3_int, SIZE);
    
    /* LE_EXPR - short arrays, size 128 */
    test_le_loop(dst4_short, src1_short, src2_short, src3_short, SIZE2);
    
    /* Mixed type GT_EXPR */
    int mixed_dst[SIZE2];
    test_mixed_gt(mixed_dst, src1_short, src1_int, src3_short, SIZE2);
    
    /* Volatile LT_EXPR */
    if (use_volatile) {
        test_volatile_lt(volatile_dst, volatile_src1, volatile_src2, SIZE);
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1_int[i];
        checksum += dst3_int[i];
        if (i < SIZE) {
            checksum += volatile_dst[i];
        }
    }
    
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst2_short[i];
        checksum += dst4_short[i];
        checksum += mixed_dst[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
