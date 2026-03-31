#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 42;
volatile int use_volatile = 1;

/* Simple LCG for semi-random values */
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

/* Initialize short arrays */
void init_arrays_short(short *a, short *b, short *c, int size) {
    int state = global_seed + 1;
    for (int i = 0; i < size; i++) {
        a[i] = lcg_rand(&state) % 1000;
        b[i] = lcg_rand(&state) % 1000;
        c[i] = lcg_rand(&state) % 1000;
    }
}

/* Test function for GT_EXPR (greater-than) */
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

/* Test function for GE_EXPR (greater-than-or-equal) */
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

/* Test function for LT_EXPR (less-than) */
void test_lt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LT_EXPR internally */
        if (a[i] < b[i]) {
            dst[i] = a[i] + b[i] + c[i];
        } else {
            dst[i] = a[i] - b[i] - c[i];
        }
    }
}

/* Test function for LE_EXPR (less-than-or-equal) */
void test_le_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LE_EXPR internally */
        if (a[i] <= b[i]) {
            dst[i] = (a[i] << 1) + c[i];
        } else {
            dst[i] = (b[i] << 1) - c[i];
        }
    }
}

/* Test with short data type for GT_EXPR */
void test_gt_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* Test with short data type for GE_EXPR */
void test_ge_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            dst[i] = a[i] * 3 + c[i];
        } else {
            dst[i] = b[i] * 3 - c[i];
        }
    }
}

/* Test with short data type for LT_EXPR */
void test_lt_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            dst[i] = a[i] + c[i] * 2;
        } else {
            dst[i] = b[i] - c[i] * 2;
        }
    }
}

/* Test with short data type for LE_EXPR */
void test_le_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            dst[i] = a[i] + b[i] + c[i];
        } else {
            dst[i] = a[i] - b[i] - c[i];
        }
    }
}

/* Mixed comparison in same loop */
void test_mixed_comparisons(int *dst1, int *dst2, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Mix GT and LT comparisons */
        if (a[i] > b[i]) {
            dst1[i] = a[i] + c[i];
        } else {
            dst1[i] = b[i];
        }
        
        if (a[i] < c[i]) {
            dst2[i] = b[i] + c[i];
        } else {
            dst2[i] = a[i];
        }
    }
}

int main() {
    /* Declare arrays with different sizes */
    int a_int[SIZE], b_int[SIZE], c_int[SIZE];
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    int dst5[SIZE2], dst6[SIZE2];
    
    short a_short[SIZE], b_short[SIZE], c_short[SIZE];
    short dst_short1[SIZE], dst_short2[SIZE], dst_short3[SIZE], dst_short4[SIZE];
    
    /* Initialize arrays */
    init_arrays(a_int, b_int, c_int, SIZE);
    init_arrays(dst5, dst6, a_int, SIZE2); /* Reuse for smaller arrays */
    init_arrays_short(a_short, b_short, c_short, SIZE);
    
    /* Test all comparison operators with int type */
    test_gt_loop(dst1, a_int, b_int, c_int, SIZE);
    test_ge_loop(dst2, a_int, b_int, c_int, SIZE);
    test_lt_loop(dst3, a_int, b_int, c_int, SIZE);
    test_le_loop(dst4, a_int, b_int, c_int, SIZE);
    
    /* Test all comparison operators with short type */
    test_gt_loop_short(dst_short1, a_short, b_short, c_short, SIZE);
    test_ge_loop_short(dst_short2, a_short, b_short, c_short, SIZE);
    test_lt_loop_short(dst_short3, a_short, b_short, c_short, SIZE);
    test_le_loop_short(dst_short4, a_short, b_short, c_short, SIZE);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst5, dst6, a_int, b_int, c_int, SIZE2);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i];
        checksum += dst_short1[i] + dst_short2[i] + dst_short3[i] + dst_short4[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst5[i] + dst6[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
