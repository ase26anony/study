/* Program to trigger vectorization of comparison operations
   targeting uncovered lines in tree-vect-stmts.cc (12216-12233) */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define SIZE2 128

/* Global arrays with volatile to prevent premature optimization */
volatile int global_seed = 42;

/* Simple PRNG to generate semi-random data */
static int simple_rand(int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

/* Initialize arrays with semi-random data */
void init_arrays(int *a, int *b, int *c, int size, int *seed) {
    for (int i = 0; i < size; i++) {
        a[i] = simple_rand(seed) % 1000;
        b[i] = simple_rand(seed) % 1000;
        c[i] = simple_rand(seed) % 1000;
    }
}

/* Test function for GT_EXPR (>) */
void test_gt_loop(int *dst, int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        /* This should generate GT_EXPR in vectorized form */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* Test function for GE_EXPR (>=) */
void test_ge_loop(short *dst, short *a, short *b, short *c, int size) {
    for (int i = 0; i < size; i++) {
        /* This should generate GE_EXPR in vectorized form */
        if (a[i] >= b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* Test function for LT_EXPR (<) */
void test_lt_loop(int *dst, int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        /* This should generate LT_EXPR in vectorized form */
        if (a[i] < b[i]) {
            dst[i] = a[i] * c[i];
        } else {
            dst[i] = b[i] + c[i];
        }
    }
}

/* Test function for LE_EXPR (<=) */
void test_le_loop(short *dst, short *a, short *b, short *c, int size) {
    for (int i = 0; i < size; i++) {
        /* This should generate LE_EXPR in vectorized form */
        if (a[i] <= b[i]) {
            dst[i] = a[i] | c[i];
        } else {
            dst[i] = b[i] & c[i];
        }
    }
}

/* Additional test with mixed comparisons */
void test_mixed_comparisons(int *dst1, int *dst2, int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        /* Multiple comparisons in same loop */
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

/* Test with different integer widths */
void test_char_comparisons(char *dst, char *a, char *b, char *c, int size) {
    for (int i = 0; i < size; i++) {
        /* Using char type for different vectorization modes */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

int main() {
    int seed = global_seed;
    
    /* Allocate arrays with different sizes */
    int a_int[SIZE], b_int[SIZE], c_int[SIZE];
    short a_short[SIZE2], b_short[SIZE2], c_short[SIZE2];
    char a_char[SIZE], b_char[SIZE], c_char[SIZE];
    
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    short dst_short1[SIZE2], dst_short2[SIZE2];
    char dst_char[SIZE];
    
    /* Initialize all arrays */
    init_arrays(a_int, b_int, c_int, SIZE, &seed);
    
    for (int i = 0; i < SIZE2; i++) {
        a_short[i] = simple_rand(&seed) % 1000;
        b_short[i] = simple_rand(&seed) % 1000;
        c_short[i] = simple_rand(&seed) % 1000;
    }
    
    for (int i = 0; i < SIZE; i++) {
        a_char[i] = simple_rand(&seed) % 128;
        b_char[i] = simple_rand(&seed) % 128;
        c_char[i] = simple_rand(&seed) % 128;
    }
    
    /* Call all test functions to trigger different comparison operators */
    
    /* GT_EXPR (>) with int */
    test_gt_loop(dst1, a_int, b_int, c_int, SIZE);
    
    /* GE_EXPR (>=) with short */
    test_ge_loop(dst_short1, a_short, b_short, c_short, SIZE2);
    
    /* LT_EXPR (<) with int */
    test_lt_loop(dst2, a_int, b_int, c_int, SIZE);
    
    /* LE_EXPR (<=) with short */
    test_le_loop(dst_short2, a_short, b_short, c_short, SIZE2);
    
    /* Mixed comparisons */
    test_mixed_comparisons(dst3, dst4, a_int, b_int, c_int, SIZE);
    
    /* Char comparisons */
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
