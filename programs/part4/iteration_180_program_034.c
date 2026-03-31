/* test_vector_cond_bitops.c
 * 
 * This program creates vectorizable loops with conditional operations
 * using all four comparison operators (>, >=, <, <=) to trigger the
 * transformation to bit operations in GCC's tree-vect-stmts.cc.
 * 
 * Compile with: gcc -O3 -ftree-vectorize -fno-vect-cost-model -march=native -o test test_vector_cond_bitops.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define N 1024
#define ALIGN 32

/* Aligned allocation for better vectorization */
static void* aligned_alloc(size_t alignment, size_t size) {
    void* ptr;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return NULL;
    }
    return ptr;
}

/* GT_EXPR (>) - Conditional assignment pattern */
void test_gt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    for (int i = 0; i < N; i++) {
        /* This conditional assignment should be transformed to bit operations */
        c[i] = (a[i] > b[i]) ? a[i] * 2 : b[i] / 2;
        
        /* Additional operation to prevent dead code elimination */
        d[i] += (a[i] > b[i]) ? 1 : 0;
    }
}

/* GE_EXPR (>=) - Conditional reduction pattern */
int test_ge_expr(int* restrict a, int* restrict b, int* restrict c) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        /* Conditional reduction - vectorizer may use mask operations */
        sum += (a[i] >= b[i]) ? c[i] : 0;
        
        /* Also use in conditional assignment */
        c[i] = (a[i] >= b[i]) ? a[i] + b[i] : a[i] - b[i];
    }
    return sum;
}

/* LT_EXPR (<) - Masked store pattern */
void test_lt_expr(float* restrict a, float* restrict b, float* restrict c) {
    for (int i = 0; i < N; i++) {
        /* Conditional operation that should trigger bit-op transformation */
        if (a[i] < b[i]) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] + b[i];
        }
    }
}

/* LE_EXPR (<=) - Complex conditional pattern */
void test_le_expr(double* restrict a, double* restrict b, double* restrict c, double* restrict d) {
    for (int i = 0; i < N; i++) {
        /* Multiple uses of the same comparison */
        int cond = (a[i] <= b[i]);
        
        /* Use in different contexts to encourage bit-op transformation */
        c[i] = cond ? a[i] * 3.0 : b[i] * 2.0;
        d[i] = cond ? a[i] - b[i] : b[i] - a[i];
        
        /* Additional conditional to prevent optimization */
        if (a[i] <= b[i]) {
            d[i] += 1.0;
        }
    }
}

/* Mixed comparisons to ensure all paths are exercised */
void test_mixed_comparisons(int* restrict a, int* restrict b, int* restrict c) {
    for (int i = 0; i < N; i++) {
        /* Mix different comparison operators in the same loop */
        if (a[i] > b[i]) {
            c[i] = a[i] * 2;
        } else if (a[i] >= b[i]) {
            c[i] = a[i] + b[i];
        } else if (a[i] < b[i]) {
            c[i] = b[i] - a[i];
        } else { /* a[i] <= b[i] but not >, >=, or < */
            c[i] = a[i] / 2;
        }
    }
}

/* Initialize arrays with varying patterns to ensure both true and false comparisons */
void init_arrays(int* a, int* b, int* c, int* d,
                 float* fa, float* fb, float* fc,
                 double* da, double* db, double* dc, double* dd) {
    for (int i = 0; i < N; i++) {
        /* Create patterns that will produce both true and false comparisons */
        a[i] = i;                     /* 0, 1, 2, 3, ... */
        b[i] = N/2;                   /* Constant 512 */
        c[i] = 0;
        d[i] = i % 10;
        
        fa[i] = (float)(i - N/2);     /* Range from -512 to 511 */
        fb[i] = (float)(i % 100);     /* Range 0-99 */
        fc[i] = 0.0f;
        
        da[i] = (double)i / 10.0;     /* 0.0, 0.1, 0.2, ... */
        db[i] = 5.0;                  /* Constant */
        dc[i] = 0.0;
        dd[i] = 0.0;
    }
}

/* Verify results to ensure computations are correct */
int verify_results(int* c1, int* c2, float* fc, double* dc, double* dd) {
    int errors = 0;
    
    /* Simple checksum verification */
    int sum1 = 0, sum2 = 0;
    float fsum = 0.0f;
    double dsum1 = 0.0, dsum2 = 0.0;
    
    for (int i = 0; i < N; i++) {
        sum1 += c1[i];
        sum2 += c2[i];
        fsum += fc[i];
        dsum1 += dc[i];
        dsum2 += dd[i];
    }
    
    /* Print checksums for manual verification if needed */
    printf("Checksums:\n");
    printf("  GT/GE test arrays: %d, %d\n", sum1, sum2);
    printf("  LT test array: %f\n", fsum);
    printf("  LE test arrays: %f, %f\n", dsum1, dsum2);
    
    return errors;
}

int main() {
    /* Allocate aligned memory for better vectorization */
    int* a = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* b = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c1 = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c2 = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* d = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    
    float* fa = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* fb = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* fc = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    
    double* da = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    double* db = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    double* dc = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    double* dd = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    
    if (!a || !b || !c1 || !c2 || !d || !fa || !fb || !fc || !da || !db || !dc || !dd) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize all arrays */
    init_arrays(a, b, c1, d, fa, fb, fc, da, db, dc, dd);
    
    printf("Testing vectorizable conditionals to trigger bit-op transformations...\n");
    
    /* Test each comparison operator in separate loops */
    test_gt_expr(a, b, c1, d);           /* Tests GT_EXPR (>) */
    
    int sum = test_ge_expr(a, b, c2);    /* Tests GE_EXPR (>=) */
    (void)sum; /* Use variable to prevent optimization */
    
    test_lt_expr(fa, fb, fc);            /* Tests LT_EXPR (<) */
    
    test_le_expr(da, db, dc, dd);        /* Tests LE_EXPR (<=) */
    
    /* Additional test with mixed comparisons */
    test_mixed_comparisons(a, b, c1);
    
    /* Verify results to ensure all computations were performed */
    int errors = verify_results(c1, c2, fc, dc, dd);
    
    /* Clean up */
    free(a); free(b); free(c1); free(c2); free(d);
    free(fa); free(fb); free(fc);
    free(da); free(db); free(dc); free(dd);
    
    if (errors == 0) {
        printf("All tests completed successfully.\n");
        printf("If compiled with -O3 -ftree-vectorize, this should trigger\n");
        printf("the conditional-to-bit-op transformation in tree-vect-stmts.cc\n");
    }
    
    return errors;
}
