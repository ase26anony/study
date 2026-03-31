/* test_vectorize_comparisons.c
 * Designed to trigger vectorization of comparison operations
 * and cover the switch cases for GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR
 * in tree-vect-stmts.cc lines 12216-12233
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test function for > comparison */
void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR */
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

/* Test function for >= comparison */
void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  /* GE_EXPR */
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] * 2;
        }
    }
}

/* Test function for < comparison */
void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {  /* LT_EXPR */
            c[i] = a[i] << 1;
        } else {
            c[i] = b[i] << 1;
        }
    }
}

/* Test function for <= comparison */
void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {  /* LE_EXPR */
            c[i] = a[i] | b[i];
        } else {
            c[i] = a[i] & b[i];
        }
    }
}

/* Alternative test using ternary operator (also vectorizable) */
void test_gt_ternary(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? a[i] : b[i];  /* GT_EXPR with floats */
    }
}

/* Test with mixed comparisons in same loop */
void test_mixed_comparisons(int *restrict a, int *restrict b, 
                           int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        /* Multiple comparisons that should all be vectorized */
        c[i] = (a[i] > b[i]) ? 1 : 0;    /* GT_EXPR */
        d[i] = (a[i] <= b[i]) ? 1 : 0;   /* LE_EXPR */
    }
}

/* Initialize arrays with patterns that create varied comparison results */
void init_arrays(int *a, int *b) {
    for (int i = 0; i < N; i++) {
        a[i] = i;              /* 0, 1, 2, 3, ... */
        b[i] = N - i - 1;      /* 1023, 1022, 1021, ... */
    }
}

void init_float_arrays(float *a, float *b) {
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 1.5f;
        b[i] = (float)(N - i) * 0.5f;
    }
}

/* Verify results by computing checksum */
int verify_results(int *c, int expected_pattern) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += c[i];
    }
    printf("Checksum: %d (pattern %d)\n", sum, expected_pattern);
    return sum != 0;  /* Return non-zero if any computation happened */
}

int main(void) {
    /* Use aligned allocations for better vectorization */
    int *a = aligned_alloc(32, N * sizeof(int));
    int *b = aligned_alloc(32, N * sizeof(int));
    int *c1 = aligned_alloc(32, N * sizeof(int));
    int *c2 = aligned_alloc(32, N * sizeof(int));
    int *c3 = aligned_alloc(32, N * sizeof(int));
    int *c4 = aligned_alloc(32, N * sizeof(int));
    int *d1 = aligned_alloc(32, N * sizeof(int));
    int *d2 = aligned_alloc(32, N * sizeof(int));
    
    float *fa = aligned_alloc(32, N * sizeof(float));
    float *fb = aligned_alloc(32, N * sizeof(float));
    float *fc = aligned_alloc(32, N * sizeof(float));
    
    if (!a || !b || !c1 || !c2 || !c3 || !c4 || !d1 || !d2 || !fa || !fb || !fc) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    init_arrays(a, b);
    init_float_arrays(fa, fb);
    
    /* Clear output arrays */
    memset(c1, 0, N * sizeof(int));
    memset(c2, 0, N * sizeof(int));
    memset(c3, 0, N * sizeof(int));
    memset(c4, 0, N * sizeof(int));
    memset(d1, 0, N * sizeof(int));
    memset(d2, 0, N * sizeof(int));
    
    printf("Testing vectorization of comparison operators...\n");
    
    /* Test each comparison operator in separate loops */
    test_gt(a, b, c1);      /* Should trigger GT_EXPR case */
    test_ge(a, b, c2);      /* Should trigger GE_EXPR case */
    test_lt(a, b, c3);      /* Should trigger LT_EXPR case */
    test_le(a, b, c4);      /* Should trigger LE_EXPR case */
    
    /* Test with ternary operator (float comparison) */
    test_gt_ternary(fa, fb, fc);  /* GT_EXPR with floats */
    
    /* Test mixed comparisons */
    test_mixed_comparisons(a, b, d1, d2);  /* GT_EXPR and LE_EXPR in same loop */
    
    /* Verify that computations happened */
    int valid = 0;
    valid |= verify_results(c1, 1);
    valid |= verify_results(c2, 2);
    valid |= verify_results(c3, 3);
    valid |= verify_results(c4, 4);
    
    /* Check float results */
    float fsum = 0;
    for (int i = 0; i < N; i++) {
        fsum += fc[i];
    }
    printf("Float checksum: %f\n", fsum);
    
    /* Clean up */
    free(a); free(b); free(c1); free(c2); free(c3); free(c4);
    free(d1); free(d2); free(fa); free(fb); free(fc);
    
    return valid ? 0 : 1;
}
