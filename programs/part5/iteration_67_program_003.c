#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 42;
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
void test_gt_loop(int *dst, int *src1, int *src2, int *src3, int n) {
    volatile int limit = n; /* Prevent loop unrolling before vectorization */
    for (int i = 0; i < limit; i++) {
        /* This should generate GT_EXPR in vectorized form */
        if (src1[i] > src2[i]) {
            dst[i] = src1[i] + src3[i];
        } else {
            dst[i] = src2[i] - src3[i];
        }
    }
}

/* Test function for GE_EXPR (>=) */
void test_ge_loop(short *dst, short *src1, short *src2, short *src3, int n) {
    volatile int limit = n;
    for (int i = 0; i < limit; i++) {
        /* This should generate GE_EXPR in vectorized form */
        if (src1[i] >= src2[i]) {
            dst[i] = src1[i] + src3[i];
        } else {
            dst[i] = src2[i] - src3[i];
        }
    }
}

/* Test function for LT_EXPR (<) */
void test_lt_loop(int *dst, int *src1, int *src2, int *src3, int n) {
    volatile int limit = n;
    for (int i = 0; i < limit; i++) {
        /* This should generate LT_EXPR in vectorized form */
        if (src1[i] < src2[i]) {
            dst[i] = src1[i] * src3[i];
        } else {
            dst[i] = src2[i] / (src3[i] + 1);
        }
    }
}

/* Test function for LE_EXPR (<=) */
void test_le_loop(short *dst, short *src1, short *src2, short *src3, int n) {
    volatile int limit = n;
    for (int i = 0; i < limit; i++) {
        /* This should generate LE_EXPR in vectorized form */
        if (src1[i] <= src2[i]) {
            dst[i] = src1[i] | src3[i];
        } else {
            dst[i] = src2[i] & src3[i];
        }
    }
}

/* Additional test with mixed comparisons in same loop */
void test_mixed_comparisons(int *dst1, int *dst2, int *src1, int *src2, int n) {
    volatile int limit = n;
    for (int i = 0; i < limit; i++) {
        /* Multiple comparisons to trigger different paths */
        if (src1[i] > src2[i]) {
            dst1[i] = src1[i] + 1;
        } else {
            dst1[i] = src2[i] - 1;
        }
        
        if (src1[i] <= src2[i]) {
            dst2[i] = src1[i] * 2;
        } else {
            dst2[i] = src2[i] / 2;
        }
    }
}

/* Test with different data widths */
void test_char_comparisons(char *dst, char *src1, char *src2, int n) {
    volatile int limit = n;
    for (int i = 0; i < limit; i++) {
        /* Using char type for different vectorization mode */
        if (src1[i] > src2[i]) {
            dst[i] = src1[i] + 10;
        } else if (src1[i] < src2[i]) {
            dst[i] = src2[i] - 10;
        } else {
            dst[i] = 0;
        }
    }
}

int main() {
    /* Allocate arrays with different sizes */
    int src1_int[SIZE], src2_int[SIZE], src3_int[SIZE];
    int dst1_int[SIZE], dst2_int[SIZE], dst3_int[SIZE], dst4_int[SIZE];
    
    short src1_short[SIZE2], src2_short[SIZE2], src3_short[SIZE2];
    short dst1_short[SIZE2], dst2_short[SIZE2];
    
    char src1_char[SIZE], src2_char[SIZE];
    char dst_char[SIZE];
    
    /* Initialize all arrays */
    init_arrays(src1_int, src2_int, src3_int, SIZE);
    
    /* Initialize short arrays */
    int state = global_seed;
    for (int i = 0; i < SIZE2; i++) {
        src1_short[i] = lcg_rand(&state) % 1000;
        src2_short[i] = lcg_rand(&state) % 1000;
        src3_short[i] = lcg_rand(&state) % 1000;
    }
    
    /* Initialize char arrays */
    state = global_seed;
    for (int i = 0; i < SIZE; i++) {
        src1_char[i] = lcg_rand(&state) % 128;
        src2_char[i] = lcg_rand(&state) % 128;
    }
    
    /* Execute all test functions to trigger different comparison operators */
    
    /* GT_EXPR case */
    test_gt_loop(dst1_int, src1_int, src2_int, src3_int, SIZE);
    
    /* GE_EXPR case with short type */
    test_ge_loop(dst1_short, src1_short, src2_short, src3_short, SIZE2);
    
    /* LT_EXPR case */
    test_lt_loop(dst2_int, src1_int, src2_int, src3_int, SIZE);
    
    /* LE_EXPR case with short type */
    test_le_loop(dst2_short, src1_short, src2_short, src3_short, SIZE2);
    
    /* Mixed comparisons */
    test_mixed_comparisons(dst3_int, dst4_int, src1_int, src2_int, SIZE);
    
    /* Char type comparisons */
    test_char_comparisons(dst_char, src1_char, src2_char, SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1_int[i] + dst2_int[i] + dst3_int[i] + dst4_int[i];
        checksum += dst_char[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst1_short[i] + dst2_short[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
