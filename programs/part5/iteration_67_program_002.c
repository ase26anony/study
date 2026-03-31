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

void init_arrays_short(short *a, short *b, short *c, int size) {
    int state = global_seed + 1;
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
        /* Generate mask from comparison, use it for conditional assignment */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];  /* Use comparison result */
        } else {
            dst[i] = b[i] - c[i];  /* Different computation for false case */
        }
    }
}

/* GE_EXPR case - should trigger bitop1 = BIT_NOT_EXPR; bitop2 = BIT_IOR_EXPR; */
void test_ge_loop(int *dst, const int *a, const int *b, const int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Greater-than-or-equal comparison */
        if (a[i] >= b[i]) {
            dst[i] = a[i] * 2 + c[i];
        } else {
            dst[i] = b[i] * 3 - c[i];
        }
    }
}

/* LT_EXPR case - should trigger bitop1 = BIT_NOT_EXPR; bitop2 = BIT_AND_EXPR; 
   with std::swap(cond_expr0, cond_expr1) */
void test_lt_loop(short *dst, const short *a, const short *b, const short *c, int n) {
    /* Use short type to trigger different mode conditions */
    for (int i = 0; i < n; i++) {
        /* Less-than comparison */
        if (a[i] < b[i]) {
            dst[i] = a[i] + c[i] * 2;
        } else {
            dst[i] = b[i] - c[i] / 2;
        }
    }
}

/* LE_EXPR case - should trigger bitop1 = BIT_NOT_EXPR; bitop2 = BIT_IOR_EXPR;
   with std::swap(cond_expr0, cond_expr1) */
void test_le_loop(short *dst, const short *a, const short *b, const short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Less-than-or-equal comparison */
        if (a[i] <= b[i]) {
            dst[i] = a[i] * 3 + c[i];
        } else {
            dst[i] = b[i] * 2 - c[i];
        }
    }
}

/* Additional test with mixed comparisons in same loop */
void test_mixed_comparisons(int *dst1, int *dst2, const int *a, const int *b, const int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Multiple comparisons to encourage vectorization */
        if (a[i] > b[i]) {
            dst1[i] = a[i] + b[i];
        } else {
            dst1[i] = c[i];
        }
        
        if (a[i] <= c[i]) {
            dst2[i] = b[i] - a[i];
        } else {
            dst2[i] = c[i] + b[i];
        }
    }
}

/* Test with volatile inputs to prevent optimization */
void test_volatile_gt(volatile int *dst, volatile const int *a, volatile const int *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            dst[i] = a[i] * 2;
        } else {
            dst[i] = b[i] / 2;
        }
    }
}

int main(void) {
    /* Declare arrays with different sizes */
    int src1_int[SIZE], src2_int[SIZE], src3_int[SIZE];
    short src1_short[SIZE2], src2_short[SIZE2], src3_short[SIZE2];
    
    int dst1_int[SIZE], dst2_int[SIZE], dst3_int[SIZE], dst4_int[SIZE];
    short dst1_short[SIZE2], dst2_short[SIZE2];
    int dst_mixed1[SIZE], dst_mixed2[SIZE];
    
    volatile int volatile_src1[SIZE], volatile_src2[SIZE], volatile_dst[SIZE];
    
    /* Initialize all arrays */
    init_arrays(src1_int, src2_int, src3_int, SIZE);
    init_arrays_short(src1_short, src2_short, src3_short, SIZE2);
    
    /* Initialize volatile arrays */
    int state = global_seed + 2;
    for (int i = 0; i < SIZE; i++) {
        volatile_src1[i] = lcg_rand(&state) % 1000;
        volatile_src2[i] = lcg_rand(&state) % 1000;
    }
    
    /* Execute all test functions to trigger different comparison patterns */
    
    /* GT_EXPR - greater than */
    test_gt_loop(dst1_int, src1_int, src2_int, src3_int, SIZE);
    
    /* GE_EXPR - greater than or equal */
    test_ge_loop(dst2_int, src1_int, src2_int, src3_int, SIZE);
    
    /* LT_EXPR - less than (with short type) */
    test_lt_loop(dst1_short, src1_short, src2_short, src3_short, SIZE2);
    
    /* LE_EXPR - less than or equal (with short type) */
    test_le_loop(dst2_short, src1_short, src2_short, src3_short, SIZE2);
    
    /* Mixed comparisons */
    test_mixed_comparisons(dst_mixed1, dst_mixed2, src1_int, src2_int, src3_int, SIZE);
    
    /* Volatile test */
    if (use_volatile) {
        test_volatile_gt(volatile_dst, volatile_src1, volatile_src2, SIZE);
    }
    
    /* Additional loops with different sizes */
    test_gt_loop(dst3_int, src2_int, src3_int, src1_int, 64);
    test_ge_loop(dst4_int, src3_int, src1_int, src2_int, 32);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1_int[i] + dst2_int[i];
        if (i < SIZE) {
            checksum += dst3_int[i] + dst4_int[i];
            checksum += dst_mixed1[i] + dst_mixed2[i];
        }
    }
    
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst1_short[i] + dst2_short[i];
    }
    
    if (use_volatile) {
        for (int i = 0; i < SIZE; i++) {
            checksum += volatile_dst[i];
        }
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
