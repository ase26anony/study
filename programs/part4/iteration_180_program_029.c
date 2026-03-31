/* test_vector_cond_bitops.c
 * 
 * This program creates vectorizable loops with conditional statements
 * using all four comparison operators (>, >=, <, <=) to trigger the
 * transformation to bit operations in GCC's tree vectorizer.
 * 
 * Compile with: gcc -O3 -ftree-vectorize -fno-vect-cost-model -march=native test_vector_cond_bitops.c -o test_vector_cond_bitops
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

/* Test GT_EXPR (>) transformation */
void test_gt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    /* Pattern: Conditional assignment based on > comparison */
    for (int i = 0; i < N; ++i) {
        if (a[i] > b[i]) {
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] / 2;
        }
    }
    
    /* Additional pattern: Masked operation with reduction */
    int sum = 0;
    for (int i = 0; i < N; ++i) {
        if (a[i] > b[i]) {
            sum += d[i];
        }
    }
    c[0] += sum; /* Prevent dead code elimination */
}

/* Test GE_EXPR (>=) transformation */
void test_ge_expr(float* restrict a, float* restrict b, float* restrict c, float* restrict d) {
    /* Pattern: Conditional blend using >= */
    for (int i = 0; i < N; ++i) {
        c[i] = (a[i] >= b[i]) ? a[i] + d[i] : b[i] - d[i];
    }
    
    /* Pattern: Conditional increment with GE */
    float sum = 0.0f;
    for (int i = 0; i < N; ++i) {
        if (a[i] >= 0.5f * b[i]) {
            sum += d[i];
        }
    }
    c[0] += sum; /* Prevent dead code elimination */
}

/* Test LT_EXPR (<) transformation */
void test_lt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    /* Pattern: Conditional assignment based on < comparison */
    for (int i = 0; i < N; ++i) {
        if (a[i] < b[i]) {
            c[i] = a[i] * 3;
        } else {
            c[i] = b[i] * 2;
        }
    }
    
    /* Pattern: Masked store with LT */
    for (int i = 0; i < N; ++i) {
        if (a[i] < b[i]) {
            d[i] = a[i] + b[i];
        }
    }
}

/* Test LE_EXPR (<=) transformation */
void test_le_expr(float* restrict a, float* restrict b, float* restrict c, float* restrict d) {
    /* Pattern: Conditional blend using <= */
    for (int i = 0; i < N; ++i) {
        c[i] = (a[i] <= b[i]) ? a[i] * d[i] : b[i] / d[i];
    }
    
    /* Pattern: Reduction with LE comparison */
    float prod = 1.0f;
    for (int i = 0; i < N; ++i) {
        if (a[i] <= b[i]) {
            prod *= 1.0f + d[i];
        }
    }
    c[0] *= prod; /* Prevent dead code elimination */
}

/* Mixed test with all operators in different loops */
void test_mixed_operators(int* restrict a, int* restrict b, int* restrict c) {
    /* GT in one loop */
    for (int i = 0; i < N; ++i) {
        c[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
    
    /* GE in another loop */
    for (int i = 0; i < N; ++i) {
        if (a[i] >= b[i]) {
            c[i] += a[i];
        }
    }
    
    /* LT in another loop */
    for (int i = 0; i < N; ++i) {
        c[i] = (a[i] < b[i]) ? -a[i] : -b[i];
    }
    
    /* LE in another loop */
    for (int i = 0; i < N; ++i) {
        if (a[i] <= b[i]) {
            c[i] *= 2;
        }
    }
}

/* Initialize arrays with varying patterns to ensure both true and false comparisons */
void init_arrays(int* a, int* b, int* c, float* fa, float* fb, float* fc, float* fd) {
    for (int i = 0; i < N; ++i) {
        /* Create patterns that will yield mixed comparison results */
        a[i] = i - N/2;              /* Range: [-N/2, N/2-1] */
        b[i] = (i % 3) * 100;        /* Values: 0, 100, 200 */
        c[i] = 0;
        
        fa[i] = (float)(i % 10) * 0.5f;
        fb[i] = (float)(i % 7) * 0.7f;
        fc[i] = 0.0f;
        fd[i] = (float)(i % 5) * 0.3f + 0.1f; /* Avoid division by zero */
    }
}

/* Verify results by comparing with a simple reference implementation */
int verify_results(int* a, int* b, int* c, float* fa, float* fb, float* fc) {
    int errors = 0;
    
    /* Simple reference for GT test */
    int ref_c[N];
    for (int i = 0; i < N; ++i) {
        ref_c[i] = (a[i] > b[i]) ? a[i] * 2 : b[i] / 2;
    }
    
    /* Check first N/4 elements as representative */
    for (int i = 0; i < N/4; ++i) {
        if (c[i] != ref_c[i]) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at index %d: expected %d, got %d\n", i, ref_c[i], c[i]);
            }
        }
    }
    
    return errors;
}

int main() {
    /* Allocate aligned memory for better vectorization */
    int* a = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* b = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* d = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    
    float* fa = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* fb = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* fc = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* fd = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    
    if (!a || !b || !c || !d || !fa || !fb || !fc || !fd) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with varying data patterns */
    init_arrays(a, b, c, fa, fb, fc, fd);
    
    /* Initialize d array */
    for (int i = 0; i < N; ++i) {
        d[i] = (i % 4) * 50;
    }
    
    printf("Testing GT_EXPR (> operator)...\n");
    test_gt_expr(a, b, c, d);
    
    printf("Testing GE_EXPR (>= operator)...\n");
    test_ge_expr(fa, fb, fc, fd);
    
    printf("Testing LT_EXPR (< operator)...\n");
    test_lt_expr(a, b, c, d);
    
    printf("Testing LE_EXPR (<= operator)...\n");
    test_le_expr(fa, fb, fc, fd);
    
    printf("Testing mixed operators...\n");
    test_mixed_operators(a, b, c);
    
    /* Verify and compute checksum */
    int errors = verify_results(a, b, c, fa, fb, fc);
    
    /* Compute checksums to ensure computations aren't optimized away */
    int int_sum = 0;
    float float_sum = 0.0f;
    
    for (int i = 0; i < N; ++i) {
        int_sum += c[i];
        float_sum += fc[i];
    }
    
    printf("Integer checksum: %d\n", int_sum);
    printf("Float checksum: %f\n", float_sum);
    printf("Verification errors: %d\n", errors);
    
    /* Clean up */
    free(a); free(b); free(c); free(d);
    free(fa); free(fb); free(fc); free(fd);
    
    return errors > 0 ? 1 : 0;
}
