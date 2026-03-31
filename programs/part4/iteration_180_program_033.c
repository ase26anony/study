/* test_vector_cond_bitops.c
 * Designed to trigger GCC's vectorizer transformation of comparison
 * operations to bit operations (lines 12216-12233 in tree-vect-stmts.cc)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGN 32

/* Aligned allocations for better vectorization */
static float* alloc_aligned(size_t size) {
    void* ptr;
    if (posix_memalign(&ptr, ALIGN, size * sizeof(float)) != 0) {
        return NULL;
    }
    return (float*)ptr;
}

/* GT_EXPR (>) - Conditional assignment with > comparison */
void test_gt_expr(float* restrict a, float* restrict b, 
                  float* restrict c, float* restrict d, int n) {
    for (int i = 0; i < n; ++i) {
        /* This should generate GT_EXPR comparison */
        if (a[i] > b[i]) {
            c[i] = a[i] * b[i];  /* True path */
        } else {
            c[i] = d[i];         /* False path */
        }
    }
}

/* GE_EXPR (>=) - Conditional reduction with >= comparison */
float test_ge_expr(float* restrict a, float* restrict b, 
                   float* restrict c, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; ++i) {
        /* This should generate GE_EXPR comparison */
        if (a[i] >= b[i]) {
            sum += c[i] * 2.0f;  /* Conditional accumulation */
        } else {
            sum += c[i];         /* Different operation for false */
        }
    }
    return sum;
}

/* LT_EXPR (<) - Masked store with < comparison */
void test_lt_expr(float* restrict a, float* restrict b, 
                  float* restrict c, int n) {
    for (int i = 0; i < n; ++i) {
        /* This should generate LT_EXPR comparison */
        if (a[i] < b[i]) {
            c[i] = a[i] + b[i];  /* True path only */
        }
        /* No else branch - c[i] remains unchanged if condition false */
    }
}

/* LE_EXPR (<=) - Ternary conditional with <= comparison */
void test_le_expr(float* restrict a, float* restrict b, 
                  float* restrict c, float* restrict d, int n) {
    for (int i = 0; i < n; ++i) {
        /* This should generate LE_EXPR comparison */
        c[i] = (a[i] <= b[i]) ? (a[i] - b[i]) : (d[i] * 2.0f);
    }
}

/* Mixed comparisons in same loop to potentially trigger multiple paths */
void test_mixed_comparisons(float* restrict a, float* restrict b,
                           float* restrict c, float* restrict d, int n) {
    for (int i = 0; i < n; ++i) {
        /* Mix of different comparison operators */
        if (a[i] > b[i]) {      /* GT_EXPR */
            c[i] = a[i];
        } else if (a[i] >= b[i]) { /* GE_EXPR */
            c[i] = b[i];
        } else if (a[i] < b[i]) {  /* LT_EXPR */
            c[i] = a[i] + b[i];
        } else if (a[i] <= b[i]) { /* LE_EXPR */
            c[i] = a[i] - b[i];
        } else {
            c[i] = d[i];
        }
    }
}

/* Initialize test data with patterns that create mixed true/false results */
void init_data(float* a, float* b, float* c, float* d, int n) {
    for (int i = 0; i < n; ++i) {
        /* Create varying patterns to ensure both true and false paths */
        a[i] = (float)(i % 128) - 64.0f;      /* Range: -64 to 63 */
        b[i] = (float)((i + 32) % 96) - 48.0f; /* Range: -48 to 47 */
        c[i] = (float)(i * 2);
        d[i] = (float)(i % 64);
    }
}

/* Verify results by comparing with scalar reference implementation */
int verify_results(float* a, float* b, float* c, float* d, int n) {
    float* ref_c = (float*)malloc(n * sizeof(float));
    float ref_sum = 0.0f;
    float test_sum = 0.0f;
    int errors = 0;
    
    /* Scalar reference for GE test */
    for (int i = 0; i < n; ++i) {
        if (a[i] >= b[i]) {
            ref_sum += c[i] * 2.0f;
        } else {
            ref_sum += c[i];
        }
    }
    
    /* Test GE_EXPR function */
    float ge_result = test_ge_expr(a, b, c, n);
    if (fabs(ge_result - ref_sum) > 0.001f) {
        printf("GE_EXPR verification failed: %f != %f\n", ge_result, ref_sum);
        errors++;
    }
    
    free(ref_c);
    return errors;
}

int main() {
    /* Allocate aligned memory for better vectorization */
    float* a = alloc_aligned(N);
    float* b = alloc_aligned(N);
    float* c = alloc_aligned(N);
    float* d = alloc_aligned(N);
    float* e = alloc_aligned(N);
    
    if (!a || !b || !c || !d || !e) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize test data */
    init_data(a, b, c, d, N);
    
    printf("Testing vectorization of comparison operations...\n");
    
    /* Clear output arrays */
    memset(e, 0, N * sizeof(float));
    memset(c, 0, N * sizeof(float));
    
    /* Test each comparison type separately */
    test_gt_expr(a, b, e, d, N);      /* GT_EXPR */
    test_lt_expr(a, b, c, N);         /* LT_EXPR */
    test_le_expr(a, b, e, d, N);      /* LE_EXPR */
    
    /* Test mixed comparisons */
    float* mixed_out = alloc_aligned(N);
    test_mixed_comparisons(a, b, mixed_out, d, N);
    
    /* Verify results */
    int errors = verify_results(a, b, c, d, N);
    
    /* Compute checksum to ensure computations aren't optimized away */
    float checksum = 0.0f;
    for (int i = 0; i < N; ++i) {
        checksum += e[i] + c[i] + mixed_out[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Errors: %d\n", errors);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d); free(e); free(mixed_out);
    
    return errors > 0 ? 1 : 0;
}
