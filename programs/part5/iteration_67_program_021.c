/* Program to trigger vectorization of comparison operations
   targeting uncovered lines in tree-vect-stmts.cc (12216-12233) */

#include <stdio.h>
#include <stdint.h>

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

/* Test function for GT_EXPR (>) */
void test_gt_loop(int *dst, int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        /* This should generate GT_EXPR in GIMPLE */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* Test function for GE_EXPR (>=) */
void test_ge_loop(short *dst, short *a, short *b, short *c, int size) {
    /* Different data type (short) to trigger different modes */
    for (int i = 0; i < size; i++) {
        /* This should generate GE_EXPR in GIMPLE */
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
        /* This should generate LT_EXPR in GIMPLE */
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
        /* This should generate LE_EXPR in GIMPLE */
        if (a[i] <= b[i]) {
            dst[i] = a[i] - c[i];
        } else {
            dst[i] = b[i] * c[i];
        }
    }
}

/* Additional test with mixed comparisons in same loop */
void test_mixed_comparisons(int *dst1, int *dst2, int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        /* Mix of different comparisons */
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

/* Test with volatile inputs to prevent optimization */
void test_volatile_gt(volatile int *dst, volatile int *a, volatile int *b, int size) {
    for (int i = 0; i < size; i++) {
        if (a[i] > b[i]) {
            dst[i] = a[i] + b[i];
        } else {
            dst[i] = a[i] - b[i];
        }
    }
}

int main() {
    /* Declare arrays with different sizes */
    int src1[SIZE], src2[SIZE], src3[SIZE];
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    
    short ssrc1[SIZE2], ssrc2[SIZE2], ssrc3[SIZE2];
    short sdst1[SIZE2], sdst2[SIZE2];
    
    volatile int v_src1[SIZE], v_src2[SIZE];
    volatile int v_dst[SIZE];
    
    /* Initialize with semi-random data */
    init_arrays(src1, src2, src3, SIZE);
    
    /* Initialize short arrays */
    int state = global_seed + 1;
    for (int i = 0; i < SIZE2; i++) {
        ssrc1[i] = lcg_rand(&state) % 1000;
        ssrc2[i] = lcg_rand(&state) % 1000;
        ssrc3[i] = lcg_rand(&state) % 1000;
    }
    
    /* Initialize volatile arrays */
    state = global_seed + 2;
    for (int i = 0; i < SIZE; i++) {
        v_src1[i] = lcg_rand(&state) % 1000;
        v_src2[i] = lcg_rand(&state) % 1000;
    }
    
    /* Test all comparison operators */
    test_gt_loop(dst1, src1, src2, src3, SIZE);      /* >  operator */
    test_ge_loop(sdst1, ssrc1, ssrc2, ssrc3, SIZE2); /* >= operator */
    test_lt_loop(dst2, src1, src2, src3, SIZE);      /* <  operator */
    test_le_loop(sdst2, ssrc1, ssrc2, ssrc3, SIZE2); /* <= operator */
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst3, dst4, src1, src2, src3, SIZE);
    
    /* Test with volatile to prevent optimization */
    if (use_volatile) {
        test_volatile_gt(v_dst, v_src1, v_src2, SIZE);
    }
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i];
        if (i < SIZE2) {
            checksum += sdst1[i] + sdst2[i];
        }
        if (use_volatile && i < SIZE) {
            checksum += v_dst[i];
        }
    }
    
    printf("Checksum: %lu\n", (unsigned long)checksum);
    
    return 0;
}
