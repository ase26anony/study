/* test_vector_comparisons.c
 * Designed to trigger vectorization of comparison operations
 * to cover lines 12216-12233 in tree-vect-stmts.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
void test_mixed_comparisons(int *restrict a, int *restrict b, 
                           int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        /* Use multiple comparison types in same loop */
        int cond1 = a[i] > b[i];   /* GT_EXPR */
        int cond2 = a[i] >= b[i];  /* GE_EXPR */
        int cond3 = a[i] < b[i];   /* LT_EXPR */
        int cond4 = a[i] <= b[i];  /* LE_EXPR */
        
        /* Complex expression using all comparisons */
        c[i] = (cond1 && cond2) ? a[i] : b[i];
        d[i] = (cond3 || cond4) ? a[i] * b[i] : a[i] + b[i];
    }
}

/* Test with ternary operator (conditional expression) */
void test_ternary(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        /* Ternary with > comparison */
        c[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
}

/* Initialize arrays with pattern to ensure varied comparison results */
void init_arrays(int *a, int *b, float *fa, float *fb) {
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i - 1;  /* Creates mix of true/false comparisons */
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(N - i) * 0.75f;
    }
}

/* Compute checksum to prevent dead code elimination */
int64_t compute_checksum(int *arr, int size) {
    int64_t sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

float compute_fchecksum(float *arr, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

int main() {
    /* Use aligned arrays for better vectorization */
    ALIGNED int a[N], b[N], c[N], d[N], e[N];
    ALIGNED float fa[N], fb[N], fc[N], fd[N];
    
    /* Initialize with pattern */
    init_arrays(a, b, fa, fb);
    
    /* Test each comparison operator separately */
    test_gt(a, b, c);
    test_ge(fa, fb, fc);
    test_lt(a, b, d);
    test_le(fa, fb, fd);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(a, b, c, e);
    
    /* Test ternary operator */
    test_ternary(a, b, d);
    
    /* Compute and print checksums to ensure code executes */
    int64_t sum1 = compute_checksum(c, N);
    int64_t sum2 = compute_checksum(d, N);
    int64_t sum3 = compute_checksum(e, N);
    float sum4 = compute_fchecksum(fc, N);
    float sum5 = compute_fchecksum(fd, N);
    
    printf("Checksums (for verification):\n");
    printf("test_gt: %ld\n", sum1);
    printf("test_lt/ternary: %ld\n", sum2);
    printf("test_mixed: %ld\n", sum3);
    printf("test_ge: %f\n", sum4);
    printf("test_le: %f\n", sum5);
    
    /* Simple validation */
    if (sum1 != 0 && sum2 != 0 && sum3 != 0 && sum4 != 0.0f && sum5 != 0.0f) {
        printf("All tests executed successfully.\n");
    } else {
        printf("Warning: Some checksums are zero.\n");
    }
    
    return 0;
}
