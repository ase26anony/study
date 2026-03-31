/* test_vector_comparisons.c
 * Designed to trigger vectorizer transformation of comparison operations
 * to bitwise mask operations in GCC's tree-vect-stmts.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison operator */

/* GT_EXPR case - lines 12216-12218 */
void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR */
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

/* GE_EXPR case - lines 12219-12221 */
void test_ge(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  /* GE_EXPR */
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] / (b[i] + 1.0f);
        }
    }
}

/* LT_EXPR case - lines 12222-12226 */
void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {  /* LT_EXPR - should trigger std::swap */
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] * 3;
        }
    }
}

/* LE_EXPR case - lines 12227-12233 */
void test_le(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {  /* LE_EXPR - should trigger std::swap */
            c[i] = a[i] - b[i];
        } else {
            c[i] = b[i] - a[i];
        }
    }
}

/* Additional test with mixed comparisons to ensure all paths are taken */
void test_mixed_comparisons(int *restrict a, int *restrict b, 
                           int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        /* Multiple comparisons in same loop */
        if (a[i] > b[i]) {   /* GT_EXPR */
            c[i] = 1;
        } else if (a[i] >= b[i]) {  /* GE_EXPR */
            c[i] = 2;
        } else if (a[i] < b[i]) {   /* LT_EXPR */
            c[i] = 3;
        } else if (a[i] <= b[i]) {  /* LE_EXPR */
            c[i] = 4;
        } else {
            c[i] = 0;
        }
        
        /* Use ternary operator as alternative mask generation */
        d[i] = (a[i] > b[i]) ? a[i] : b[i];  /* Another GT_EXPR */
    }
}

/* Initialize arrays with pattern data to ensure varied comparison results */
void init_arrays(int *a_int, int *b_int, float *a_float, float *b_float) {
    for (int i = 0; i < N; i++) {
        a_int[i] = i;
        b_int[i] = N - i - 1;  /* Creates mix of true/false comparisons */
        a_float[i] = (float)i * 1.5f;
        b_float[i] = (float)(N - i) * 0.75f;
    }
}

/* Compute checksum to ensure loops execute and prevent dead code elimination */
int compute_checksum(int *c_int, float *c_float) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += c_int[i];
        sum += (int)c_float[i];
    }
    return sum;
}

int main(void) {
    /* Use aligned arrays to help vectorization */
    ALIGNED int a_int[N], b_int[N], c_int[N], d_int[N], e_int[N];
    ALIGNED float a_float[N], b_float[N], c_float[N], d_float[N];
    
    /* Initialize with pattern data */
    init_arrays(a_int, b_int, a_float, b_float);
    
    /* Execute all test functions */
    test_gt(a_int, b_int, c_int);
    test_ge(a_float, b_float, c_float);
    test_lt(a_int, b_int, d_int);
    test_le(a_float, b_float, d_float);
    test_mixed_comparisons(a_int, b_int, e_int, a_int); /* Reuse a_int as temp */
    
    /* Compute and print checksum to ensure code executes */
    int checksum1 = compute_checksum(c_int, c_float);
    int checksum2 = compute_checksum(d_int, d_float);
    int checksum3 = compute_checksum(e_int, c_float);
    
    printf("Checksums: %d, %d, %d\n", checksum1, checksum2, checksum3);
    printf("All comparison tests completed.\n");
    
    return 0;
}
