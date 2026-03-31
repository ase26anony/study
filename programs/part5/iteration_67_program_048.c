/* Program to trigger vectorization of comparison operations for GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR */
#include <stdio.h>
#include <stdint.h>

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

/* Test function for GE_EXPR (greater-than-or-equal) */
void test_ge_loop(int *dst, int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        /* This should generate GE_EXPR in vectorized form */
        if (a[i] >= b[i]) {
            dst[i] = a[i] * 2 + c[i];
        } else {
            dst[i] = b[i] * 2 - c[i];
        }
    }
}

/* Test function for LT_EXPR (less-than) */
void test_lt_loop(int *dst, int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        /* This should generate LT_EXPR in vectorized form */
        if (a[i] < b[i]) {
            dst[i] = a[i] + b[i] + c[i];
        } else {
            dst[i] = a[i] - b[i] - c[i];
        }
    }
}

/* Test function for LE_EXPR (less-than-or-equal) */
void test_le_loop(int *dst, int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        /* This should generate LE_EXPR in vectorized form */
        if (a[i] <= b[i]) {
            dst[i] = (a[i] << 1) + c[i];
        } else {
            dst[i] = (b[i] << 1) - c[i];
        }
    }
}

/* Same tests with short data type for different vectorization modes */
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
            dst[i] = a[i] * 3 + c[i];
        } else {
            dst[i] = b[i] * 3 - c[i];
        }
    }
}

void test_lt_loop_short(short *dst, short *a, short *b, short *c, int size) {
    for (int i = 0; i < size; i++) {
        if (a[i] < b[i]) {
            dst[i] = a[i] + c[i] * 2;
        } else {
            dst[i] = b[i] - c[i] * 2;
        }
    }
}

void test_le_loop_short(short *dst, short *a, short *b, short *c, int size) {
    for (int i = 0; i < size; i++) {
        if (a[i] <= b[i]) {
            dst[i] = a[i] + b[i] + c[i];
        } else {
            dst[i] = a[i] - b[i] - c[i];
        }
    }
}

/* Additional test with mixed comparisons in same loop */
void test_mixed_comparisons(int *dst1, int *dst2, int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        /* Mix GT and LT in same loop */
        if (a[i] > b[i]) {
            dst1[i] = a[i] + 1;
        } else {
            dst1[i] = b[i] - 1;
        }
        
        if (a[i] < c[i]) {
            dst2[i] = a[i] * 2;
        } else {
            dst2[i] = c[i] * 2;
        }
    }
}

/* Compute checksum to prevent dead code elimination */
unsigned long long compute_checksum(int *arr, int size) {
    unsigned long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum += (unsigned)arr[i];
    }
    return sum;
}

unsigned long long compute_checksum_short(short *arr, int size) {
    unsigned long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum += (unsigned short)arr[i];
    }
    return sum;
}

int main() {
    /* Declare arrays with different sizes to test various vectorization factors */
    int src1[SIZE], src2[SIZE], src3[SIZE];
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    int dst5[SIZE], dst6[SIZE];
    
    short src1_short[SIZE2], src2_short[SIZE2], src3_short[SIZE2];
    short dst1_short[SIZE2], dst2_short[SIZE2], dst3_short[SIZE2], dst4_short[SIZE2];
    
    /* Initialize arrays with semi-random data */
    init_arrays(src1, src2, src3, SIZE);
    init_arrays_short(src1_short, src2_short, src3_short, SIZE2);
    
    /* Test all four comparison operators with int type */
    test_gt_loop(dst1, src1, src2, src3, SIZE);
    test_ge_loop(dst2, src1, src2, src3, SIZE);
    test_lt_loop(dst3, src1, src2, src3, SIZE);
    test_le_loop(dst4, src1, src2, src3, SIZE);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst5, dst6, src1, src2, src3, SIZE);
    
    /* Test all four comparison operators with short type */
    test_gt_loop_short(dst1_short, src1_short, src2_short, src3_short, SIZE2);
    test_ge_loop_short(dst2_short, src1_short, src2_short, src3_short, SIZE2);
    test_lt_loop_short(dst3_short, src1_short, src2_short, src3_short, SIZE2);
    test_le_loop_short(dst4_short, src1_short, src2_short, src3_short, SIZE2);
    
    /* Compute checksums to prevent dead code elimination */
    unsigned long long checksum = 0;
    checksum += compute_checksum(dst1, SIZE);
    checksum += compute_checksum(dst2, SIZE);
    checksum += compute_checksum(dst3, SIZE);
    checksum += compute_checksum(dst4, SIZE);
    checksum += compute_checksum(dst5, SIZE);
    checksum += compute_checksum(dst6, SIZE);
    checksum += compute_checksum_short(dst1_short, SIZE2);
    checksum += compute_checksum_short(dst2_short, SIZE2);
    checksum += compute_checksum_short(dst3_short, SIZE2);
    checksum += compute_checksum_short(dst4_short, SIZE2);
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
