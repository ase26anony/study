/* test_vectorize_comparisons.c
 * Designed to trigger vectorizer transformation of comparison operations
 * to bitwise mask operations in GCC's tree-vect-stmts.cc
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison operator */

void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR - should trigger case GT_EXPR */
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] / 2;
        }
    }
}

void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  /* GE_EXPR - should trigger case GE_EXPR */
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {  /* LT_EXPR - should trigger case LT_EXPR */
            c[i] = a[i] | b[i];  /* Bitwise operation to encourage mask usage */
        } else {
            c[i] = a[i] & b[i];
        }
    }
}

void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {  /* LE_EXPR - should trigger case LE_EXPR */
            c[i] = (a[i] << 1) ^ b[i];
        } else {
            c[i] = (a[i] >> 1) | b[i];
        }
    }
}

/* Additional test with floating point to ensure different type handling */
void test_gt_float(float *restrict fa, float *restrict fb, float *restrict fc) {
    for (int i = 0; i < N; i++) {
        if (fa[i] > fb[i]) {  /* GT_EXPR with floats */
            fc[i] = fa[i] * 2.0f;
        } else {
            fc[i] = fb[i] * 0.5f;
        }
    }
}

void test_lt_float(float *restrict fa, float *restrict fb, float *restrict fc) {
    for (int i = 0; i < N; i++) {
        if (fa[i] < fb[i]) {  /* LT_EXPR with floats */
            fc[i] = fa[i] + fb[i];
        } else {
            fc[i] = fa[i] - fb[i];
        }
    }
}

/* Helper function to initialize arrays with pattern that creates varied comparisons */
void init_arrays(int *a, int *b, float *fa, float *fb) {
    for (int i = 0; i < N; i++) {
        a[i] = i;                    /* 0, 1, 2, 3, ... */
        b[i] = N/2 - i;              /* 512, 511, 510, ... */
        fa[i] = (float)i * 0.5f;
        fb[i] = (float)(N - i) * 0.25f;
    }
}

/* Compute checksum to ensure loops execute and produce results */
int compute_checksum(int *c1, int *c2, int *c3, int *c4, float *fc1, float *fc2) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += c1[i] + c2[i] + c3[i] + c4[i];
        sum += (int)fc1[i] + (int)fc2[i];
    }
    return sum;
}

int main() {
    /* Use aligned arrays to help vectorization */
    ALIGNED int a[N], b[N];
    ALIGNED int c1[N], c2[N], c3[N], c4[N];
    ALIGNED float fa[N], fb[N];
    ALIGNED float fc1[N], fc2[N];
    
    /* Initialize with patterns that ensure all comparison outcomes occur */
    init_arrays(a, b, fa, fb);
    
    /* Execute all test functions */
    test_gt(a, b, c1);
    test_ge(a, b, c2);
    test_lt(a, b, c3);
    test_le(a, b, c4);
    test_gt_float(fa, fb, fc1);
    test_lt_float(fa, fb, fc2);
    
    /* Compute and print checksum to prevent dead code elimination */
    int checksum = compute_checksum(c1, c2, c3, c4, fc1, fc2);
    printf("Checksum: %d\n", checksum);
    
    /* Also print a few values to verify correctness */
    printf("Sample outputs - c1[0:3]: %d %d %d %d\n", c1[0], c1[1], c1[2], c1[3]);
    printf("Sample outputs - c3[0:3]: %d %d %d %d\n", c3[0], c3[1], c3[2], c3[3]);
    
    return 0;
}
