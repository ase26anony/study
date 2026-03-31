/* test_vectorized_comparisons.c
 * 
 * This program is designed to trigger auto-vectorization of loops containing
 * conditional statements with comparison operators (>, >=, <, <=) to exercise
 * the transformation of comparisons to bit operations in GCC's tree-vect-stmts.cc.
 * 
 * Compile with: gcc -O3 -ftree-vectorize -fno-vect-cost-model -march=native -fopt-info-vec
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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

/* GT_EXPR (>) - Conditional assignment pattern */
void test_gt_expr(float* __restrict a, float* __restrict b, 
                   float* __restrict c, float* __restrict d, int n) {
    for (int i = 0; i < n; ++i) {
        /* This conditional will be transformed to bit operations when vectorized */
        if (a[i] > b[i]) {
            c[i] = a[i] * 2.0f;
        } else {
            c[i] = b[i] * 0.5f;
        }
        /* Additional computation to prevent dead code elimination */
        d[i] = (a[i] > b[i]) ? c[i] + 1.0f : c[i] - 1.0f;
    }
}

/* GE_EXPR (>=) - Conditional reduction pattern */
float test_ge_expr(float* __restrict a, float* __restrict b, 
                   float* __restrict c, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; ++i) {
        /* Conditional increment - forces mask generation */
        if (a[i] >= b[i]) {
            sum += c[i];
        }
        /* Blend operation using conditional operator */
        c[i] = (a[i] >= b[i]) ? a[i] + b[i] : a[i] - b[i];
    }
    return sum;
}

/* LT_EXPR (<) - Masked store pattern */
void test_lt_expr(float* __restrict a, float* __restrict b, 
                   float* __restrict c, float* __restrict d, int n) {
    for (int i = 0; i < n; ++i) {
        /* This should trigger std::swap(cond_expr0, cond_expr1) in the transformation */
        if (a[i] < b[i]) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] / (b[i] + 1.0f);
        }
        /* Additional conditional to ensure both paths are used */
        d[i] = (a[i] < b[i]) ? c[i] * 2.0f : c[i] * 0.5f;
    }
}

/* LE_EXPR (<=) - Complex conditional pattern */
void test_le_expr(float* __restrict a, float* __restrict b, 
                   float* __restrict c, float* __restrict d, int n) {
    for (int i = 0; i < n; ++i) {
        /* Multiple uses of <= comparison */
        float temp = (a[i] <= b[i]) ? a[i] : b[i];
        c[i] = temp * 3.0f;
        
        /* Nested conditional to create more complex mask usage */
        if (a[i] <= b[i]) {
            d[i] = c[i] + a[i];
        } else {
            d[i] = c[i] - b[i];
        }
    }
}

/* Integer version to test with different data types */
void test_int_comparisons(int* __restrict a, int* __restrict b, 
                          int* __restrict c, int n) {
    /* Mix of different comparison operators */
    for (int i = 0; i < n; ++i) {
        if (a[i] > b[i]) {
            c[i] = a[i] << 1;
        } else if (a[i] >= b[i]) {
            c[i] = a[i] >> 1;
        } else if (a[i] < b[i]) {
            c[i] = a[i] + b[i];
        } else if (a[i] <= b[i]) {
            c[i] = a[i] - b[i];
        }
    }
}

/* Initialize arrays with varying patterns to ensure both true and false comparisons */
void init_arrays(float* a, float* b, float* c, float* d, int n) {
    for (int i = 0; i < n; ++i) {
        a[i] = (float)(i - n/2);          /* Range: -512 to 511 */
        b[i] = (float)(i % 64);           /* Range: 0 to 63 */
        c[i] = (float)(i * 0.1f);         /* Increasing values */
        d[i] = (float)((i % 32) * 2.0f);  /* Patterned values */
    }
}

void init_int_arrays(int* a, int* b, int* c, int n) {
    for (int i = 0; i < n; ++i) {
        a[i] = i * 2;
        b[i] = i + 100;
        c[i] = 0;
    }
}

/* Verification function to ensure computations are correct */
int verify_results(float* c, float* d, int n) {
    float checksum_c = 0.0f;
    float checksum_d = 0.0f;
    
    for (int i = 0; i < n; ++i) {
        checksum_c += c[i];
        checksum_d += d[i];
    }
    
    printf("Checksum C: %f\n", checksum_c);
    printf("Checksum D: %f\n", checksum_d);
    
    /* Simple validation - just ensure values are non-zero */
    return (checksum_c != 0.0f && checksum_d != 0.0f);
}

int main(void) {
    /* Allocate aligned memory for better vectorization */
    float* a = alloc_aligned(N);
    float* b = alloc_aligned(N);
    float* c = alloc_aligned(N);
    float* d = alloc_aligned(N);
    
    int* a_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* b_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    
    if (!a || !b || !c || !d || !a_int || !b_int || !c_int) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    init_arrays(a, b, c, d, N);
    init_int_arrays(a_int, b_int, c_int, N);
    
    printf("Testing GT_EXPR (>)...\n");
    test_gt_expr(a, b, c, d, N);
    
    printf("Testing GE_EXPR (>=)...\n");
    float sum = test_ge_expr(a, b, c, N);
    printf("Reduction sum: %f\n", sum);
    
    printf("Testing LT_EXPR (<)...\n");
    test_lt_expr(a, b, c, d, N);
    
    printf("Testing LE_EXPR (<=)...\n");
    test_le_expr(a, b, c, d, N);
    
    printf("Testing integer comparisons...\n");
    test_int_comparisons(a_int, b_int, c_int, N);
    
    /* Verify results to prevent dead code elimination */
    int valid = verify_results(c, d, N);
    
    /* Free allocated memory */
    free(a); free(b); free(c); free(d);
    free(a_int); free(b_int); free(c_int);
    
    return valid ? 0 : 1;
}
