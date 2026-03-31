/* test_vectorized_comparisons.c
 * Designed to trigger vectorization of comparison operations
 * and cover the switch cases in tree-vect-stmts.cc lines 12216-12233
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison operator */

/* GT_EXPR case */
void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR */
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

/* GE_EXPR case */
void test_ge(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  /* GE_EXPR */
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] / (b[i] + 1.0f);
        }
    }
}

/* LT_EXPR case */
void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {  /* LT_EXPR */
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] * 3;
        }
    }
}

/* LE_EXPR case */
void test_le(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {  /* LE_EXPR */
            c[i] = a[i] - b[i];
        } else {
            c[i] = b[i] - a[i];
        }
    }
}

/* Additional test with mixed comparisons to ensure all paths are taken */
void test_mixed_comparisons(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        /* Use different comparisons based on index pattern */
        if (i % 4 == 0) {
            if (a[i] > b[i]) c[i] = 1;    /* GT_EXPR */
        } else if (i % 4 == 1) {
            if (a[i] >= b[i]) c[i] = 2;   /* GE_EXPR */
        } else if (i % 4 == 2) {
            if (a[i] < b[i]) c[i] = 3;    /* LT_EXPR */
        } else {
            if (a[i] <= b[i]) c[i] = 4;   /* LE_EXPR */
        }
    }
}

/* Test with ternary operator (also vectorizable) */
void test_ternary_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? a[i] : b[i];  /* GT_EXPR in ternary */
    }
}

/* Initialize arrays with pattern to ensure comparisons have mixed results */
void init_arrays(int *a, int *b, float *fa, float *fb) {
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i - 1;  /* Creates mixed comparison results */
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(N - i) * 0.75f;
    }
}

/* Compute checksum to prevent dead code elimination */
int compute_checksum(int *c, float *fc) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += c[i];
        sum += (int)fc[i];
    }
    return sum;
}

int main() {
    /* Use aligned arrays for better vectorization */
    ALIGNED int a[N], b[N], c1[N], c3[N], c5[N], c6[N];
    ALIGNED float fa[N], fb[N], fc2[N], fc4[N];
    
    /* Initialize with pattern */
    init_arrays(a, b, fa, fb);
    
    /* Clear output arrays */
    memset(c1, 0, sizeof(c1));
    memset(c3, 0, sizeof(c3));
    memset(c5, 0, sizeof(c5));
    memset(c6, 0, sizeof(c6));
    memset(fc2, 0, sizeof(fc2));
    memset(fc4, 0, sizeof(fc4));
    
    /* Execute all test functions */
    test_gt(a, b, c1);           /* GT_EXPR */
    test_ge(fa, fb, fc2);        /* GE_EXPR */
    test_lt(a, b, c3);           /* LT_EXPR */
    test_le(fa, fb, fc4);        /* LE_EXPR */
    test_mixed_comparisons(a, b, c5);  /* All comparisons */
    test_ternary_gt(a, b, c6);   /* GT_EXPR in ternary */
    
    /* Compute and print checksum to ensure code runs */
    int checksum = compute_checksum(c1, fc2);
    checksum += compute_checksum(c3, fc4);
    checksum += compute_checksum(c5, NULL);
    checksum += compute_checksum(c6, NULL);
    
    printf("Checksum: %d\n", checksum);
    printf("All vectorized comparison tests completed.\n");
    
    /* Verify a few values to ensure correctness */
    printf("Sample verification (first 5 elements):\n");
    for (int i = 0; i < 5; i++) {
        printf("  [%d] a=%d, b=%d, c1=%d, c3=%d\n", 
               i, a[i], b[i], c1[i], c3[i]);
    }
    
    return 0;
}
