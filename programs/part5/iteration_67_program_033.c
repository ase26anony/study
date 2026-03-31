#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 12345;

/* Simple LCG for semi-random data */
static inline int lcg_rand(int *state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

/* Initialize arrays with semi-random data */
void init_arrays(int *a, int *b, int *c, int n) {
    int state = global_seed;
    for (int i = 0; i < n; i++) {
        a[i] = lcg_rand(&state) % 1000;
        b[i] = lcg_rand(&state) % 1000;
        c[i] = lcg_rand(&state) % 1000;
    }
}

/* Test functions for each comparison operator */

/* Greater-than (GT_EXPR) with int type */
void test_gt_int(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i];
        }
    }
}

/* Greater-than-or-equal (GE_EXPR) with short type */
void test_ge_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR */
        if (a[i] >= b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i];
        }
    }
}

/* Less-than (LT_EXPR) with int type, different length */
void test_lt_int(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
        if (a[i] < b[i]) {
            dst[i] = a[i] * c[i];
        } else {
            dst[i] = b[i] + 1;
        }
    }
}

/* Less-than-or-equal (LE_EXPR) with short type */
void test_le_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
        if (a[i] <= b[i]) {
            dst[i] = a[i] - c[i];
        } else {
            dst[i] = b[i];
        }
    }
}

/* Additional test with mixed operations to ensure coverage */
void test_mixed_comparisons(int *dst1, int *dst2, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Multiple comparisons in same loop */
        if (a[i] > b[i]) {
            dst1[i] = a[i] + 1;
        } else {
            dst1[i] = b[i] - 1;
        }
        
        if (a[i] <= c[i]) {
            dst2[i] = b[i] * 2;
        } else {
            dst2[i] = c[i];
        }
    }
}

/* Main function with checksum to prevent dead code elimination */
int main() {
    /* Declare arrays with different sizes */
    int a_int[SIZE], b_int[SIZE], c_int[SIZE];
    short a_short[SIZE2], b_short[SIZE2], c_short[SIZE2];
    
    int dst1[SIZE], dst2[SIZE], dst3[SIZE];
    short dst4[SIZE2], dst5[SIZE2];
    
    /* Initialize with semi-random data */
    init_arrays(a_int, b_int, c_int, SIZE);
    
    /* Initialize short arrays */
    int state = global_seed;
    for (int i = 0; i < SIZE2; i++) {
        a_short[i] = lcg_rand(&state) % 1000;
        b_short[i] = lcg_rand(&state) % 1000;
        c_short[i] = lcg_rand(&state) % 1000;
    }
    
    /* Call all test functions */
    test_gt_int(dst1, a_int, b_int, c_int, SIZE);
    test_ge_short(dst4, a_short, b_short, c_short, SIZE2);
    test_lt_int(dst2, a_int, b_int, c_int, SIZE);
    test_le_short(dst5, a_short, b_short, c_short, SIZE2);
    test_mixed_comparisons(dst3, dst1, a_int, b_int, c_int, SIZE);
    
    /* Compute checksum to prevent optimization */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i];
        if (i < SIZE2) {
            checksum += dst4[i] + dst5[i];
        }
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
