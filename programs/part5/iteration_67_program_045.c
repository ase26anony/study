#include <stdio.h>
#include <stdlib.h>

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

/* GT_EXPR case - should trigger bitop1 = BIT_NOT_EXPR; bitop2 = BIT_AND_EXPR; */
void test_gt_loop(int *dst, const int *a, const int *b, const int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Generate mask from comparison, use for conditional update */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* GE_EXPR case - should trigger bitop1 = BIT_NOT_EXPR; bitop2 = BIT_IOR_EXPR; */
void test_ge_loop(short *dst, const short *a, const short *b, const short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Different integer width (short) to test different modes */
        if (a[i] >= b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* LT_EXPR case - should trigger bitop1 = BIT_NOT_EXPR; bitop2 = BIT_AND_EXPR; 
   with std::swap(cond_expr0, cond_expr1) */
void test_lt_loop(int *dst, const int *a, const int *b, const int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Less-than comparison for mask generation */
        if (a[i] < b[i]) {
            dst[i] = a[i] * c[i];
        } else {
            dst[i] = b[i] + c[i];
        }
    }
}

/* LE_EXPR case - should trigger bitop1 = BIT_NOT_EXPR; bitop2 = BIT_IOR_EXPR;
   with std::swap(cond_expr0, cond_expr1) */
void test_le_loop(short *dst, const short *a, const short *b, const short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Less-than-or-equal comparison */
        if (a[i] <= b[i]) {
            dst[i] = a[i] | c[i];  /* Bitwise operation to ensure it's not optimized */
        } else {
            dst[i] = b[i] & c[i];
        }
    }
}

/* Additional test with mixed comparisons in same loop */
void test_mixed_comparisons(int *dst1, int *dst2, const int *a, const int *b, const int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Multiple comparisons to stress the vectorizer */
        if (a[i] > b[i]) {
            dst1[i] = a[i] + 1;
        } else {
            dst1[i] = b[i] - 1;
        }
        
        if (a[i] <= c[i]) {
            dst2[i] = a[i] << 1;
        } else {
            dst2[i] = c[i] >> 1;
        }
    }
}

/* Test with volatile parameters to prevent optimization */
void test_volatile_gt(volatile int *dst, volatile const int *a, volatile const int *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            dst[i] = a[i] * 2;
        } else {
            dst[i] = b[i] / 2;
        }
    }
}

int main() {
    /* Declare arrays with different sizes */
    int src1_int[SIZE], src2_int[SIZE], src3_int[SIZE];
    short src1_short[SIZE2], src2_short[SIZE2], src3_short[SIZE2];
    
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    short dst_short1[SIZE2], dst_short2[SIZE2];
    
    volatile int dst_vol[SIZE];
    volatile int src_vol1[SIZE], src_vol2[SIZE];
    
    /* Initialize with semi-random data */
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
        src_vol1[i] = lcg_rand(&state) % 1000;
        src_vol2[i] = lcg_rand(&state) % 1000;
    }
    
    /* Call all test functions to trigger different comparison patterns */
    
    /* GT_EXPR - line 12216-12218 */
    test_gt_loop(dst1, src1_int, src2_int, src3_int, SIZE);
    
    /* GE_EXPR - line 12219-12221 */
    test_ge_loop(dst_short1, src1_short, src2_short, src3_short, SIZE2);
    
    /* LT_EXPR - line 12222-12226 */
    test_lt_loop(dst2, src1_int, src2_int, src3_int, SIZE);
    
    /* LE_EXPR - line 12227-12233 */
    test_le_loop(dst_short2, src1_short, src2_short, src3_short, SIZE2);
    
    /* Mixed comparisons */
    test_mixed_comparisons(dst3, dst4, src1_int, src2_int, src3_int, SIZE);
    
    /* Volatile test */
    if (use_volatile) {
        test_volatile_gt(dst_vol, src_vol1, src_vol2, SIZE);
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i];
        if (i < SIZE) {
            checksum += dst_vol[i];
        }
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst_short1[i] + dst_short2[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
