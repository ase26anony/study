#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 42;

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

/* Greater-than (GT_EXPR) */
void test_gt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* Greater-than-or-equal (GE_EXPR) */
void test_ge_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            dst[i] = a[i] * 2 + c[i];
        } else {
            dst[i] = b[i] * 2 - c[i];
        }
    }
}

/* Less-than (LT_EXPR) */
void test_lt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            dst[i] = a[i] + b[i] + c[i];
        } else {
            dst[i] = a[i] - b[i] - c[i];
        }
    }
}

/* Less-than-or-equal (LE_EXPR) */
void test_le_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            dst[i] = (a[i] << 1) + c[i];
        } else {
            dst[i] = (b[i] << 1) - c[i];
        }
    }
}

/* Mixed data types: short integers */
void test_gt_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

void test_lt_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            dst[i] = a[i] * 3 + c[i];
        } else {
            dst[i] = b[i] * 3 - c[i];
        }
    }
}

/* Different loop length */
void test_ge_loop_small(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            dst[i] = a[i] + (c[i] >> 1);
        } else {
            dst[i] = b[i] - (c[i] >> 1);
        }
    }
}

void test_le_loop_small(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            dst[i] = (a[i] & 0xFF) + c[i];
        } else {
            dst[i] = (b[i] & 0xFF) - c[i];
        }
    }
}

int main() {
    /* Declare arrays with different sizes */
    int a[SIZE], b[SIZE], c[SIZE];
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    
    int a2[SIZE2], b2[SIZE2], c2[SIZE2];
    int dst5[SIZE2], dst6[SIZE2];
    
    short sa[SIZE], sb[SIZE], sc[SIZE];
    short sdst1[SIZE], sdst2[SIZE];
    
    /* Initialize all arrays */
    init_arrays(a, b, c, SIZE);
    init_arrays(a2, b2, c2, SIZE2);
    
    /* Initialize short arrays */
    int state = global_seed;
    for (int i = 0; i < SIZE; i++) {
        sa[i] = lcg_rand(&state) % 1000;
        sb[i] = lcg_rand(&state) % 1000;
        sc[i] = lcg_rand(&state) % 1000;
    }
    
    /* Test all comparison operators with int arrays */
    test_gt_loop(dst1, a, b, c, SIZE);
    test_ge_loop(dst2, a, b, c, SIZE);
    test_lt_loop(dst3, a, b, c, SIZE);
    test_le_loop(dst4, a, b, c, SIZE);
    
    /* Test with different loop length */
    test_ge_loop_small(dst5, a2, b2, c2, SIZE2);
    test_le_loop_small(dst6, a2, b2, c2, SIZE2);
    
    /* Test with short data type */
    test_gt_loop_short(sdst1, sa, sb, sc, SIZE);
    test_lt_loop_short(sdst2, sa, sb, sc, SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i];
        checksum += sdst1[i] + sdst2[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst5[i] + dst6[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
