/* Program to trigger vectorization of comparison operations for GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define SIZE2 128

/* Global volatile arrays to prevent optimization */
volatile int global_seed = 12345;

/* Simple LCG for semi-random data */
static int lcg(int *state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

/* Initialize arrays with semi-random data */
void init_arrays(int *a, int *b, int *c, int size) {
    int state = global_seed;
    for (int i = 0; i < size; i++) {
        a[i] = lcg(&state) % 1000;
        b[i] = lcg(&state) % 1000;
        c[i] = lcg(&state) % 1000;
    }
}

/* Test function for GT_EXPR (greater than) */
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

/* Test function for GE_EXPR (greater than or equal) */
void test_ge_loop(short *dst, const short *a, const short *b, const short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GE_EXPR in vectorized form */
        if (a[i] >= b[i]) {
            dst[i] = (short)(a[i] + c[i]);
        } else {
            dst[i] = (short)(b[i] - c[i]);
        }
    }
}

/* Test function for LT_EXPR (less than) */
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

/* Test function for LE_EXPR (less than or equal) */
void test_le_loop(short *dst, const short *a, const short *b, const short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LE_EXPR in vectorized form */
        if (a[i] <= b[i]) {
            dst[i] = (short)(a[i] - c[i]);
        } else {
            dst[i] = (short)(b[i] * c[i]);
        }
    }
}

/* Additional test with mixed comparisons in same loop */
void test_mixed_comparisons(int *dst1, int *dst2, const int *a, const int *b, const int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Multiple comparisons to potentially trigger different paths */
        if (a[i] > b[i]) {
            dst1[i] = a[i] + 1;
        } else {
            dst1[i] = b[i] - 1;
        }
        
        if (a[i] <= c[i]) {
            dst2[i] = a[i] * 2;
        } else {
            dst2[i] = c[i] / 2;
        }
    }
}

/* Test with different loop lengths */
void test_various_lengths(int *dst, const int *a, const int *b, int n) {
    /* Variable length loop */
    volatile int len = n;  /* volatile to prevent constant propagation */
    for (int i = 0; i < len; i++) {
        if (a[i] >= b[i]) {
            dst[i] = a[i];
        } else {
            dst[i] = b[i];
        }
    }
}

int main(void) {
    /* Declare arrays with different sizes */
    int src1_int[SIZE], src2_int[SIZE], src3_int[SIZE];
    short src1_short[SIZE2], src2_short[SIZE2], src3_short[SIZE2];
    
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    short dst_short1[SIZE2], dst_short2[SIZE2];
    int dst_mixed1[SIZE], dst_mixed2[SIZE];
    
    /* Initialize arrays */
    init_arrays(src1_int, src2_int, src3_int, SIZE);
    
    /* Initialize short arrays */
    int state = global_seed;
    for (int i = 0; i < SIZE2; i++) {
        src1_short[i] = (short)(lcg(&state) % 1000);
        src2_short[i] = (short)(lcg(&state) % 1000);
        src3_short[i] = (short)(lcg(&state) % 1000);
    }
    
    /* Test all comparison operators */
    test_gt_loop(dst1, src1_int, src2_int, src3_int, SIZE);
    test_ge_loop(dst_short1, src1_short, src2_short, src3_short, SIZE2);
    test_lt_loop(dst2, src1_int, src2_int, src3_int, SIZE);
    test_le_loop(dst_short2, src1_short, src2_short, src3_short, SIZE2);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst3, dst4, src1_int, src2_int, src3_int, SIZE);
    
    /* Test with different loop length */
    test_various_lengths(dst_mixed1, src1_int, src2_int, SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i] + dst_mixed1[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst_short1[i] + dst_short2[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
