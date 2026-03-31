#include <stdio.h>
#include <stdint.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 42;
volatile int use_volatile = 1;

/* Simple LCG for semi-random data */
static inline int lcg_next(int *state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

/* Initialize arrays with semi-random data */
void init_arrays(int *a, int *b, int *c, int size) {
    int state = global_seed;
    for (int i = 0; i < size; i++) {
        a[i] = lcg_next(&state) % 1000;
        b[i] = lcg_next(&state) % 1000;
        c[i] = lcg_next(&state) % 1000;
    }
}

/* Test GT_EXPR transformation */
void test_gt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* Test GE_EXPR transformation */
void test_ge_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR */
        if (a[i] >= b[i]) {
            dst[i] = a[i] * 2 + c[i];
        } else {
            dst[i] = b[i] * 2 - c[i];
        }
    }
}

/* Test LT_EXPR transformation */
void test_lt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
        if (a[i] < b[i]) {
            dst[i] = a[i] + b[i] + c[i];
        } else {
            dst[i] = a[i] - b[i] - c[i];
        }
    }
}

/* Test LE_EXPR transformation */
void test_le_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
        if (a[i] <= b[i]) {
            dst[i] = a[i] * 3 + c[i];
        } else {
            dst[i] = b[i] * 3 - c[i];
        }
    }
}

/* Test with short integers (different data width) */
void test_gt_loop_short(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* Test with mixed comparisons in same loop */
void test_mixed_comparisons(int *dst1, int *dst2, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Mix GT and LT in same loop body */
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

/* Test with volatile inputs to prevent optimization */
void test_ge_loop_volatile(volatile int *dst, volatile int *a, volatile int *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            dst[i] = a[i] * 2;
        } else {
            dst[i] = b[i] / 2;
        }
    }
}

int main() {
    /* Declare arrays with different sizes */
    int src1[SIZE], src2[SIZE], src3[SIZE];
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    
    short src1_short[SIZE2], src2_short[SIZE2], src3_short[SIZE2];
    short dst_short[SIZE2];
    
    volatile int volatile_src1[SIZE], volatile_src2[SIZE];
    volatile int volatile_dst[SIZE];
    
    /* Initialize with semi-random data */
    init_arrays(src1, src2, src3, SIZE);
    
    /* Initialize short arrays */
    int state = global_seed;
    for (int i = 0; i < SIZE2; i++) {
        src1_short[i] = lcg_next(&state) % 1000;
        src2_short[i] = lcg_next(&state) % 1000;
        src3_short[i] = lcg_next(&state) % 1000;
    }
    
    /* Initialize volatile arrays */
    state = global_seed + 1;
    for (int i = 0; i < SIZE; i++) {
        volatile_src1[i] = lcg_next(&state) % 1000;
        volatile_src2[i] = lcg_next(&state) % 1000;
    }
    
    /* Test all comparison operators with different array sizes */
    test_gt_loop(dst1, src1, src2, src3, SIZE);
    test_ge_loop(dst2, src1, src2, src3, SIZE);
    test_lt_loop(dst3, src1, src2, src3, SIZE);
    test_le_loop(dst4, src1, src2, src3, SIZE);
    
    /* Test with different data type */
    test_gt_loop_short(dst_short, src1_short, src2_short, src3_short, SIZE2);
    
    /* Test mixed comparisons */
    int mixed_dst1[SIZE], mixed_dst2[SIZE];
    test_mixed_comparisons(mixed_dst1, mixed_dst2, src1, src2, src3, SIZE);
    
    /* Test with volatile qualifier */
    if (use_volatile) {
        test_ge_loop_volatile(volatile_dst, volatile_src1, volatile_src2, SIZE);
    }
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i];
        if (i < SIZE) {
            checksum += mixed_dst1[i] + mixed_dst2[i];
        }
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst_short[i];
    }
    
    printf("Checksum: %lu\n", (unsigned long)checksum);
    
    return 0;
}
