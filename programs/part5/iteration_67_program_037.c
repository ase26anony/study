#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 12345;
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
        a[i] = (lcg_rand(&state) % 1000) - 500;
        b[i] = (lcg_rand(&state) % 1000) - 500;
        c[i] = (lcg_rand(&state) % 1000) - 500;
    }
}

/* Test functions for each comparison operator */

/* GT_EXPR case: greater-than */
void test_gt_loop(int *dst, int *src1, int *src2, int *src3, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GT_EXPR */
        if (src1[i] > src2[i]) {
            dst[i] = src1[i] + src3[i];
        } else {
            dst[i] = src2[i] - src3[i];
        }
    }
}

/* GE_EXPR case: greater-than-or-equal */
void test_ge_loop(short *dst, short *src1, short *src2, short *src3, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GE_EXPR */
        if (src1[i] >= src2[i]) {
            dst[i] = src1[i] + src3[i];
        } else {
            dst[i] = src2[i] - src3[i];
        }
    }
}

/* LT_EXPR case: less-than */
void test_lt_loop(int *dst, int *src1, int *src2, int *src3, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LT_EXPR */
        if (src1[i] < src2[i]) {
            dst[i] = src1[i] * src3[i];
        } else {
            dst[i] = src2[i] / (src3[i] != 0 ? src3[i] : 1);
        }
    }
}

/* LE_EXPR case: less-than-or-equal */
void test_le_loop(short *dst, short *src1, short *src2, short *src3, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LE_EXPR */
        if (src1[i] <= src2[i]) {
            dst[i] = src1[i] | src3[i];
        } else {
            dst[i] = src2[i] & src3[i];
        }
    }
}

/* Mixed types and sizes to trigger different vectorization paths */
void test_mixed_comparisons(void) {
    /* Arrays of different sizes and types */
    int a_int[SIZE], b_int[SIZE], c_int[SIZE], dst_int[SIZE];
    short a_short[SIZE2], b_short[SIZE2], c_short[SIZE2], dst_short[SIZE2];
    
    /* Initialize with volatile read to prevent constant folding */
    int size_int = use_volatile ? SIZE : 0;
    int size_short = use_volatile ? SIZE2 : 0;
    
    init_arrays(a_int, b_int, c_int, size_int);
    
    /* Initialize short arrays */
    int state = global_seed + 1;
    for (int i = 0; i < size_short; i++) {
        a_short[i] = (lcg_rand(&state) % 1000) - 500;
        b_short[i] = (lcg_rand(&state) % 1000) - 500;
        c_short[i] = (lcg_rand(&state) % 1000) - 500;
    }
    
    /* Test all comparison operators */
    test_gt_loop(dst_int, a_int, b_int, c_int, size_int);
    test_ge_loop(dst_short, a_short, b_short, c_short, size_short);
    test_lt_loop(dst_int, b_int, a_int, c_int, size_int);  /* Swapped to ensure some true conditions */
    test_le_loop(dst_short, b_short, a_short, c_short, size_short);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < size_int; i++) {
        checksum += dst_int[i];
    }
    for (int i = 0; i < size_short; i++) {
        checksum += dst_short[i];
    }
    
    /* Use checksum to prevent optimization */
    if (checksum == 0) {
        printf("Zero checksum\n");
    }
}

/* Additional test with different loop lengths */
void test_various_lengths(void) {
    int sizes[] = {64, 128, 256};
    
    for (int s = 0; s < 3; s++) {
        int n = sizes[s];
        int a[n], b[n], c[n], dst[n];
        
        init_arrays(a, b, c, n);
        
        /* Test each comparison with different loop lengths */
        for (int i = 0; i < n; i++) {
            /* GT_EXPR */
            if (a[i] > b[i]) {
                dst[i] = a[i] + c[i];
            } else {
                dst[i] = b[i];
            }
        }
        
        /* Another pass with LE_EXPR */
        for (int i = 0; i < n; i++) {
            /* LE_EXPR */
            if (a[i] <= c[i]) {
                dst[i] += b[i];
            } else {
                dst[i] -= c[i];
            }
        }
    }
}

/* Main function with execution flow */
int main(void) {
    /* Test with mixed comparisons */
    test_mixed_comparisons();
    
    /* Test with various loop lengths */
    test_various_lengths();
    
    /* Additional direct tests to ensure all operators are covered */
    int final_a[100], final_b[100], final_c[100], final_dst[100];
    init_arrays(final_a, final_b, final_c, 100);
    
    /* Explicit GT comparison */
    for (int i = 0; i < 100; i++) {
        if (final_a[i] > final_b[i]) {
            final_dst[i] = 1;
        }
    }
    
    /* Explicit GE comparison */
    for (int i = 0; i < 100; i++) {
        if (final_a[i] >= final_c[i]) {
            final_dst[i] += 2;
        }
    }
    
    /* Explicit LT comparison */
    for (int i = 0; i < 100; i++) {
        if (final_b[i] < final_c[i]) {
            final_dst[i] += 3;
        }
    }
    
    /* Explicit LE comparison */
    for (int i = 0; i < 100; i++) {
        if (final_b[i] <= final_a[i]) {
            final_dst[i] += 4;
        }
    }
    
    /* Final checksum */
    int final_checksum = 0;
    for (int i = 0; i < 100; i++) {
        final_checksum += final_dst[i];
    }
    
    printf("Final checksum: %d\n", final_checksum);
    
    return 0;
}
