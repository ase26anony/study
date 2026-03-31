#include <stdio.h>
#include <stdlib.h>

#define SIZE 256
#define SIZE2 128

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 12345;
volatile int global_mod = 1000;

/* Simple LCG for semi-random data */
static int lcg_next(int *state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

/* Initialize arrays with semi-random data */
void init_arrays(int *a, int *b, int *c, int size) {
    int state = global_seed;
    for (int i = 0; i < size; i++) {
        a[i] = lcg_next(&state) % global_mod;
        b[i] = lcg_next(&state) % global_mod;
        c[i] = lcg_next(&state) % global_mod;
    }
}

/* Test functions for each comparison operator */

/* GT_EXPR case: > comparison */
void test_gt_loop(int *dst, const int *a, const int *b, const int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GT_EXPR */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i];
        }
    }
}

/* GE_EXPR case: >= comparison */
void test_ge_loop(int *dst, const int *a, const int *b, const int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate GE_EXPR */
        if (a[i] >= b[i]) {
            dst[i] = a[i] * c[i];
        } else {
            dst[i] = b[i] - c[i];
        }
    }
}

/* LT_EXPR case: < comparison */
void test_lt_loop(int *dst, const int *a, const int *b, const int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LT_EXPR */
        if (a[i] < b[i]) {
            dst[i] = a[i] | c[i];
        } else {
            dst[i] = b[i] & c[i];
        }
    }
}

/* LE_EXPR case: <= comparison */
void test_le_loop(int *dst, const int *a, const int *b, const int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should generate LE_EXPR */
        if (a[i] <= b[i]) {
            dst[i] = a[i] ^ c[i];
        } else {
            dst[i] = b[i] | c[i];
        }
    }
}

/* Test with short type to trigger different mode conditions */
void test_gt_loop_short(short *dst, const short *a, const short *b, const short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* GT_EXPR with short type */
        if (a[i] > b[i]) {
            dst[i] = a[i] + c[i];
        } else {
            dst[i] = b[i];
        }
    }
}

void test_le_loop_short(short *dst, const short *a, const short *b, const short *c, int n) {
    for (int i = 0; i < n; i++) {
        /* LE_EXPR with short type */
        if (a[i] <= b[i]) {
            dst[i] = a[i] - c[i];
        } else {
            dst[i] = b[i] + c[i];
        }
    }
}

/* Mixed comparison in same loop */
void test_mixed_comparisons(int *dst1, int *dst2, int *dst3, int *dst4,
                           const int *a, const int *b, const int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* All four comparisons in one loop */
        if (a[i] > b[i]) {
            dst1[i] = a[i] + c[i];
        } else {
            dst1[i] = b[i];
        }
        
        if (a[i] >= b[i]) {
            dst2[i] = a[i] * c[i];
        } else {
            dst2[i] = b[i] - c[i];
        }
        
        if (a[i] < b[i]) {
            dst3[i] = a[i] | c[i];
        } else {
            dst3[i] = b[i] & c[i];
        }
        
        if (a[i] <= b[i]) {
            dst4[i] = a[i] ^ c[i];
        } else {
            dst4[i] = b[i] | c[i];
        }
    }
}

int main() {
    /* Declare arrays with different sizes */
    int a[SIZE], b[SIZE], c[SIZE];
    int dst1[SIZE], dst2[SIZE], dst3[SIZE], dst4[SIZE];
    
    short a_short[SIZE2], b_short[SIZE2], c_short[SIZE2];
    short dst_short1[SIZE2], dst_short2[SIZE2];
    
    /* Initialize with semi-random data */
    init_arrays(a, b, c, SIZE);
    
    /* Initialize short arrays */
    int state = global_seed;
    for (int i = 0; i < SIZE2; i++) {
        a_short[i] = lcg_next(&state) % 1000;
        b_short[i] = lcg_next(&state) % 1000;
        c_short[i] = lcg_next(&state) % 1000;
    }
    
    /* Test each comparison operator separately */
    test_gt_loop(dst1, a, b, c, SIZE);
    test_ge_loop(dst2, a, b, c, SIZE);
    test_lt_loop(dst3, a, b, c, SIZE);
    test_le_loop(dst4, a, b, c, SIZE);
    
    /* Test with short type */
    test_gt_loop_short(dst_short1, a_short, b_short, c_short, SIZE2);
    test_le_loop_short(dst_short2, a_short, b_short, c_short, SIZE2);
    
    /* Test mixed comparisons */
    int dst_mixed1[SIZE2], dst_mixed2[SIZE2], dst_mixed3[SIZE2], dst_mixed4[SIZE2];
    int a_mixed[SIZE2], b_mixed[SIZE2], c_mixed[SIZE2];
    init_arrays(a_mixed, b_mixed, c_mixed, SIZE2);
    test_mixed_comparisons(dst_mixed1, dst_mixed2, dst_mixed3, dst_mixed4,
                          a_mixed, b_mixed, c_mixed, SIZE2);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + dst4[i];
    }
    for (int i = 0; i < SIZE2; i++) {
        checksum += dst_short1[i] + dst_short2[i];
        checksum += dst_mixed1[i] + dst_mixed2[i] + dst_mixed3[i] + dst_mixed4[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
