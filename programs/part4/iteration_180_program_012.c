/* test_vector_cond_bitops.c
 * Designed to trigger vectorizer transformation of comparison operations
 * to bit operations in GCC's tree-vect-stmts.cc (lines 12216-12233)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define ALIGN 32

/* Aligned allocation for better vectorization */
static void* aligned_alloc(size_t size) {
    void* ptr;
    if (posix_memalign(&ptr, ALIGN, size) != 0) {
        return NULL;
    }
    return ptr;
}

/* GT_EXPR (>) test - should trigger BIT_NOT_EXPR + BIT_AND_EXPR */
void test_gt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict out) {
    for (int i = 0; i < N; ++i) {
        /* Conditional assignment using > */
        out[i] = (a[i] > b[i]) ? c[i] * 2 : c[i] / 2;
    }
}

/* GE_EXPR (>=) test - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR */
void test_ge_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict out) {
    for (int i = 0; i < N; ++i) {
        /* Conditional assignment using >= */
        out[i] = (a[i] >= b[i]) ? a[i] + b[i] : a[i] - b[i];
    }
}

/* LT_EXPR (<) test - should trigger BIT_NOT_EXPR + BIT_AND_EXPR with swap */
void test_lt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict out) {
    for (int i = 0; i < N; ++i) {
        /* Conditional assignment using < */
        out[i] = (a[i] < b[i]) ? c[i] * 3 : c[i];
    }
}

/* LE_EXPR (<=) test - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
void test_le_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict out) {
    for (int i = 0; i < N; ++i) {
        /* Conditional assignment using <= */
        out[i] = (a[i] <= b[i]) ? a[i] * b[i] : a[i] + c[i];
    }
}

/* Reduction with GT_EXPR */
int test_gt_reduction(int* restrict a, int* restrict b) {
    int sum = 0;
    for (int i = 0; i < N; ++i) {
        /* Conditional reduction using > */
        sum += (a[i] > b[i]) ? a[i] : 0;
    }
    return sum;
}

/* Reduction with GE_EXPR */
int test_ge_reduction(int* restrict a, int* restrict b) {
    int sum = 0;
    for (int i = 0; i < N; ++i) {
        /* Conditional reduction using >= */
        sum += (a[i] >= b[i]) ? b[i] : 1;
    }
    return sum;
}

/* Mixed conditional operations to ensure all paths are taken */
void test_mixed_conditions(float* restrict fa, float* restrict fb, float* restrict out) {
    for (int i = 0; i < N; ++i) {
        /* Mix of different comparison operators */
        if (fa[i] > fb[i]) {
            out[i] = fa[i] * 2.0f;
        } else if (fa[i] >= fb[i] * 0.5f) {
            out[i] = fa[i] + fb[i];
        } else if (fa[i] < fb[i]) {
            out[i] = fa[i] - fb[i];
        } else if (fa[i] <= fb[i] * 1.5f) {
            out[i] = fa[i] / 2.0f;
        } else {
            out[i] = fa[i];
        }
    }
}

/* Initialize arrays with pattern that creates mix of true/false conditions */
void init_arrays(int* a, int* b, int* c, float* fa, float* fb) {
    for (int i = 0; i < N; ++i) {
        /* Create varying patterns to ensure all comparison results occur */
        a[i] = i - N/2;                    /* Range: -512 to 511 */
        b[i] = (i % 3) * 100 - 150;        /* Pattern: -150, -50, 50, -150... */
        c[i] = (i % 5) * 50;               /* Pattern: 0, 50, 100, 150, 200, 0... */
        fa[i] = (float)(i - N/2) * 0.5f;
        fb[i] = (float)(i % 4) * 25.0f;
    }
}

/* Verify results by comparing with reference implementation */
int verify_results(int* out_gt, int* out_ge, int* out_lt, int* out_le,
                   int* a, int* b, int* c) {
    int errors = 0;
    
    for (int i = 0; i < N; ++i) {
        int ref_gt = (a[i] > b[i]) ? c[i] * 2 : c[i] / 2;
        int ref_ge = (a[i] >= b[i]) ? a[i] + b[i] : a[i] - b[i];
        int ref_lt = (a[i] < b[i]) ? c[i] * 3 : c[i];
        int ref_le = (a[i] <= b[i]) ? a[i] * b[i] : a[i] + c[i];
        
        if (out_gt[i] != ref_gt) errors++;
        if (out_ge[i] != ref_ge) errors++;
        if (out_lt[i] != ref_lt) errors++;
        if (out_le[i] != ref_le) errors++;
    }
    
    return errors;
}

int main() {
    /* Allocate aligned arrays for better vectorization */
    int* a = (int*)aligned_alloc(N * sizeof(int));
    int* b = (int*)aligned_alloc(N * sizeof(int));
    int* c = (int*)aligned_alloc(N * sizeof(int));
    int* out_gt = (int*)aligned_alloc(N * sizeof(int));
    int* out_ge = (int*)aligned_alloc(N * sizeof(int));
    int* out_lt = (int*)aligned_alloc(N * sizeof(int));
    int* out_le = (int*)aligned_alloc(N * sizeof(int));
    float* fa = (float*)aligned_alloc(N * sizeof(float));
    float* fb = (float*)aligned_alloc(N * sizeof(float));
    float* fout = (float*)aligned_alloc(N * sizeof(float));
    
    if (!a || !b || !c || !out_gt || !out_ge || !out_lt || !out_le || !fa || !fb || !fout) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with patterns that ensure all comparison results */
    init_arrays(a, b, c, fa, fb);
    
    /* Execute all test functions */
    test_gt_expr(a, b, c, out_gt);
    test_ge_expr(a, b, c, out_ge);
    test_lt_expr(a, b, c, out_lt);
    test_le_expr(a, b, c, out_le);
    
    /* Execute reduction tests */
    int sum_gt = test_gt_reduction(a, b);
    int sum_ge = test_ge_reduction(a, b);
    
    /* Execute mixed floating-point test */
    test_mixed_conditions(fa, fb, fout);
    
    /* Verify results */
    int errors = verify_results(out_gt, out_ge, out_lt, out_le, a, b, c);
    
    /* Compute checksums to ensure computations aren't optimized away */
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += out_gt[i] + out_ge[i] + out_lt[i] + out_le[i] + (int)fout[i];
    }
    checksum += sum_gt + sum_ge;
    
    printf("Test completed with %d errors\n", errors);
    printf("Final checksum: %d\n", checksum);
    
    /* Free allocated memory */
    free(a); free(b); free(c);
    free(out_gt); free(out_ge); free(out_lt); free(out_le);
    free(fa); free(fb); free(fout);
    
    return errors > 0 ? 1 : 0;
}
