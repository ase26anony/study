/* test_vector_cond_bitops.c
 * 
 * This program is designed to trigger auto-vectorization of loops with
 * conditional statements using comparison operators (>, >=, <, <=) that
 * should be transformed to bit operations (BIT_NOT_EXPR, BIT_AND_EXPR, BIT_IOR_EXPR)
 * in GCC's tree-vect-stmts.cc (lines 12216-12233).
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define N 1024
#define ALIGN 32

/* Aligned allocations for better vectorization */
static void* aligned_alloc(size_t align, size_t size) {
    void* ptr;
    if (posix_memalign(&ptr, align, size) != 0) return NULL;
    return ptr;
}

/* Test function for GT_EXPR (>) */
void test_gt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    /* Pattern: Conditional assignment using > comparison
     * Should trigger GT_EXPR -> BIT_NOT_EXPR, BIT_AND_EXPR transformation */
    for (int i = 0; i < N; ++i) {
        if (a[i] > b[i]) {
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] / 2;
        }
    }
    
    /* Additional pattern: Masked operation with > */
    for (int i = 0; i < N; ++i) {
        d[i] = (a[i] > b[i]) ? (a[i] + b[i]) : (a[i] - b[i]);
    }
}

/* Test function for GE_EXPR (>=) */
void test_ge_expr(float* restrict a, float* restrict b, float* restrict c, float* restrict d) {
    /* Pattern: Conditional assignment using >= comparison
     * Should trigger GE_EXPR -> BIT_NOT_EXPR, BIT_IOR_EXPR transformation */
    for (int i = 0; i < N; ++i) {
        if (a[i] >= b[i]) {
            c[i] = a[i] * 3.0f;
        } else {
            c[i] = b[i] * 0.5f;
        }
    }
    
    /* Pattern: Reduction with >= condition */
    float sum = 0.0f;
    for (int i = 0; i < N; ++i) {
        sum += (a[i] >= b[i]) ? a[i] : 0.0f;
    }
    d[0] = sum;
}

/* Test function for LT_EXPR (<) */
void test_lt_expr(int64_t* restrict a, int64_t* restrict b, int64_t* restrict c, int64_t* restrict d) {
    /* Pattern: Conditional assignment using < comparison
     * Should trigger LT_EXPR -> BIT_NOT_EXPR, BIT_AND_EXPR with operand swap */
    for (int i = 0; i < N; ++i) {
        if (a[i] < b[i]) {
            c[i] = a[i] * 5;
        } else {
            c[i] = b[i] * 3;
        }
    }
    
    /* Pattern: Conditional store with < */
    for (int i = 0; i < N; ++i) {
        d[i] = (a[i] < b[i]) ? (a[i] | b[i]) : (a[i] & b[i]);
    }
}

/* Test function for LE_EXPR (<=) */
void test_le_expr(double* restrict a, double* restrict b, double* restrict c, double* restrict d) {
    /* Pattern: Conditional assignment using <= comparison
     * Should trigger LE_EXPR -> BIT_NOT_EXPR, BIT_IOR_EXPR with operand swap */
    for (int i = 0; i < N; ++i) {
        if (a[i] <= b[i]) {
            c[i] = a[i] * 2.5;
        } else {
            c[i] = b[i] * 1.5;
        }
    }
    
    /* Pattern: Blend operation with <= */
    for (int i = 0; i < N; ++i) {
        d[i] = (a[i] <= b[i]) ? (a[i] + 1.0) : (b[i] - 1.0);
    }
}

/* Mixed test with all comparison types in one loop */
void test_mixed_comparisons(int* restrict a, int* restrict b, 
                           int* restrict c, int* restrict d,
                           int* restrict e, int* restrict f) {
    /* This loop contains multiple comparison types that might be
     * vectorized together, increasing chances of hitting the target code */
    for (int i = 0; i < N; ++i) {
        /* GT_EXPR */
        int cond1 = (a[i] > b[i]) ? 1 : 0;
        /* GE_EXPR */
        int cond2 = (a[i] >= b[i]) ? 2 : 0;
        /* LT_EXPR */
        int cond3 = (a[i] < b[i]) ? 3 : 0;
        /* LE_EXPR */
        int cond4 = (a[i] <= b[i]) ? 4 : 0;
        
        c[i] = cond1 + cond2;
        d[i] = cond3 + cond4;
        e[i] = (cond1 && cond2) ? a[i] : b[i];
        f[i] = (cond3 || cond4) ? a[i] * b[i] : a[i] + b[i];
    }
}

/* Initialize arrays with varying patterns to ensure both true and false comparisons */
void init_arrays(int* a, int* b, float* fa, float* fb, 
                 int64_t* la, int64_t* lb, double* da, double* db) {
    for (int i = 0; i < N; ++i) {
        /* Create mixed true/false conditions */
        a[i] = i;
        b[i] = N/2;
        
        fa[i] = (float)(i * 1.5f);
        fb[i] = (float)(N - i);
        
        la[i] = (int64_t)(i * 2);
        lb[i] = (int64_t)(i + 100);
        
        da[i] = (double)i * 0.7;
        db[i] = (double)(i % 100) * 1.2;
    }
}

/* Verify results to prevent dead code elimination */
int verify_results(int* c1, int* c2, float* c3, float* c4,
                   int64_t* c5, int64_t* c6, double* c7, double* c8) {
    int checksum = 0;
    
    for (int i = 0; i < N; ++i) {
        checksum += c1[i] ^ c2[i];
        checksum += (int)c3[i];
        checksum += (int)c4[0];
        checksum += (int)(c5[i] & 0xFFFFFFFF);
        checksum += (int)(c6[i] & 0xFFFFFFFF);
        checksum += (int)c7[i];
        checksum += (int)c8[i];
    }
    
    return checksum;
}

int main() {
    /* Allocate aligned memory for better vectorization */
    int* a = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* b = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c1 = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c2 = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c3 = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c4 = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c5 = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c6 = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    
    float* fa = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* fb = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* fc1 = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* fc2 = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    
    int64_t* la = (int64_t*)aligned_alloc(ALIGN, N * sizeof(int64_t));
    int64_t* lb = (int64_t*)aligned_alloc(ALIGN, N * sizeof(int64_t));
    int64_t* lc1 = (int64_t*)aligned_alloc(ALIGN, N * sizeof(int64_t));
    int64_t* lc2 = (int64_t*)aligned_alloc(ALIGN, N * sizeof(int64_t));
    
    double* da = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    double* db = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    double* dc1 = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    double* dc2 = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    
    if (!a || !b || !c1 || !c2 || !c3 || !c4 || !c5 || !c6 ||
        !fa || !fb || !fc1 || !fc2 ||
        !la || !lb || !lc1 || !lc2 ||
        !da || !db || !dc1 || !dc2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with varying data */
    init_arrays(a, b, fa, fb, la, lb, da, db);
    
    /* Execute all test functions to trigger different comparison transformations */
    test_gt_expr(a, b, c1, c2);
    test_ge_expr(fa, fb, fc1, fc2);
    test_lt_expr(la, lb, lc1, lc2);
    test_le_expr(da, db, dc1, dc2);
    test_mixed_comparisons(a, b, c3, c4, c5, c6);
    
    /* Verify results to ensure computations aren't optimized away */
    int result = verify_results(c1, c2, fc1, fc2, lc1, lc2, dc1, dc2);
    
    /* Use result to affect program output */
    printf("Test completed with checksum: %d\n", result);
    
    /* Cleanup */
    free(a); free(b); free(c1); free(c2); free(c3); free(c4); free(c5); free(c6);
    free(fa); free(fb); free(fc1); free(fc2);
    free(la); free(lb); free(lc1); free(lc2);
    free(da); free(db); free(dc1); free(dc2);
    
    return (result != 0) ? 0 : 1;
}
