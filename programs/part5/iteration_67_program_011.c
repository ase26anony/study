#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define SIZE2 128

/* Prevent constant propagation */
volatile int global_seed = 42;

/* Simple LCG for semi-random data */
static int lcg(int *state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

/* Initialize arrays with semi-random data */
void init_arrays(int *a, int *b, int *c, int size) {
    int state = global_seed;
    for (int i = 0; i < size; i++) {
        a[i] = lcg(&state) % 1000;
        b[i] = lcg(&state) % 1000;
        c[i] = lcg(&state) % 1000;
    }
}

/* Test GT_EXPR transformation */
void test_gt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i];
        }
    }
}

/* Test GE_EXPR transformation */
void test_ge_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            dst[i] = a[i] - c[i];
        } else {
            dst[i] = b[i] + 1;
        }
    }
}

/* Test LT_EXPR transformation */
void test_lt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            dst[i] = a[i] * 2;
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* Test LE_EXPR transformation */
void test_le_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            dst[i] = a[i] + b[i];
        } else {
            dst[i] = c[i];
        }
    }
}

/* Mixed data type tests - using short to trigger different modes */
void test_gt_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            dst[i] = (short)(a[i] + c[i]);
        } else {
            dst[i] = b[i];
        }
    }
}

void test_ge_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            dst[i] = (short)(a[i] - c[i]);
        } else {
            dst[i] = (short)(b[i] + 1);
        }
    }
}

void test_lt_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            dst[i] = (short)(a[i] * 2);
        } else {
            dst[i] = (short)(b[i] - c[i]);
        }
    }
}

void test_le_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            dst[i] = (short)(a[i] + b[i]);
        } else {
            dst[i] = c[i];
        }
    }
}

/* Additional test with different loop length */
void test_mixed_comparisons(int *dst, int *a, int *b, int *c, int n) {
    /* Mix of comparisons in one loop - vectorizer may handle differently */
    for (int i = 0; i < n; i++) {
        if (i % 4 == 0) {
            if (a[i] > b[i]) dst[i] = a[i];
        } else if (i % 4 == 1) {
            if (a[i] >= b[i]) dst[i] = b[i];
        } else if (i % 4 == 2) {
            if (a[i] < b[i]) dst[i] = c[i];
        } else {
            if (a[i] <= b[i]) dst[i] = a[i] + b[i] + c[i];
        }
    }
}

int main() {
    /* Declare arrays with different sizes */
    int a_int[SIZE], b_int[SIZE], c_int[SIZE];
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    int dst_mixed[SIZE];
    
    short a_short[SIZE2], b_short[SIZE2], c_short[SIZE2];
    short dst1_short[SIZE2], dst2_short[SIZE2], dst3_short[SIZE2], dst4_short[SIZE2];
    
    /* Initialize with semi-random data */
    init_arrays(a_int, b_int, c_int, SIZE);
    
    /* Initialize short arrays */
    int state = global_seed + 1;
    for (int i = 0; i < SIZE2; i++) {
        a_short[i] = (short)(lcg(&state) % 1000);
        b_short[i] = (short)(lcg(&state) % 1000);
        c_short[i] = (short)(lcg(&state) % 1000);
    }
    
    /* Test all comparison operators with int type */
    test_gt_loop(dst1, a_int, b_int, c_int, SIZE);
    test_ge_loop(dst2, a_int, b_int, c_int, SIZE);
    test_lt_loop(dst3, a_int, b_int, c_int, SIZE);
    test_le_loop(dst4, a_int, b_int, c_int, SIZE);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst_mixed, a_int, b_int, c_int, SIZE);
    
    /* Test all comparison operators with short type */
    test_gt_loop_short(dst1_short, a_short, b_short, c_short, SIZE2);
    test_ge_loop_short(dst2_short, a_short, b_short, c_short, SIZE2);
    test_lt_loop_short(dst3_short, a_short, b_short, c_short, SIZE2);
    test_le_loop_short(dst4_short, a_short, b_short, c_short, SIZE2);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i] + dst_mixed[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst1_short[i] + dst2_short[i] + dst3_short[i] + dst4_short[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
