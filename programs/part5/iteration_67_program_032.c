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
void test_ge_loop(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GE_EXPR comparison */
        if (a[i] >= b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
void test_lt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LT_EXPR comparison */
        if (a[i] < b[i]) {
            dst[i] = a[i] * c[i];
        } else {
            dst[i] = b[i] + c[i];
        }
    }
}

/* LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
void test_le_loop(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LE_EXPR comparison */
        if (a[i] <= b[i]) {
            dst[i] = a[i] - c[i];
        } else {
            dst[i] = b[i] * c[i];
        }
    }
}

/* Additional test with mixed comparisons in same loop */
void test_mixed_comparisons(int *dst1, int *dst2, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Mix GT and LT comparisons */
        if (a[i] > b[i]) {
            dst1[i] = a[i] + 1;
        } else {
            dst1[i] = b[i] - 1;
        }
        
        if (a[i] < c[i]) {
            dst2[i] = a[i] * 2;
        } else {
            dst2[i] = c[i] / 2;
        }
    }
}

/* Test with different data widths */
void test_char_comparisons(char *dst, char *a, char *b, char *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Use all comparison operators with char type */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else if (a[i] <= c[i]) {
            dst[i] = b[i] - c[i];
        } else {
            dst[i] = a[i] * b[i];
        }
    }
}

int main() {
    /* Declare arrays with different sizes and types */
    int a_int[SIZE], b_int[SIZE], c_int[SIZE];
    short a_short[SIZE2], b_short[SIZE2], c_short[SIZE2];
    char a_char[SIZE], b_char[SIZE], c_char[SIZE];
    
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    short dst_short1[SIZE2], dst_short2[SIZE2];
    char dst_char[SIZE];
    
    /* Initialize all arrays */
    init_arrays(a_int, b_int, c_int, SIZE);
    
    /* Initialize short arrays */
    int state = global_seed;
    for (int i = 0; i < SIZE2; i++) {
        a_short[i] = lcg_rand(&state) % 1000;
        b_short[i] = lcg_rand(&state) % 1000;
        c_short[i] = lcg_rand(&state) % 1000;
    }
    
    /* Initialize char arrays */
    state = global_seed;
    for (int i = 0; i < SIZE; i++) {
        a_char[i] = lcg_rand(&state) % 128;
        b_char[i] = lcg_rand(&state) % 128;
        c_char[i] = lcg_rand(&state) % 128;
    }
    
    /* Call all test functions to trigger vectorization patterns */
    
    /* Test GT_EXPR with int arrays */
    test_gt_loop(dst1, a_int, b_int, c_int, SIZE);
    
    /* Test GE_EXPR with short arrays */
    test_ge_loop(dst_short1, a_short, b_short, c_short, SIZE2);
    
    /* Test LT_EXPR with int arrays */
    test_lt_loop(dst2, a_int, b_int, c_int, SIZE);
    
    /* Test LE_EXPR with short arrays */
    test_le_loop(dst_short2, a_short, b_short, c_short, SIZE2);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst3, dst4, a_int, b_int, c_int, SIZE);
    
    /* Test with char type */
    test_char_comparisons(dst_char, a_char, b_char, c_char, SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i] + dst_char[i];
    }
    
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst_short1[i] + dst_short2[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
