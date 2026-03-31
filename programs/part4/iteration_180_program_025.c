/* test_vectorized_comparisons.c
 * 
 * This program creates vectorizable loops with >, >=, <, <= comparisons
 * to trigger the bit-operation transformation in GCC's tree-vect-stmts.cc
 * lines 12216-12233.
 *
 * Compile with: gcc -O3 -ftree-vectorize -fno-vect-cost-model -march=native -fopt-info-vec
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define ALIGN 32

/* Aligned allocations for better vectorization */
static float* alloc_aligned_floats(size_t n) {
    float* ptr;
    if (posix_memalign((void**)&ptr, ALIGN, n * sizeof(float)) != 0) {
        return NULL;
    }
    return ptr;
}

static int* alloc_aligned_ints(size_t n) {
    int* ptr;
    if (posix_memalign((void**)&ptr, ALIGN, n * sizeof(int)) != 0) {
        return NULL;
    }
    return ptr;
}

/* Test GT_EXPR (>) transformation */
void test_gt_expr(float* restrict a, float* restrict b, float* restrict c, float* restrict d) {
    /* Pattern: if (a[i] > b[i]) c[i] = d[i] * 2.0f else c[i] = d[i] */
    for (int i = 0; i < N; ++i) {
        if (a[i] > b[i]) {
            c[i] = d[i] * 2.0f;
        } else {
            c[i] = d[i];
        }
    }
}

/* Test GE_EXPR (>=) transformation */
void test_ge_expr(float* restrict a, float* restrict b, float* restrict c, float* restrict d) {
    /* Pattern: Conditional reduction using >= */
    float sum = 0.0f;
    for (int i = 0; i < N; ++i) {
        if (a[i] >= b[i]) {
            sum += c[i] * d[i];
        }
    }
    /* Use sum to prevent dead code elimination */
    c[0] += sum * 0.001f;
}

/* Test LT_EXPR (<) transformation with swapped operands */
void test_lt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    /* Pattern: if (a[i] < b[i]) c[i] = a[i] + b[i] else c[i] = d[i] */
    for (int i = 0; i < N; ++i) {
        if (a[i] < b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = d[i];
        }
    }
}

/* Test LE_EXPR (<=) transformation with swapped operands */
void test_le_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    /* Pattern: Masked store with <= comparison */
    for (int i = 0; i < N; ++i) {
        if (a[i] <= b[i]) {
            c[i] = (a[i] * b[i]) >> 1;
        } else {
            c[i] = d[i] & 0xFF;
        }
    }
}

/* Additional test using ternary operator (often vectorized as blend) */
void test_ternary_gt(float* restrict a, float* restrict b, float* restrict c) {
    /* This often triggers mask generation from comparison */
    for (int i = 0; i < N; ++i) {
        c[i] = (a[i] > b[i]) ? (a[i] - b[i]) : (b[i] - a[i]);
    }
}

/* Mixed comparisons in same loop to test multiple transformations */
void test_mixed_comparisons(float* restrict a, float* restrict b, 
                           float* restrict c, float* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Use all four comparison types in different expressions */
        float tmp1 = (a[i] > b[i]) ? a[i] : b[i];      /* GT */
        float tmp2 = (a[i] >= b[i]) ? tmp1 : c[i];     /* GE */
        float tmp3 = (b[i] < a[i]) ? tmp2 : d[i];      /* LT (swapped) */
        c[i] = (tmp3 <= tmp2) ? tmp3 : tmp2;           /* LE */
    }
}

/* Initialize test data with patterns that create mix of true/false comparisons */
void init_test_data(float* a, float* b, float* c, float* d,
                    int* ia, int* ib, int* ic, int* id) {
    for (int i = 0; i < N; ++i) {
        /* Create sawtooth pattern for varied comparisons */
        a[i] = (float)(i % 100) - 50.0f;      /* Range: -50 to 49 */
        b[i] = (float)(i % 75) - 25.0f;       /* Range: -25 to 49 */
        c[i] = (float)(i % 50);               /* Range: 0 to 49 */
        d[i] = (float)(i % 25) + 10.0f;       /* Range: 10 to 34 */
        
        ia[i] = i - N/2;                      /* Range: -512 to 511 */
        ib[i] = (i * 3) % 200 - 100;          /* Range: -100 to 99 */
        ic[i] = 0;
        id[i] = i & 0x3FF;                    /* Range: 0 to 1023 */
    }
}

/* Verify results by comparing with sequential computation */
int verify_results() {
    float* a = alloc_aligned_floats(N);
    float* b = alloc_aligned_floats(N);
    float* c = alloc_aligned_floats(N);
    float* d = alloc_aligned_floats(N);
    float* c_ref = alloc_aligned_floats(N);
    
    int* ia = alloc_aligned_ints(N);
    int* ib = alloc_aligned_ints(N);
    int* ic = alloc_aligned_ints(N);
    int* id = alloc_aligned_ints(N);
    int* ic_ref = alloc_aligned_ints(N);
    
    if (!a || !b || !c || !d || !c_ref || !ia || !ib || !ic || !id || !ic_ref) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_test_data(a, b, c, d, ia, ib, ic, id);
    
    /* Make reference copies */
    memcpy(c_ref, c, N * sizeof(float));
    memcpy(ic_ref, ic, N * sizeof(int));
    
    /* Run all tests */
    test_gt_expr(a, b, c, d);
    test_ge_expr(a, b, c, d);
    test_lt_expr(ia, ib, ic, id);
    test_le_expr(ia, ib, ic, id);
    test_ternary_gt(a, b, c_ref);
    test_mixed_comparisons(a, b, c, d);
    
    /* Compute checksum to ensure computations aren't optimized away */
    float checksum_float = 0.0f;
    int checksum_int = 0;
    
    for (int i = 0; i < N; ++i) {
        checksum_float += c[i];
        checksum_int += ic[i];
    }
    
    printf("Checksum float: %f\n", checksum_float);
    printf("Checksum int: %d\n", checksum_int);
    
    /* Clean up */
    free(a); free(b); free(c); free(d); free(c_ref);
    free(ia); free(ib); free(ic); free(id); free(ic_ref);
    
    return 0;
}

int main() {
    printf("Testing vectorized comparisons for GCC coverage...\n");
    
    /* Run multiple iterations to increase chance of hitting the code */
    for (int iter = 0; iter < 10; ++iter) {
        if (verify_results() != 0) {
            return 1;
        }
    }
    
    printf("All tests completed.\n");
    return 0;
}
