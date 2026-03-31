#include <stdio.h>
#include <stdint.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 42;
volatile int use_volatile = 1;

/* Simple PRNG for semi-random data */
static int simple_rand(int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

/* Initialize arrays with semi-random data */
void init_arrays(int *a, int *b, int *c, short *sa, short *sb, short *sc, int size) {
    int seed = global_seed;
    for (int i = 0; i < size; i++) {
        a[i] = simple_rand(&seed) % 1000;
        b[i] = simple_rand(&seed) % 1000;
        c[i] = simple_rand(&seed) % 1000;
        sa[i] = (short)(simple_rand(&seed) % 1000);
        sb[i] = (short)(simple_rand(&seed) % 1000);
        sc[i] = (short)(simple_rand(&seed) % 1000);
    }
}

/* Test function for GT_EXPR (>) */
void test_gt_loop(int *a, int *b, int *c, int *dst, int size) {
    for (int i = 0; i < size; i++) {
        /* This should generate GT_EXPR internally */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* Test function for GE_EXPR (>=) */
void test_ge_loop(int *a, int *b, int *c, int *dst, int size) {
    for (int i = 0; i < size; i++) {
        /* This should generate GE_EXPR internally */
        if (a[i] >= b[i]) {
            dst[i] = a[i] * 2 + c[i];
        } else {
            dst[i] = b[i] * 2 - c[i];
        }
    }
}

/* Test function for LT_EXPR (<) */
void test_lt_loop(int *a, int *b, int *c, int *dst, int size) {
    for (int i = 0; i < size; i++) {
        /* This should generate LT_EXPR internally */
        if (a[i] < b[i]) {
            dst[i] = a[i] + b[i] + c[i];
        } else {
            dst[i] = a[i] - b[i] + c[i];
        }
    }
}

/* Test function for LE_EXPR (<=) */
void test_le_loop(int *a, int *b, int *c, int *dst, int size) {
    for (int i = 0; i < size; i++) {
        /* This should generate LE_EXPR internally */
        if (a[i] <= b[i]) {
            dst[i] = (a[i] << 1) + c[i];
        } else {
            dst[i] = (b[i] << 1) - c[i];
        }
    }
}

/* Same tests with short (16-bit) types to trigger different modes */
void test_gt_loop_short(short *a, short *b, short *c, short *dst, int size) {
    for (int i = 0; i < size; i++) {
        if (a[i] > b[i]) {
            dst[i] = (short)(a[i] + c[i]);
        } else {
            dst[i] = (short)(b[i] - c[i]);
        }
    }
}

void test_ge_loop_short(short *a, short *b, short *c, short *dst, int size) {
    for (int i = 0; i < size; i++) {
        if (a[i] >= b[i]) {
            dst[i] = (short)(a[i] * 3 + c[i]);
        } else {
            dst[i] = (short)(b[i] * 3 - c[i]);
        }
    }
}

void test_lt_loop_short(short *a, short *b, short *c, short *dst, int size) {
    for (int i = 0; i < size; i++) {
        if (a[i] < b[i]) {
            dst[i] = (short)(a[i] + c[i] * 2);
        } else {
            dst[i] = (short)(b[i] - c[i] * 2);
        }
    }
}

void test_le_loop_short(short *a, short *b, short *c, short *dst, int size) {
    for (int i = 0; i < size; i++) {
        if (a[i] <= b[i]) {
            dst[i] = (short)(a[i] + b[i] + c[i]);
        } else {
            dst[i] = (short)(a[i] - b[i] + c[i]);
        }
    }
}

/* Additional test with mixed comparisons in same loop */
void test_mixed_comparisons(int *a, int *b, int *c, int *dst, int size) {
    for (int i = 0; i < size; i++) {
        /* Mix different comparisons to ensure all are processed */
        if (a[i] > b[i]) {
            dst[i] += a[i];
        }
        if (a[i] >= c[i]) {
            dst[i] += b[i];
        }
        if (b[i] < c[i]) {
            dst[i] += c[i];
        }
        if (b[i] <= a[i]) {
            dst[i] -= a[i];
        }
    }
}

int main() {
    /* Declare arrays with different sizes */
    int a[SIZE], b[SIZE], c[SIZE];
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    int dst_mixed[SIZE];
    
    short sa[SIZE2], sb[SIZE2], sc[SIZE2];
    short sdst1[SIZE2], sdst2[SIZE2], sdst3[SIZE2], sdst4[SIZE2];
    
    /* Initialize with semi-random data */
    init_arrays(a, b, c, sa, sb, sc, SIZE2);
    
    /* Test all four comparison operators with int type */
    test_gt_loop(a, b, c, dst1, SIZE);
    test_ge_loop(a, b, c, dst2, SIZE);
    test_lt_loop(a, b, c, dst3, SIZE);
    test_le_loop(a, b, c, dst4, SIZE);
    
    /* Test all four comparison operators with short type */
    test_gt_loop_short(sa, sb, sc, sdst1, SIZE2);
    test_ge_loop_short(sa, sb, sc, sdst2, SIZE2);
    test_lt_loop_short(sa, sb, sc, sdst3, SIZE2);
    test_le_loop_short(sa, sb, sc, sdst4, SIZE2);
    
    /* Test mixed comparisons */
    for (int i = 0; i < SIZE; i++) {
        dst_mixed[i] = 0;
    }
    test_mixed_comparisons(a, b, c, dst_mixed, SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i] + dst_mixed[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += sdst1[i] + sdst2[i] + sdst3[i] + sdst4[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    return 0;
}
