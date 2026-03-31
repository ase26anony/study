/* test_vectorized_comparisons.c
 * Designed to trigger GCC's vectorization of comparison operations
 * targeting lines 12216-12233 in tree-vect-stmts.cc
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

/* GT_EXPR (>) test */
void test_gt_expr(int* restrict a, int* restrict b, int* restrict c, int n) {
    for (int i = 0; i < n; ++i) {
        /* Conditional assignment using > operator */
        c[i] = (a[i] > b[i]) ? a[i] * 2 : b[i];
    }
}

/* GE_EXPR (>=) test */
void test_ge_expr(int* restrict a, int* restrict b, int* restrict c, int n) {
    for (int i = 0; i < n; ++i) {
        /* Conditional assignment using >= operator */
        c[i] = (a[i] >= b[i]) ? a[i] + b[i] : a[i] - b[i];
    }
}

/* LT_EXPR (<) test */
void test_lt_expr(int* restrict a, int* restrict b, int* restrict c, int n) {
    for (int i = 0; i < n; ++i) {
        /* Conditional assignment using < operator */
        c[i] = (a[i] < b[i]) ? a[i] * b[i] : a[i] / (b[i] + 1);
    }
}

/* LE_EXPR (<=) test */
void test_le_expr(int* restrict a, int* restrict b, int* restrict c, int n) {
    for (int i = 0; i < n; ++i) {
        /* Conditional assignment using <= operator */
        c[i] = (a[i] <= b[i]) ? a[i] | b[i] : a[i] & b[i];
    }
}

/* Additional test with floating point to ensure different type handling */
void test_float_comparisons(float* restrict fa, float* restrict fb, float* restrict fc, int n) {
    for (int i = 0; i < n; ++i) {
        /* Mix of different comparison operators */
        if (fa[i] > fb[i]) {
            fc[i] = fa[i] * 2.0f;
        } else if (fa[i] <= fb[i]) {
            fc[i] = fb[i] * 0.5f;
        } else {
            fc[i] = fa[i] + fb[i];
        }
    }
}

/* Reduction pattern with comparisons */
int test_reduction_with_comparisons(int* restrict a, int* restrict b, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        /* Conditional increment using >= operator */
        sum += (a[i] >= b[i]) ? a[i] : 0;
        
        /* Additional conditional using < operator */
        if (a[i] < b[i]) {
            sum -= b[i];
        }
    }
    return sum;
}

/* Masked store pattern */
void test_masked_store(int* restrict a, int* restrict b, int* restrict c, int n) {
    for (int i = 0; i < n; ++i) {
        /* Direct conditional store using > operator */
        if (a[i] > b[i]) {
            c[i] = a[i] * 3;
        } else {
            c[i] = b[i] * 2;
        }
    }
}

/* Initialize test data with patterns that create mixed true/false results */
void init_test_data(int* a, int* b, float* fa, float* fb) {
    for (int i = 0; i < N; ++i) {
        /* Create varying patterns for integer arrays */
        a[i] = i;
        b[i] = (i % 3 == 0) ? N/2 : (i % 5 == 0) ? i*2 : i/2;
        
        /* Create varying patterns for float arrays */
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(i % 7) * 2.0f;
    }
}

/* Verify results by comparing with sequential reference implementation */
int verify_results(int* c1, int* c2, int n) {
    for (int i = 0; i < n; ++i) {
        if (c1[i] != c2[i]) {
            return 0;
        }
    }
    return 1;
}

int main() {
    /* Aligned allocations for better vectorization */
    int* a = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* b = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c1 = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c2 = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c3 = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c4 = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    float* fa = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* fb = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* fc = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    
    if (!a || !b || !c1 || !c2 || !c3 || !c4 || !fa || !fb || !fc) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize test data */
    init_test_data(a, b, fa, fb);
    
    /* Clear output arrays */
    memset(c1, 0, N * sizeof(int));
    memset(c2, 0, N * sizeof(int));
    memset(c3, 0, N * sizeof(int));
    memset(c4, 0, N * sizeof(int));
    memset(fc, 0, N * sizeof(float));
    
    printf("Testing vectorized comparisons...\n");
    
    /* Test each comparison operator in separate loops */
    test_gt_expr(a, b, c1, N);  /* > operator */
    test_ge_expr(a, b, c2, N);  /* >= operator */
    test_lt_expr(a, b, c3, N);  /* < operator */
    test_le_expr(a, b, c4, N);  /* <= operator */
    
    /* Test floating point comparisons */
    test_float_comparisons(fa, fb, fc, N);
    
    /* Test reduction pattern */
    int reduction_result = test_reduction_with_comparisons(a, b, N);
    
    /* Test masked store pattern */
    test_masked_store(a, b, c1, N);
    
    /* Compute checksums to ensure computations aren't optimized away */
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += c1[i] + c2[i] + c3[i] + c4[i] + (int)fc[i];
    }
    checksum += reduction_result;
    
    printf("Checksum: %d\n", checksum);
    printf("All tests completed.\n");
    
    /* Cleanup */
    free(a); free(b); free(c1); free(c2); free(c3); free(c4);
    free(fa); free(fb); free(fc);
    
    return 0;
}
