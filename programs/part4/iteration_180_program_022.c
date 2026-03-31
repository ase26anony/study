/* test_vector_cond_bitops.c
 * 
 * This program creates vectorizable loops with conditional operations
 * using all four comparison operators (>, >=, <, <=) to trigger the
 * transformation to bit operations in GCC's tree vectorizer.
 * 
 * Compile with: gcc -O3 -ftree-vectorize -fno-vect-cost-model -march=native -fno-tree-slp-vectorize test_vector_cond_bitops.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define ALIGN 32

/* Aligned allocation to help vectorization */
static void* aligned_alloc(size_t alignment, size_t size) {
    void* ptr;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return NULL;
    }
    return ptr;
}

/* GT_EXPR (>) test with conditional assignment pattern */
void test_gt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* This conditional assignment should be transformed to bit operations */
        c[i] = (a[i] > b[i]) ? a[i] * 2 : b[i] / 2;
        
        /* Additional operation to prevent dead code elimination */
        d[i] += (a[i] > b[i]) ? 1 : -1;
    }
}

/* GE_EXPR (>=) test with masked store pattern */
void test_ge_expr(float* restrict a, float* restrict b, float* restrict c) {
    for (int i = 0; i < N; ++i) {
        /* Masked store pattern - only store if condition is true */
        if (a[i] >= b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

/* LT_EXPR (<) test with conditional reduction pattern */
int test_lt_expr(int* restrict a, int* restrict b) {
    int sum = 0;
    for (int i = 0; i < N; ++i) {
        /* Conditional reduction - only add if condition is true */
        sum += (a[i] < b[i]) ? a[i] : 0;
    }
    return sum;
}

/* LE_EXPR (<=) test with blend/select pattern */
void test_le_expr(double* restrict a, double* restrict b, double* restrict c, double* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Complex conditional expression that should trigger bit operation transformation */
        d[i] = (a[i] <= b[i]) ? (c[i] * 2.0) : (c[i] / 2.0);
    }
}

/* Mixed comparison test to ensure all operators are used */
void test_mixed_comparisons(int* restrict a, int* restrict b, int* restrict out) {
    for (int i = 0; i < N; ++i) {
        /* Use all four comparison operators in different expressions */
        int temp = 0;
        if (a[i] > b[i]) temp += 1;      /* GT_EXPR */
        if (a[i] >= b[i]) temp += 2;     /* GE_EXPR */
        if (a[i] < b[i]) temp += 4;      /* LT_EXPR */
        if (a[i] <= b[i]) temp += 8;     /* LE_EXPR */
        out[i] = temp;
    }
}

/* Helper function to initialize arrays with interesting patterns */
void init_arrays(int* a, int* b, float* fa, float* fb, double* da, double* db, double* dc) {
    for (int i = 0; i < N; ++i) {
        /* Create data that will produce mixed true/false results for comparisons */
        a[i] = i - N/2;                     /* Range: -512 to 511 */
        b[i] = (i % 3) * 100 - 150;         /* Pattern: -150, -50, 50, -150, ... */
        
        fa[i] = (float)(i * 0.5f);
        fb[i] = (float)((i % 5) * 20.0f);
        
        da[i] = (double)i * 0.25;
        db[i] = (double)(i % 7) * 10.0;
        dc[i] = (double)(i % 11) * 5.0;
    }
}

/* Reference implementation for validation */
void compute_reference(int* a, int* b, float* fa, float* fb, double* da, double* db, double* dc,
                      int* ref_c, int* ref_d, float* ref_fc, int* ref_sum, double* ref_dd, int* ref_mixed) {
    /* Reference for GT test */
    for (int i = 0; i < N; ++i) {
        ref_c[i] = (a[i] > b[i]) ? a[i] * 2 : b[i] / 2;
        ref_d[i] = (a[i] > b[i]) ? 1 : -1;
    }
    
    /* Reference for GE test */
    for (int i = 0; i < N; ++i) {
        ref_fc[i] = (fa[i] >= fb[i]) ? fa[i] + fb[i] : fa[i] - fb[i];
    }
    
    /* Reference for LT test */
    *ref_sum = 0;
    for (int i = 0; i < N; ++i) {
        *ref_sum += (a[i] < b[i]) ? a[i] : 0;
    }
    
    /* Reference for LE test */
    for (int i = 0; i < N; ++i) {
        ref_dd[i] = (da[i] <= db[i]) ? (dc[i] * 2.0) : (dc[i] / 2.0);
    }
    
    /* Reference for mixed test */
    for (int i = 0; i < N; ++i) {
        int temp = 0;
        if (a[i] > b[i]) temp += 1;
        if (a[i] >= b[i]) temp += 2;
        if (a[i] < b[i]) temp += 4;
        if (a[i] <= b[i]) temp += 8;
        ref_mixed[i] = temp;
    }
}

int main() {
    /* Allocate aligned arrays for better vectorization */
    int* a = aligned_alloc(ALIGN, N * sizeof(int));
    int* b = aligned_alloc(ALIGN, N * sizeof(int));
    int* c = aligned_alloc(ALIGN, N * sizeof(int));
    int* d = aligned_alloc(ALIGN, N * sizeof(int));
    float* fa = aligned_alloc(ALIGN, N * sizeof(float));
    float* fb = aligned_alloc(ALIGN, N * sizeof(float));
    float* fc = aligned_alloc(ALIGN, N * sizeof(float));
    double* da = aligned_alloc(ALIGN, N * sizeof(double));
    double* db = aligned_alloc(ALIGN, N * sizeof(double));
    double* dc = aligned_alloc(ALIGN, N * sizeof(double));
    double* dd = aligned_alloc(ALIGN, N * sizeof(double));
    int* mixed_out = aligned_alloc(ALIGN, N * sizeof(int));
    
    /* Reference arrays */
    int* ref_c = aligned_alloc(ALIGN, N * sizeof(int));
    int* ref_d = aligned_alloc(ALIGN, N * sizeof(int));
    float* ref_fc = aligned_alloc(ALIGN, N * sizeof(float));
    int ref_sum;
    double* ref_dd = aligned_alloc(ALIGN, N * sizeof(double));
    int* ref_mixed = aligned_alloc(ALIGN, N * sizeof(int));
    
    if (!a || !b || !c || !d || !fa || !fb || !fc || !da || !db || !dc || !dd || !mixed_out ||
        !ref_c || !ref_d || !ref_fc || !ref_dd || !ref_mixed) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize d array for accumulation */
    memset(d, 0, N * sizeof(int));
    
    /* Initialize arrays with test data */
    init_arrays(a, b, fa, fb, da, db, dc);
    
    /* Compute reference values */
    compute_reference(a, b, fa, fb, da, db, dc, ref_c, ref_d, ref_fc, &ref_sum, ref_dd, ref_mixed);
    
    /* Run all tests to trigger the vectorizer transformations */
    test_gt_expr(a, b, c, d);          /* Should trigger GT_EXPR -> BIT_NOT_EXPR, BIT_AND_EXPR */
    test_ge_expr(fa, fb, fc);          /* Should trigger GE_EXPR -> BIT_NOT_EXPR, BIT_IOR_EXPR */
    int sum = test_lt_expr(a, b);      /* Should trigger LT_EXPR -> BIT_NOT_EXPR, BIT_AND_EXPR with swap */
    test_le_expr(da, db, dc, dd);      /* Should trigger LE_EXPR -> BIT_NOT_EXPR, BIT_IOR_EXPR with swap */
    test_mixed_comparisons(a, b, mixed_out); /* Mixed use of all operators */
    
    /* Verify results */
    int errors = 0;
    
    /* Check GT test results */
    for (int i = 0; i < N; ++i) {
        if (c[i] != ref_c[i]) {
            errors++;
            if (errors <= 5) {
                printf("GT test error at index %d: got %d, expected %d\n", i, c[i], ref_c[i]);
            }
        }
        if (d[i] != ref_d[i]) {
            errors++;
            if (errors <= 5) {
                printf("GT accumulation error at index %d: got %d, expected %d\n", i, d[i], ref_d[i]);
            }
        }
    }
    
    /* Check GE test results */
    for (int i = 0; i < N; ++i) {
        if (fc[i] != ref_fc[i]) {
            errors++;
            if (errors <= 5) {
                printf("GE test error at index %d: got %f, expected %f\n", i, fc[i], ref_fc[i]);
            }
        }
    }
    
    /* Check LT test result */
    if (sum != ref_sum) {
        errors++;
        printf("LT test error: got sum %d, expected %d\n", sum, ref_sum);
    }
    
    /* Check LE test results */
    for (int i = 0; i < N; ++i) {
        if (dd[i] != ref_dd[i]) {
            errors++;
            if (errors <= 5) {
                printf("LE test error at index %d: got %f, expected %f\n", i, dd[i], ref_dd[i]);
            }
        }
    }
    
    /* Check mixed test results */
    for (int i = 0; i < N; ++i) {
        if (mixed_out[i] != ref_mixed[i]) {
            errors++;
            if (errors <= 5) {
                printf("Mixed test error at index %d: got %d, expected %d\n", i, mixed_out[i], ref_mixed[i]);
            }
        }
    }
    
    /* Clean up */
    free(a); free(b); free(c); free(d);
    free(fa); free(fb); free(fc);
    free(da); free(db); free(dc); free(dd);
    free(mixed_out);
    free(ref_c); free(ref_d); free(ref_fc); free(ref_dd); free(ref_mixed);
    
    if (errors == 0) {
        printf("All tests passed successfully!\n");
        printf("The vectorizer should have transformed comparison operations to bit operations.\n");
        return 0;
    } else {
        printf("Found %d errors\n", errors);
        return 1;
    }
}
