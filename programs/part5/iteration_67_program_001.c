#include <stdio.h>
#include <stdint.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 42;
volatile int use_short = 1;

/* Simple LCG for semi-random data */
static inline int lcg(int *state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

/* Initialize arrays with semi-random data */
void init_arrays(int *a, int *b, int *c, int n) {
    int state = global_seed;
    for (int i = 0; i < n; i++) {
        a[i] = lcg(&state) % 1000;
        b[i] = lcg(&state) % 1000;
        c[i] = lcg(&state) % 1000;
    }
}

void init_arrays_short(short *a, short *b, short *c, int n) {
    int state = global_seed + 1;
    for (int i = 0; i < n; i++) {
        a[i] = lcg(&state) % 1000;
        b[i] = lcg(&state) % 1000;
        c[i] = lcg(&state) % 1000;
    }
}

/* Test functions for each comparison operator */

/* GT_EXPR - greater than */
void test_gt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GT_EXPR internally */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* GE_EXPR - greater than or equal */
void test_ge_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GE_EXPR internally */
        if (a[i] >= b[i]) {
            dst[i] = a[i] * 2 + c[i];
        } else {
            dst[i] = b[i] * 2 - c[i];
        }
    }
}

/* LT_EXPR - less than */
void test_lt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LT_EXPR internally */
        if (a[i] < b[i]) {
            dst[i] = a[i] + b[i] + c[i];
        } else {
            dst[i] = a[i] - b[i] + c[i];
        }
    }
}

/* LE_EXPR - less than or equal */
void test_le_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LE_EXPR internally */
        if (a[i] <= b[i]) {
            dst[i] = a[i] * 3 + c[i];
        } else {
            dst[i] = b[i] * 3 - c[i];
        }
    }
}

/* Same tests with short type for different mode conditions */

void test_gt_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

void test_ge_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            dst[i] = a[i] * 2 + c[i];
        } else {
            dst[i] = b[i] * 2 - c[i];
        }
    }
}

void test_lt_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            dst[i] = a[i] + b[i] + c[i];
        } else {
            dst[i] = a[i] - b[i] + c[i];
        }
    }
}

void test_le_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            dst[i] = a[i] * 3 + c[i];
        } else {
            dst[i] = b[i] * 3 - c[i];
        }
    }
}

/* Mixed comparison in same loop to potentially trigger multiple patterns */
void test_mixed_comparisons(int *dst1, int *dst2, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Mix of different comparisons */
        if (a[i] > b[i]) {
            dst1[i] = a[i] + c[i];
        } else {
            dst1[i] = b[i];
        }
        
        if (a[i] <= c[i]) {
            dst2[i] = b[i] + c[i];
        } else {
            dst2[i] = a[i];
        }
    }
}

int main() {
    /* Declare arrays with different sizes */
    int a_int[SIZE], b_int[SIZE], c_int[SIZE];
    int dst1_int[SIZE], dst2_int[SIZE], dst3_int[SIZE], dst4_int[SIZE];
    int dst5_int[SIZE], dst6_int[SIZE];
    
    short a_short[SIZE2], b_short[SIZE2], c_short[SIZE2];
    short dst1_short[SIZE2], dst2_short[SIZE2], dst3_short[SIZE2], dst4_short[SIZE2];
    
    /* Initialize with semi-random data */
    init_arrays(a_int, b_int, c_int, SIZE);
    init_arrays_short(a_short, b_short, c_short, SIZE2);
    
    /* Test all comparison operators with int type */
    test_gt_loop(dst1_int, a_int, b_int, c_int, SIZE);
    test_ge_loop(dst2_int, a_int, b_int, c_int, SIZE);
    test_lt_loop(dst3_int, a_int, b_int, c_int, SIZE);
    test_le_loop(dst4_int, a_int, b_int, c_int, SIZE);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst5_int, dst6_int, a_int, b_int, c_int, SIZE);
    
    /* Test all comparison operators with short type */
    if (use_short) {
        test_gt_loop_short(dst1_short, a_short, b_short, c_short, SIZE2);
        test_ge_loop_short(dst2_short, a_short, b_short, c_short, SIZE2);
        test_lt_loop_short(dst3_short, a_short, b_short, c_short, SIZE2);
        test_le_loop_short(dst4_short, a_short, b_short, c_short, SIZE2);
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1_int[i] + dst2_int[i] + dst3_int[i] + dst4_int[i];
        checksum += dst5_int[i] + dst6_int[i];
    }
    
    if (use_short) {
        for (int i = 0; i < SIZE2; i++) {
            checksum += dst1_short[i] + dst2_short[i] + dst3_short[i] + dst4_short[i];
        }
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
