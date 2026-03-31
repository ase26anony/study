/* test_vector_cond_bitops.c
 * 
 * This program is designed to trigger the transformation of comparison
 * operations (GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR) to bit operations
 * (BIT_NOT_EXPR, BIT_AND_EXPR, BIT_IOR_EXPR) during auto-vectorization
 * in GCC's tree-vect-stmts.cc (lines 12216-12233).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGN 32

/* Aligned allocation to help vectorization */
static void* aligned_alloc(size_t align, size_t size) {
    void* ptr;
    if (posix_memalign(&ptr, align, size) != 0) {
        return NULL;
    }
    return ptr;
}

/* Test GT_EXPR (>) transformation to BIT_NOT_EXPR + BIT_AND_EXPR */
void test_gt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Conditional assignment that should vectorize with mask generation */
        c[i] = (a[i] > b[i]) ? (a[i] * 2) : (b[i] / 2);
        
        /* Additional operation to ensure the comparison isn't optimized away */
        d[i] = (a[i] > 0) ? c[i] + 1 : c[i] - 1;
    }
}

/* Test GE_EXPR (>=) transformation to BIT_NOT_EXPR + BIT_IOR_EXPR */
void test_ge_expr(float* restrict a, float* restrict b, float* restrict c, float* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Mixed conditional operations to force mask usage */
        if (a[i] >= b[i]) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] + b[i];
        }
        
        /* Reduction-like pattern to encourage vectorization */
        d[i] += (a[i] >= 0.5f) ? c[i] * 2.0f : c[i] * 0.5f;
    }
}

/* Test LT_EXPR (<) transformation to BIT_NOT_EXPR + BIT_AND_EXPR with swap */
void test_lt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Conditional store with comparison on left side */
        if (a[i] < b[i]) {
            c[i] = a[i] * b[i] + 1;
        } else {
            c[i] = a[i] - b[i];
        }
        
        /* Another conditional to increase complexity */
        d[i] = (b[i] < a[i]) ? c[i] >> 2 : c[i] << 2;
    }
}

/* Test LE_EXPR (<=) transformation to BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
void test_le_expr(float* restrict a, float* restrict b, float* restrict c, float* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Conditional assignment with <= operator */
        c[i] = (a[i] <= b[i]) ? (a[i] + b[i]) : (a[i] - b[i]);
        
        /* Nested conditional to prevent simple optimizations */
        if (b[i] <= a[i]) {
            d[i] = c[i] * 3.0f;
        } else {
            d[i] = c[i] / 3.0f;
        }
    }
}

/* Additional test with mixed comparisons in same loop */
void test_mixed_comparisons(int* restrict a, int* restrict b, 
                           int* restrict c, int* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Use all four comparison types in same loop */
        int tmp = 0;
        tmp += (a[i] > b[i]) ? 1 : 0;   // GT_EXPR
        tmp += (a[i] >= b[i]) ? 2 : 0;  // GE_EXPR
        tmp += (a[i] < b[i]) ? 4 : 0;   // LT_EXPR
        tmp += (a[i] <= b[i]) ? 8 : 0;  // LE_EXPR
        
        c[i] = tmp * a[i];
        d[i] = (tmp > 5) ? b[i] * 2 : b[i] / 2;
    }
}

/* Verification function to ensure computations are correct */
int verify_results(int* a, int* b, int* c, int* d, 
                  float* fa, float* fb, float* fc, float* fd) {
    int errors = 0;
    
    /* Verify GT_EXPR results */
    for (int i = 0; i < N; ++i) {
        int expected_c = (a[i] > b[i]) ? (a[i] * 2) : (b[i] / 2);
        int expected_d = (a[i] > 0) ? (expected_c + 1) : (expected_c - 1);
        
        if (c[i] != expected_c || d[i] != expected_d) {
            errors++;
        }
    }
    
    /* Verify GE_EXPR results */
    for (int i = 0; i < N; ++i) {
        float expected_c = (fa[i] >= fb[i]) ? (fa[i] * fb[i]) : (fa[i] + fb[i]);
        float expected_d = fd[i];  // Note: d is accumulated, not recomputed
        
        if (fc[i] != expected_c) {
            errors++;
        }
    }
    
    return errors;
}

int main() {
    /* Allocate aligned arrays for better vectorization */
    int* a = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* b = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* d = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* e = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* f = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    
    float* fa = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* fb = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* fc = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* fd = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    
    if (!a || !b || !c || !d || !e || !f || !fa || !fb || !fc || !fd) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data with patterns that create mixed comparison results */
    for (int i = 0; i < N; ++i) {
        a[i] = i - N/2;                    /* Range: [-512, 511] */
        b[i] = (i % 3) * 100 - 150;        /* Pattern: -150, -50, 50, -150, ... */
        c[i] = 0;
        d[i] = 0;
        e[i] = 0;
        f[i] = 0;
        
        fa[i] = (float)i / 10.0f;          /* Range: [0.0, 102.3] */
        fb[i] = (float)(i % 20) * 3.0f;    /* Pattern: 0, 3, 6, ..., 57, 0, ... */
        fc[i] = 0.0f;
        fd[i] = (float)(i % 10);
    }
    
    /* Execute all test functions multiple times to increase coverage chance */
    for (int iter = 0; iter < 3; ++iter) {
        test_gt_expr(a, b, c, d);
        test_ge_expr(fa, fb, fc, fd);
        test_lt_expr(a, b, e, f);
        test_le_expr(fa, fb, fc, fd);
        test_mixed_comparisons(a, b, c, d);
    }
    
    /* Verify results to prevent dead code elimination */
    int errors = verify_results(a, b, c, d, fa, fb, fc, fd);
    
    /* Compute checksum for output */
    int64_t checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += c[i] + d[i] + e[i] + f[i];
        checksum += (int64_t)(fc[i] * 1000) + (int64_t)(fd[i] * 1000);
    }
    
    printf("Test completed with %d errors\n", errors);
    printf("Checksum: %ld\n", (long)checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d); free(e); free(f);
    free(fa); free(fb); free(fc); free(fd);
    
    return errors > 0 ? 1 : 0;
}
