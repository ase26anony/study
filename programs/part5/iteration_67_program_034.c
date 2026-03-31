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

/* Test GT_EXPR transformation */
void test_gt_loop(int *dst, int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        /* This should generate GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* Test GE_EXPR transformation */
void test_ge_loop(int *dst, int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        /* This should generate GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR */
        if (a[i] >= b[i]) {
            dst[i] = a[i] * 2 + c[i];
        } else {
            dst[i] = b[i] * 3 - c[i];
        }
    }
}

/* Test LT_EXPR transformation */
void test_lt_loop(int *dst, int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        /* This should generate LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
        if (a[i] < b[i]) {
            dst[i] = a[i] - c[i];
        } else {
            dst[i] = b[i] + c[i];
        }
    }
}

/* Test LE_EXPR transformation */
void test_le_loop(int *dst, int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        /* This should generate LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
        if (a[i] <= b[i]) {
            dst[i] = a[i] + b[i] + c[i];
        } else {
            dst[i] = a[i] - b[i] - c[i];
        }
    }
}

/* Test with short integers to trigger different modes */
void test_gt_loop_short(short *dst, short *a, short *b, short *c, int size) {
    for (int i = 0; i < size; i++) {
        if (a[i] > b[i]) {
            dst[i] = (short)(a[i] + c[i]);
        } else {
            dst[i] = (short)(b[i] - c[i]);
        }
    }
}

/* Test with mixed comparisons in same loop */
void test_mixed_comparisons(int *dst1, int *dst2, int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        /* Mix GT and LT in same loop body */
        if (a[i] > b[i]) {
            dst1[i] = a[i] + 1;
        } else {
            dst1[i] = b[i] - 1;
        }
        
        if (a[i] < c[i]) {
            dst2[i] = a[i] * 2;
        } else {
            dst2[i] = c[i] * 3;
        }
    }
}

/* Test with volatile inputs to prevent optimization */
void test_ge_loop_volatile(volatile int *dst, volatile int *a, volatile int *b, int size) {
    for (int i = 0; i < size; i++) {
        if (a[i] >= b[i]) {
            dst[i] = a[i] + 100;
        } else {
            dst[i] = b[i] - 100;
        }
    }
}

/* Test with different loop lengths */
void test_le_variable_length(int *dst, int *a, int *b, int *c, int length) {
    /* Use volatile to prevent loop unrolling before vectorization */
    volatile int len = length;
    for (int i = 0; i < len; i++) {
        if (a[i] <= b[i]) {
            dst[i] = a[i] + c[i] + i;
        } else {
            dst[i] = b[i] - c[i] - i;
        }
    }
}

int main() {
    /* Declare arrays with different sizes */
    int a[SIZE], b[SIZE], c[SIZE];
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    int dst5[SIZE], dst6[SIZE];
    
    short sa[SIZE2], sb[SIZE2], sc[SIZE2];
    short sdst[SIZE2];
    
    volatile int va[SIZE], vb[SIZE], vdst[SIZE];
    
    /* Initialize all arrays */
    init_arrays(a, b, c, SIZE);
    init_arrays(dst5, dst6, a, SIZE); /* Reuse for initialization */
    
    /* Initialize short arrays */
    int state = global_seed;
    for (int i = 0; i < SIZE2; i++) {
        sa[i] = (short)(lcg_rand(&state) % 1000);
        sb[i] = (short)(lcg_rand(&state) % 1000);
        sc[i] = (short)(lcg_rand(&state) % 1000);
    }
    
    /* Initialize volatile arrays */
    for (int i = 0; i < SIZE; i++) {
        va[i] = a[i];
        vb[i] = b[i];
    }
    
    /* Test all comparison operators */
    test_gt_loop(dst1, a, b, c, SIZE);
    test_ge_loop(dst2, a, b, c, SIZE);
    test_lt_loop(dst3, a, b, c, SIZE);
    test_le_loop(dst4, a, b, c, SIZE);
    
    /* Test with short integers */
    test_gt_loop_short(sdst, sa, sb, sc, SIZE2);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst5, dst6, a, b, c, SIZE);
    
    /* Test with volatile */
    if (use_volatile) {
        test_ge_loop_volatile(vdst, va, vb, SIZE);
    }
    
    /* Test variable length */
    test_le_variable_length(dst1, a, b, c, SIZE - 64);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i] + dst5[i] + dst6[i];
        if (i < SIZE && use_volatile) {
            checksum += vdst[i];
        }
    }
    
    for (int i = 0; i < SIZE2; i++) {
        checksum += sdst[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
