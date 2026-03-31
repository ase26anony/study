/* test_vector_comparisons.c
 * Designed to trigger vectorization of comparison operations
 * and cover the switch cases for GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR
 * in tree-vect-stmts.cc lines 12216-12233
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test function for > operator (GT_EXPR) */
void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            c[i] = a[i] - b[i];
        } else {
            c[i] = 0;
        }
    }
}

/* Test function for >= operator (GE_EXPR) */
void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = -1;
        }
    }
}

/* Test function for < operator (LT_EXPR) */
void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            c[i] = b[i] - a[i];
        } else {
            c[i] = 0;
        }
    }
}

/* Test function for <= operator (LE_EXPR) */
void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i];
        }
    }
}

/* Additional test with floating point to ensure different type handling */
void test_float_gt(float *restrict fa, float *restrict fb, float *restrict fc) {
    for (int i = 0; i < N; i++) {
        if (fa[i] > fb[i]) {
            fc[i] = fa[i];
        } else {
            fc[i] = fb[i];
        }
    }
}

/* Test with mixed operations to ensure all paths are taken */
void test_mixed_comparisons(int *restrict a, int *restrict b, 
                           int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        /* This should trigger multiple comparison transformations */
        if (a[i] > b[i]) {
            c[i] = 1;
        } else if (a[i] <= b[i]) {
            c[i] = 2;
        }
        
        if (a[i] >= b[i]) {
            d[i] = a[i];
        } else if (a[i] < b[i]) {
            d[i] = b[i];
        }
    }
}

/* Initialize arrays with pattern that ensures all comparison results occur */
void init_arrays(int *a, int *b, float *fa, float *fb) {
    for (int i = 0; i < N; i++) {
        /* Pattern: a increases, b decreases, creating all comparison conditions */
        a[i] = i;
        b[i] = N - i - 1;
        fa[i] = (float)i / 10.0f;
        fb[i] = (float)(N - i) / 10.0f;
    }
}

/* Compute checksum to ensure code executes and prevent dead code elimination */
int compute_checksum(int *arr) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
    return sum;
}

float compute_fchecksum(float *arr) {
    float sum = 0.0f;
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
    return sum;
}

int main() {
    /* Use aligned allocations for better vectorization */
    int *a = (int*)aligned_alloc(32, N * sizeof(int));
    int *b = (int*)aligned_alloc(32, N * sizeof(int));
    int *c1 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c2 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c3 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c4 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c5 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c6 = (int*)aligned_alloc(32, N * sizeof(int));
    float *fa = (float*)aligned_alloc(32, N * sizeof(float));
    float *fb = (float*)aligned_alloc(32, N * sizeof(float));
    float *fc = (float*)aligned_alloc(32, N * sizeof(float));
    
    if (!a || !b || !c1 || !c2 || !c3 || !c4 || !c5 || !c6 || !fa || !fb || !fc) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(a, b, fa, fb);
    
    /* Execute all test functions to trigger different comparison operators */
    test_gt(a, b, c1);
    test_ge(a, b, c2);
    test_lt(a, b, c3);
    test_le(a, b, c4);
    test_float_gt(fa, fb, fc);
    test_mixed_comparisons(a, b, c5, c6);
    
    /* Compute and print checksums to ensure code executed */
    printf("Checksum GT: %d\n", compute_checksum(c1));
    printf("Checksum GE: %d\n", compute_checksum(c2));
    printf("Checksum LT: %d\n", compute_checksum(c3));
    printf("Checksum LE: %d\n", compute_checksum(c4));
    printf("Checksum Float GT: %f\n", compute_fchecksum(fc));
    printf("Checksum Mixed 1: %d\n", compute_checksum(c5));
    printf("Checksum Mixed 2: %d\n", compute_checksum(c6));
    
    /* Clean up */
    free(a); free(b); free(c1); free(c2); free(c3); free(c4);
    free(c5); free(c6); free(fa); free(fb); free(fc);
    
    return 0;
}
