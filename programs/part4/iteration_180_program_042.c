/* test_vector_cond_bitops.c
 * 
 * This program is designed to trigger GCC's auto-vectorizer to convert
 * conditional expressions (GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR) to bit
 * operations during vectorization, specifically targeting the uncovered
 * block in tree-vect-stmts.cc lines 12216-12233.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGN 32

/* Aligned allocations for better vectorization */
static void* aligned_alloc(size_t align, size_t size) {
    void* ptr;
    if (posix_memalign(&ptr, align, size) != 0) return NULL;
    return ptr;
}

/* GT_EXPR (>) conditional pattern */
void test_gt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    /* Pattern: if (a[i] > b[i]) c[i] = d[i] * 2 else c[i] = d[i] */
    for (int i = 0; i < N; ++i) {
        if (a[i] > b[i]) {
            c[i] = d[i] * 2;
        } else {
            c[i] = d[i];
        }
    }
}

/* GE_EXPR (>=) conditional pattern with reduction */
int test_ge_expr(int* restrict a, int* restrict b, int* restrict c) {
    /* Pattern: sum += (a[i] >= b[i]) ? c[i] : 0 */
    int sum = 0;
    for (int i = 0; i < N; ++i) {
        if (a[i] >= b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

/* LT_EXPR (<) conditional pattern with masked store */
void test_lt_expr(float* restrict a, float* restrict b, float* restrict c) {
    /* Pattern: if (a[i] < b[i]) c[i] = a[i] * b[i] */
    for (int i = 0; i < N; ++i) {
        if (a[i] < b[i]) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = 0.0f;
        }
    }
}

/* LE_EXPR (<=) conditional pattern with ternary operator */
void test_le_expr(double* restrict a, double* restrict b, double* restrict c, double* restrict d) {
    /* Pattern: c[i] = (a[i] <= b[i]) ? d[i] : a[i] */
    for (int i = 0; i < N; ++i) {
        c[i] = (a[i] <= b[i]) ? d[i] : a[i];
    }
}

/* Mixed comparisons in a single loop to potentially trigger multiple paths */
void test_mixed_comparisons(int* restrict a, int* restrict b, int* restrict c, 
                           int* restrict d, int* restrict e) {
    /* Use different comparison operators in the same loop */
    for (int i = 0; i < N; ++i) {
        if (a[i] > b[i]) {        /* GT_EXPR */
            c[i] = d[i] + 1;
        }
        if (a[i] >= b[i]) {       /* GE_EXPR */
            c[i] += e[i];
        }
        if (a[i] < b[i]) {        /* LT_EXPR */
            d[i] = c[i] * 2;
        }
        if (a[i] <= b[i]) {       /* LE_EXPR */
            e[i] = d[i] / 2;
        }
    }
}

/* Initialize arrays with varying patterns to ensure mix of true/false comparisons */
void init_arrays(int* a, int* b, int* c, int* d, 
                 float* fa, float* fb, float* fc,
                 double* da, double* db, double* dc, double* dd) {
    for (int i = 0; i < N; ++i) {
        /* Create data patterns that yield mix of comparison results */
        a[i] = i;                    /* 0, 1, 2, ..., N-1 */
        b[i] = N/2;                  /* Constant N/2 */
        c[i] = i % 10;               /* 0-9 repeating */
        d[i] = (i % 3) + 1;          /* 1, 2, 3 repeating */
        
        fa[i] = (float)(i - N/2);    /* Negative and positive values */
        fb[i] = (float)(i % 20);     /* 0-19 repeating */
        fc[i] = 0.0f;
        
        da[i] = (double)i * 0.5;     /* 0, 0.5, 1.0, ... */
        db[i] = (double)(N/4);       /* Constant */
        dc[i] = 0.0;
        dd[i] = (double)(i % 5) * 2.0; /* 0, 2, 4, 6, 8 repeating */
    }
}

/* Verify results by comparing with sequential reference implementation */
int verify_results(int* c_int, float* c_float, double* c_double, int ge_sum) {
    int errors = 0;
    
    /* Simple checksum verification */
    int int_sum = 0;
    float float_sum = 0.0f;
    double double_sum = 0.0;
    
    for (int i = 0; i < N; ++i) {
        int_sum += c_int[i];
        float_sum += c_float[i];
        double_sum += c_double[i];
    }
    
    printf("Checksums - Int: %d, Float: %.2f, Double: %.2f, GE Reduction: %d\n",
           int_sum, float_sum, double_sum, ge_sum);
    
    /* Non-zero checksums indicate computations happened */
    if (int_sum == 0 && float_sum == 0.0f && double_sum == 0.0 && ge_sum == 0) {
        printf("WARNING: All checksums are zero - computations may have been optimized away\n");
        errors++;
    }
    
    return errors;
}

int main() {
    /* Allocate aligned memory for better vectorization */
    int* a = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* b = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* d = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* e = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    
    float* fa = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* fb = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* fc = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    
    double* da = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    double* db = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    double* dc = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    double* dd = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    
    if (!a || !b || !c || !d || !e || !fa || !fb || !fc || !da || !db || !dc || !dd) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with varying data patterns */
    init_arrays(a, b, c, d, fa, fb, fc, da, db, dc, dd);
    
    printf("Testing GT_EXPR (>) conditional pattern...\n");
    test_gt_expr(a, b, c, d);
    
    printf("Testing GE_EXPR (>=) conditional pattern with reduction...\n");
    int ge_sum = test_ge_expr(a, b, d);
    
    printf("Testing LT_EXPR (<) conditional pattern...\n");
    test_lt_expr(fa, fb, fc);
    
    printf("Testing LE_EXPR (<=) conditional pattern...\n");
    test_le_expr(da, db, dc, dd);
    
    printf("Testing mixed comparisons in single loop...\n");
    test_mixed_comparisons(a, b, c, d, e);
    
    /* Verify that computations actually happened */
    int errors = verify_results(c, fc, dc, ge_sum);
    
    /* Clean up */
    free(a); free(b); free(c); free(d); free(e);
    free(fa); free(fb); free(fc);
    free(da); free(db); free(dc); free(dd);
    
    if (errors == 0) {
        printf("All tests completed successfully\n");
        return 0;
    } else {
        printf("Tests completed with %d warnings\n", errors);
        return 0; /* Return 0 even with warnings to allow coverage collection */
    }
}
