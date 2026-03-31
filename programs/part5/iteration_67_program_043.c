#include <stdio.h>
#include <stdlib.h>

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
void test_gt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GT_EXPR in GIMPLE */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* Test function for GE_EXPR (>=) */
void test_ge_loop(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GE_EXPR in GIMPLE */
        if (a[i] >= b[i]) {
            dst[i] = (short)(a[i] + c[i]);
        } else {
            dst[i] = (short)(b[i] - c[i]);
        }
    }
}

/* Test function for LT_EXPR (<) */
void test_lt_loop(int *dst, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LT_EXPR in GIMPLE */
        if (a[i] < b[i]) {
            dst[i] = a[i] * c[i];
        } else {
            dst[i] = b[i] + c[i];
        }
    }
}

/* Test function for LE_EXPR (<=) */
void test_le_loop(short *dst, short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LE_EXPR in GIMPLE */
        if (a[i] <= b[i]) {
            dst[i] = (short)(a[i] - c[i]);
        } else {
            dst[i] = (short)(b[i] * c[i]);
        }
    }
}

/* Additional test with mixed comparisons in same loop */
void test_mixed_comparisons(int *dst1, int *dst2, int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Mix of different comparison operators */
        if (a[i] > b[i]) {
            dst1[i] = a[i] + 1;
        } else {
            dst1[i] = b[i] - 1;
        }
        
        if (a[i] <= c[i]) {
            dst2[i] = b[i] * 2;
        } else {
            dst2[i] = c[i] / 2;
        }
    }
}

/* Test with different loop lengths and unroll hints */
void test_various_lengths(int *dst, int *a, int *b, int n) {
    /* Multiple loops with different characteristics */
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            dst[i] = a[i] << 1;
        } else {
            dst[i] = b[i] >> 1;
        }
    }
    
    /* Smaller loop */
    for (int i = 0; i < 64; i++) {
        if (a[i] < b[i]) {
            dst[i + n] = a[i] | 0xFF;
        } else {
            dst[i + n] = b[i] & 0xFF;
        }
    }
}

int main() {
    /* Declare arrays with different sizes */
    int a_int[SIZE], b_int[SIZE], c_int[SIZE];
    short a_short[SIZE2], b_short[SIZE2], c_short[SIZE2];
    
    int dst1[SIZE], dst2[SIZE], dst3[SIZE];
    short dst_short1[SIZE2], dst_short2[SIZE2];
    int dst_mixed1[SIZE], dst_mixed2[SIZE];
    
    /* Initialize with semi-random data */
    init_arrays(a_int, b_int, c_int, SIZE);
    
    /* Initialize short arrays */
    int state = global_seed;
    for (int i = 0; i < SIZE2; i++) {
        a_short[i] = (short)(lcg_rand(&state) % 1000);
        b_short[i] = (short)(lcg_rand(&state) % 1000);
        c_short[i] = (short)(lcg_rand(&state) % 1000);
    }
    
    /* Force compiler to not optimize away initialization */
    if (use_volatile) {
        /* Dummy volatile read to prevent dead code elimination */
        volatile int dummy = a_int[0] + b_int[0];
        (void)dummy;
    }
    
    /* Test all comparison operators */
    test_gt_loop(dst1, a_int, b_int, c_int, SIZE);
    test_ge_loop(dst_short1, a_short, b_short, c_short, SIZE2);
    test_lt_loop(dst2, a_int, b_int, c_int, SIZE);
    test_le_loop(dst_short2, a_short, b_short, c_short, SIZE2);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst_mixed1, dst_mixed2, a_int, b_int, c_int, SIZE);
    
    /* Test various loop lengths */
    test_various_lengths(dst3, a_int, b_int, SIZE);
    
    /* Compute checksum to prevent optimization and verify execution */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst_mixed1[i] + dst_mixed2[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst_short1[i] + dst_short2[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
