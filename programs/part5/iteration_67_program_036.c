/* Program to trigger GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR transformations
 * in tree-vect-stmts.cc lines 12216-12233
 * Compile with: gcc -O3 -ftree-vectorize -fdump-tree-vect-details -march=native
 */

#include <stdio.h>
#include <stdint.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 42;

/* Simple PRNG for semi-random data */
static inline int simple_rand(int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

/* Initialize arrays with semi-random data */
void init_arrays(int *a, int *b, int *c, int size) {
    int seed = global_seed;
    for (int i = 0; i < size; i++) {
        a[i] = simple_rand(&seed) % 1000;
        b[i] = simple_rand(&seed) % 1000;
        c[i] = simple_rand(&seed) % 1000;
    }
}

/* Initialize short arrays */
void init_arrays_short(short *a, short *b, short *c, int size) {
    int seed = global_seed + 1;
    for (int i = 0; i < size; i++) {
        a[i] = simple_rand(&seed) % 1000;
        b[i] = simple_rand(&seed) % 1000;
        c[i] = simple_rand(&seed) % 1000;
    }
}

/* Test GT_EXPR transformation (greater-than) */
void test_gt_loop(int *dst, int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        /* This should generate GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* Test GE_EXPR transformation (greater-than-or-equal) */
void test_ge_loop(int *dst, int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        /* This should generate GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR */
        if (a[i] >= b[i]) {
            dst[i] = a[i] * 2 + c[i];
        } else {
            dst[i] = b[i] * 2 - c[i];
        }
    }
}

/* Test LT_EXPR transformation (less-than) */
void test_lt_loop(int *dst, int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        /* This should generate LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
        if (a[i] < b[i]) {
            dst[i] = a[i] + b[i] + c[i];
        } else {
            dst[i] = a[i] - b[i] - c[i];
        }
    }
}

/* Test LE_EXPR transformation (less-than-or-equal) */
void test_le_loop(int *dst, int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        /* This should generate LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
        if (a[i] <= b[i]) {
            dst[i] = a[i] * 3 + c[i];
        } else {
            dst[i] = b[i] * 3 - c[i];
        }
    }
}

/* Test with short data type for different mode conditions */
void test_gt_loop_short(short *dst, short *a, short *b, short *c, int size) {
    for (int i = 0; i < size; i++) {
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

void test_ge_loop_short(short *dst, short *a, short *b, short *c, int size) {
    for (int i = 0; i < size; i++) {
        if (a[i] >= b[i]) {
            dst[i] = a[i] * 2 + c[i];
        } else {
            dst[i] = b[i] * 2 - c[i];
        }
    }
}

void test_lt_loop_short(short *dst, short *a, short *b, short *c, int size) {
    for (int i = 0; i < size; i++) {
        if (a[i] < b[i]) {
            dst[i] = a[i] + b[i] + c[i];
        } else {
            dst[i] = a[i] - b[i] - c[i];
        }
    }
}

void test_le_loop_short(short *dst, short *a, short *b, short *c, int size) {
    for (int i = 0; i < size; i++) {
        if (a[i] <= b[i]) {
            dst[i] = a[i] * 3 + c[i];
        } else {
            dst[i] = b[i] * 3 - c[i];
        }
    }
}

/* Main function with checksum to prevent dead code elimination */
int main() {
    /* Declare arrays with different sizes */
    int a_int[SIZE], b_int[SIZE], c_int[SIZE];
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    
    short a_short[SIZE2], b_short[SIZE2], c_short[SIZE2];
    short dst1_short[SIZE2], dst2_short[SIZE2], dst3_short[SIZE2], dst4_short[SIZE2];
    
    /* Initialize arrays */
    init_arrays(a_int, b_int, c_int, SIZE);
    init_arrays_short(a_short, b_short, c_short, SIZE2);
    
    /* Test all comparison operators with int type */
    test_gt_loop(dst1, a_int, b_int, c_int, SIZE);
    test_ge_loop(dst2, a_int, b_int, c_int, SIZE);
    test_lt_loop(dst3, a_int, b_int, c_int, SIZE);
    test_le_loop(dst4, a_int, b_int, c_int, SIZE);
    
    /* Test all comparison operators with short type */
    test_gt_loop_short(dst1_short, a_short, b_short, c_short, SIZE2);
    test_ge_loop_short(dst2_short, a_short, b_short, c_short, SIZE2);
    test_lt_loop_short(dst3_short, a_short, b_short, c_short, SIZE2);
    test_le_loop_short(dst4_short, a_short, b_short, c_short, SIZE2);
    
    /* Compute checksum to prevent optimization and verify execution */
    uint64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst1_short[i] + dst2_short[i] + dst3_short[i] + dst4_short[i];
    }
    
    printf("Checksum: %lu\n", checksum);
    return 0;
}
