#include <stdio.h>
#include <stdint.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 42;
volatile int use_volatile = 1;

/* Simple LCG for semi-random data */
static inline int lcg(int *state) {
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

void init_short_arrays(short *a, short *b, short *c, int size) {
    int state = global_seed + 1;
    for (int i = 0; i < size; i++) {
        a[i] = lcg(&state) % 1000;
        b[i] = lcg(&state) % 1000;
        c[i] = lcg(&state) % 1000;
    }
}

/* Test functions for each comparison operator */

/* GT_EXPR case */
void test_gt_loop(int *dst, int *src1, int *src2, int *src3, int n) {
    volatile int limit = n;  /* Prevent optimization */
    for (int i = 0; i < limit; i++) {
        /* Generate mask from GT comparison */
        if (src1[i] > src2[i]) {
            dst[i] = src1[i] + src3[i];
        } else {
            dst[i] = src2[i] - src3[i];
        }
    }
}

/* GE_EXPR case */
void test_ge_loop(int *dst, int *src1, int *src2, int *src3, int n) {
    volatile int limit = n;
    for (int i = 0; i < limit; i++) {
        /* Generate mask from GE comparison */
        if (src1[i] >= src2[i]) {
            dst[i] = src1[i] * 2 + src3[i];
        } else {
            dst[i] = src2[i] * 3 - src3[i];
        }
    }
}

/* LT_EXPR case */
void test_lt_loop(int *dst, int *src1, int *src2, int *src3, int n) {
    volatile int limit = n;
    for (int i = 0; i < limit; i++) {
        /* Generate mask from LT comparison */
        if (src1[i] < src2[i]) {
            dst[i] = src1[i] + src2[i] + src3[i];
        } else {
            dst[i] = src2[i] - src1[i] + src3[i];
        }
    }
}

/* LE_EXPR case */
void test_le_loop(int *dst, int *src1, int *src2, int *src3, int n) {
    volatile int limit = n;
    for (int i = 0; i < limit; i++) {
        /* Generate mask from LE comparison */
        if (src1[i] <= src2[i]) {
            dst[i] = src1[i] * src3[i];
        } else {
            dst[i] = src2[i] + src3[i];
        }
    }
}

/* Mixed data types - short arrays */
void test_gt_short(short *dst, short *src1, short *src2, short *src3, int n) {
    volatile int limit = n;
    for (int i = 0; i < limit; i++) {
        /* GT comparison on short type */
        if (src1[i] > src2[i]) {
            dst[i] = src1[i] + src3[i];
        } else {
            dst[i] = src2[i] - src3[i];
        }
    }
}

void test_le_short(short *dst, short *src1, short *src2, short *src3, int n) {
    volatile int limit = n;
    for (int i = 0; i < limit; i++) {
        /* LE comparison on short type */
        if (src1[i] <= src2[i]) {
            dst[i] = src1[i] * 2;
        } else {
            dst[i] = src2[i] * 3;
        }
    }
}

/* Different loop lengths */
void test_ge_small(int *dst, int *src1, int *src2, int *src3, int n) {
    volatile int limit = n;
    for (int i = 0; i < limit; i++) {
        /* GE comparison in smaller loop */
        if (src1[i] >= src2[i]) {
            dst[i] = (src1[i] << 1) | src3[i];
        } else {
            dst[i] = (src2[i] >> 1) & src3[i];
        }
    }
}

void test_lt_medium(int *dst, int *src1, int *src2, int *src3, int n) {
    volatile int limit = n;
    for (int i = 0; i < limit; i++) {
        /* LT comparison in medium loop */
        if (src1[i] < src2[i]) {
            dst[i] = src1[i] ^ src3[i];
        } else {
            dst[i] = src2[i] | src3[i];
        }
    }
}

int main() {
    /* Declare arrays */
    int src1[SIZE], src2[SIZE], src3[SIZE];
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    
    short s_src1[SIZE2], s_src2[SIZE2], s_src3[SIZE2];
    short s_dst1[SIZE2], s_dst2[SIZE2];
    
    int small_src1[64], small_src2[64], small_src3[64];
    int small_dst[64];
    
    int medium_src1[192], medium_src2[192], medium_src3[192];
    int medium_dst[192];
    
    /* Initialize all arrays */
    init_arrays(src1, src2, src3, SIZE);
    init_arrays(small_src1, small_src2, small_src3, 64);
    init_arrays(medium_src1, medium_src2, medium_src3, 192);
    init_short_arrays(s_src1, s_src2, s_src3, SIZE2);
    
    /* Execute all test functions to trigger different comparison patterns */
    
    /* GT_EXPR - int arrays, size 256 */
    test_gt_loop(dst1, src1, src2, src3, SIZE);
    
    /* GE_EXPR - int arrays, size 256 */
    test_ge_loop(dst2, src1, src2, src3, SIZE);
    
    /* LT_EXPR - int arrays, size 256 */
    test_lt_loop(dst3, src1, src2, src3, SIZE);
    
    /* LE_EXPR - int arrays, size 256 */
    test_le_loop(dst4, src1, src2, src3, SIZE);
    
    /* GT_EXPR - short arrays, size 128 */
    test_gt_short(s_dst1, s_src1, s_src2, s_src3, SIZE2);
    
    /* LE_EXPR - short arrays, size 128 */
    test_le_short(s_dst2, s_src1, s_src2, s_src3, SIZE2);
    
    /* GE_EXPR - small int arrays, size 64 */
    test_ge_small(small_dst, small_src1, small_src2, small_src3, 64);
    
    /* LT_EXPR - medium int arrays, size 192 */
    test_lt_medium(medium_dst, medium_src1, medium_src2, medium_src3, 192);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i];
    }
    
    for (int i = 0; i < SIZE2; i++) {
        checksum += s_dst1[i] + s_dst2[i];
    }
    
    for (int i = 0; i < 64; i++) {
        checksum += small_dst[i];
    }
    
    for (int i = 0; i < 192; i++) {
        checksum += medium_dst[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
