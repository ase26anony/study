#include <stdio.h>
#include <stdint.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 42;
volatile int use_volatile = 1;

/* Simple PRNG for semi-random data */
static inline int simple_rand(int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

/* Initialize arrays with semi-random data */
void init_arrays(int *a, int *b, int *c, short *sa, short *sb, short *sc, int n) {
    int seed = global_seed;
    for (int i = 0; i < n; i++) {
        a[i] = simple_rand(&seed) % 1000;
        b[i] = simple_rand(&seed) % 1000;
        c[i] = simple_rand(&seed) % 1000;
        sa[i] = simple_rand(&seed) % 500;
        sb[i] = simple_rand(&seed) % 500;
        sc[i] = simple_rand(&seed) % 500;
    }
}

/* GT_EXPR case: > comparison */
void test_gt_loop(int *dst, int *src1, int *src2, int *src3, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GT_EXPR */
        if (src1[i] > src2[i]) {
            dst[i] = src1[i] + src3[i];
        } else {
            dst[i] = src2[i] - src3[i];
        }
    }
}

/* GE_EXPR case: >= comparison */
void test_ge_loop(int *dst, int *src1, int *src2, int *src3, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GE_EXPR */
        if (src1[i] >= src2[i]) {
            dst[i] = src1[i] * 2 + src3[i];
        } else {
            dst[i] = src2[i] * 2 - src3[i];
        }
    }
}

/* LT_EXPR case: < comparison */
void test_lt_loop(int *dst, int *src1, int *src2, int *src3, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LT_EXPR */
        if (src1[i] < src2[i]) {
            dst[i] = src1[i] + src3[i] * 3;
        } else {
            dst[i] = src2[i] - src3[i] * 3;
        }
    }
}

/* LE_EXPR case: <= comparison */
void test_le_loop(int *dst, int *src1, int *src2, int *src3, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LE_EXPR */
        if (src1[i] <= src2[i]) {
            dst[i] = src1[i] * 4 + src3[i];
        } else {
            dst[i] = src2[i] * 4 - src3[i];
        }
    }
}

/* Mixed types: short arrays with GT_EXPR */
void test_gt_short(short *dst, short *src1, short *src2, short *src3, int n) {
    for (int i = 0; i < n; i++) {
        /* GT_EXPR with different data width */
        if (src1[i] > src2[i]) {
            dst[i] = src1[i] + src3[i];
        } else {
            dst[i] = src2[i] - src3[i];
        }
    }
}

/* Mixed types: short arrays with LE_EXPR */
void test_le_short(short *dst, short *src1, short *src2, short *src3, int n) {
    for (int i = 0; i < n; i++) {
        /* LE_EXPR with different data width */
        if (src1[i] <= src2[i]) {
            dst[i] = src1[i] * 2 + src3[i];
        } else {
            dst[i] = src2[i] * 2 - src3[i];
        }
    }
}

/* Another variant with different array sizes */
void test_ge_mixed(int *dst, int *src1, int *src2, int *src3, short *src4, int n) {
    for (int i = 0; i < n; i++) {
        /* GE_EXPR with mixed computation */
        if (src1[i] >= src2[i]) {
            dst[i] = src1[i] + src4[i];
        } else {
            dst[i] = src2[i] - src4[i];
        }
    }
}

/* LT_EXPR with different pattern */
void test_lt_pattern(int *dst, int *src1, int *src2, int *src3, int n) {
    for (int i = 0; i < n; i++) {
        /* LT_EXPR with swapped operands in computation */
        int temp = src3[i];
        if (src1[i] < src2[i]) {
            dst[i] = temp + src1[i];
        } else {
            dst[i] = temp - src2[i];
        }
    }
}

int main() {
    /* Declare arrays with different sizes */
    int src1[SIZE], src2[SIZE], src3[SIZE];
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    
    short s_src1[SIZE2], s_src2[SIZE2], s_src3[SIZE2];
    short s_dst1[SIZE2], s_dst2[SIZE2];
    
    int src_mixed[SIZE2];
    short src_short[SIZE2];
    int dst_mixed[SIZE2];
    
    /* Initialize with semi-random data */
    init_arrays(src1, src2, src3, s_src1, s_src2, s_src3, SIZE);
    
    /* Initialize mixed arrays */
    int seed = global_seed + 1;
    for (int i = 0; i < SIZE2; i++) {
        src_mixed[i] = simple_rand(&seed) % 1000;
        src_short[i] = simple_rand(&seed) % 500;
    }
    
    /* Test all comparison operators with int arrays */
    test_gt_loop(dst1, src1, src2, src3, SIZE);
    test_ge_loop(dst2, src1, src2, src3, SIZE);
    test_lt_loop(dst3, src1, src2, src3, SIZE);
    test_le_loop(dst4, src1, src2, src3, SIZE);
    
    /* Test with short arrays */
    test_gt_short(s_dst1, s_src1, s_src2, s_src3, SIZE2);
    test_le_short(s_dst2, s_src1, s_src2, s_src3, SIZE2);
    
    /* Test mixed types */
    test_ge_mixed(dst_mixed, src_mixed, src1, src2, src_short, SIZE2);
    
    /* Test another LT pattern */
    test_lt_pattern(dst1, src1, src2, src3, SIZE);  // Reuse dst1
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += s_dst1[i] + s_dst2[i] + dst_mixed[i];
    }
    
    printf("Checksum: %lu\n", (unsigned long)checksum);
    
    return 0;
}
