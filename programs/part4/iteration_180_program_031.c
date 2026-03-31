/* test_vector_cond_bitops.c
 * 
 * This program is designed to trigger the transformation of comparison
 * operations (GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR) to bit operations
 * (BIT_NOT_EXPR, BIT_AND_EXPR, BIT_IOR_EXPR) during auto-vectorization
 * in GCC's tree-vect-stmts.cc (lines 12216-12233).
 *
 * Compile with: gcc -O3 -ftree-vectorize -fno-vect-cost-model -march=native test_vector_cond_bitops.c -o test_vector_cond_bitops
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define SEED 42

/* Initialize arrays with varying data to create mixed true/false conditions */
static void init_arrays(int *a, int *b, int *x, int *y) {
    srand(SEED);
    for (int i = 0; i < N; i++) {
        a[i] = i;                    /* 0, 1, 2, ..., N-1 */
        b[i] = N/2;                  /* All N/2 (512) */
        x[i] = rand() % 1000;        /* Random values 0-999 */
        y[i] = rand() % 1000;        /* Random values 0-999 */
    }
}

/* GT_EXPR (>) - should trigger BIT_NOT_EXPR + BIT_AND_EXPR transformation */
static void test_gt_expr(int *a, int *b, int *x, int *y, int *out) {
    /* Conditional assignment pattern that forces mask generation */
    for (int i = 0; i < N; i++) {
        /* This will be transformed: (a[i] > b[i]) ? x[i] : y[i] */
        out[i] = (a[i] > b[i]) ? x[i] : y[i];
    }
}

/* GE_EXPR (>=) - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR transformation */
static void test_ge_expr(int *a, int *b, int *x, int *y, int *out) {
    /* Conditional increment with reduction-like pattern */
    for (int i = 0; i < N; i++) {
        /* Masked operation: out[i] = (a[i] >= b[i]) ? (x[i] + y[i]) : (x[i] - y[i]) */
        out[i] = (a[i] >= b[i]) ? (x[i] + y[i]) : (x[i] - y[i]);
    }
}

/* LT_EXPR (<) - should trigger BIT_NOT_EXPR + BIT_AND_EXPR with operand swap */
static void test_lt_expr(int *a, int *b, int *x, int *y, int *out) {
    /* Conditional assignment with < operator */
    for (int i = 0; i < N; i++) {
        /* This should trigger std::swap(cond_expr0, cond_expr1) */
        out[i] = (a[i] < b[i]) ? x[i] : y[i];
    }
}

/* LE_EXPR (<=) - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR with operand swap */
static void test_le_expr(int *a, int *b, int *x, int *y, int *out) {
    /* More complex conditional to ensure vectorization */
    for (int i = 0; i < N; i++) {
        /* Nested conditionals to create interesting mask patterns */
        if (a[i] <= b[i]) {
            out[i] = x[i] * 2;
        } else {
            out[i] = y[i] / 2;
        }
    }
}

/* Additional test with floating point to ensure different data types are covered */
static void test_float_comparisons(float *fa, float *fb, float *out) {
    /* Test all four operators with floating point */
    for (int i = 0; i < N; i++) {
        /* GT */
        if (fa[i] > fb[i]) out[i] = fa[i];
        else out[i] = fb[i];
        
        /* GE - accumulate results */
        out[i] += (fa[i] >= fb[i]) ? 1.0f : 0.5f;
        
        /* LT - modify based on condition */
        if (fa[i] < fb[i]) out[i] *= 2.0f;
        
        /* LE - final adjustment */
        out[i] = (fa[i] <= fb[i]) ? out[i] + 10.0f : out[i] - 5.0f;
    }
}

/* Reduction pattern that often triggers vectorization */
static int test_reduction_with_comparisons(int *a, int *b) {
    int sum = 0;
    /* Reduction with conditional increment */
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) sum += a[i];    /* GT_EXPR */
        if (a[i] >= b[i]) sum += b[i];   /* GE_EXPR */
        if (a[i] < b[i]) sum += i;       /* LT_EXPR */
        if (a[i] <= b[i]) sum -= i;      /* LE_EXPR */
    }
    return sum;
}

/* Verify results by comparing with sequential reference implementation */
static int verify_results(int *out_gt, int *out_ge, int *out_lt, int *out_le,
                         int *a, int *b, int *x, int *y) {
    int errors = 0;
    
    /* Verify GT results */
    for (int i = 0; i < N; i++) {
        int expected = (a[i] > b[i]) ? x[i] : y[i];
        if (out_gt[i] != expected) {
            errors++;
            if (errors < 5) printf("GT mismatch at %d: got %d, expected %d\n", 
                                  i, out_gt[i], expected);
        }
    }
    
    /* Verify GE results */
    for (int i = 0; i < N; i++) {
        int expected = (a[i] >= b[i]) ? (x[i] + y[i]) : (x[i] - y[i]);
        if (out_ge[i] != expected) {
            errors++;
            if (errors < 5) printf("GE mismatch at %d: got %d, expected %d\n", 
                                  i, out_ge[i], expected);
        }
    }
    
    /* Verify LT results */
    for (int i = 0; i < N; i++) {
        int expected = (a[i] < b[i]) ? x[i] : y[i];
        if (out_lt[i] != expected) {
            errors++;
            if (errors < 5) printf("LT mismatch at %d: got %d, expected %d\n", 
                                  i, out_lt[i], expected);
        }
    }
    
    /* Verify LE results */
    for (int i = 0; i < N; i++) {
        int expected = (a[i] <= b[i]) ? (x[i] * 2) : (y[i] / 2);
        if (out_le[i] != expected) {
            errors++;
            if (errors < 5) printf("LE mismatch at %d: got %d, expected %d\n", 
                                  i, out_le[i], expected);
        }
    }
    
    return errors;
}

int main() {
    /* Allocate and initialize integer arrays */
    int *a = malloc(N * sizeof(int));
    int *b = malloc(N * sizeof(int));
    int *x = malloc(N * sizeof(int));
    int *y = malloc(N * sizeof(int));
    int *out_gt = malloc(N * sizeof(int));
    int *out_ge = malloc(N * sizeof(int));
    int *out_lt = malloc(N * sizeof(int));
    int *out_le = malloc(N * sizeof(int));
    
    if (!a || !b || !x || !y || !out_gt || !out_ge || !out_lt || !out_le) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(a, b, x, y);
    
    /* Execute all test functions */
    test_gt_expr(a, b, x, y, out_gt);
    test_ge_expr(a, b, x, y, out_ge);
    test_lt_expr(a, b, x, y, out_lt);
    test_le_expr(a, b, x, y, out_le);
    
    /* Test reduction pattern */
    int reduction_result = test_reduction_with_comparisons(a, b);
    
    /* Test floating point comparisons */
    float *fa = malloc(N * sizeof(float));
    float *fb = malloc(N * sizeof(float));
    float *fout = malloc(N * sizeof(float));
    if (fa && fb && fout) {
        for (int i = 0; i < N; i++) {
            fa[i] = (float)i;
            fb[i] = (float)(N/2);
        }
        test_float_comparisons(fa, fb, fout);
    }
    
    /* Verify results */
    int errors = verify_results(out_gt, out_ge, out_lt, out_le, a, b, x, y);
    
    if (errors == 0) {
        printf("All tests passed successfully!\n");
        printf("Reduction result: %d\n", reduction_result);
        
        /* Compute checksums to ensure computations aren't optimized away */
        int checksum = 0;
        for (int i = 0; i < N; i++) {
            checksum += out_gt[i] + out_ge[i] + out_lt[i] + out_le[i];
        }
        printf("Final checksum: %d\n", checksum);
    } else {
        printf("Found %d errors\n", errors);
    }
    
    /* Cleanup */
    free(a); free(b); free(x); free(y);
    free(out_gt); free(out_ge); free(out_lt); free(out_le);
    if (fa) free(fa);
    if (fb) free(fb);
    if (fout) free(fout);
    
    return errors > 0 ? 1 : 0;
}
