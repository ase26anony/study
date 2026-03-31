/* test_vector_cond_bitops.c
 * 
 * This program creates vectorizable loops with conditional operations
 * using all four comparison operators (>, >=, <, <=) to trigger the
 * transformation in tree-vect-stmts.cc that converts comparisons to
 * bit operations.
 * 
 * Compile with: gcc -O3 -ftree-vectorize -fno-vect-cost-model -march=native
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

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

/* GT_EXPR (>) test: Conditional assignment based on a[i] > b[i] */
void test_gt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* This conditional will be transformed to bit operations */
        if (a[i] > b[i]) {
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] / 2;
        }
        /* Additional computation to prevent optimization */
        d[i] = (a[i] > b[i]) ? (a[i] + b[i]) : (a[i] - b[i]);
    }
}

/* GE_EXPR (>=) test: Conditional reduction and masked store */
int test_ge_expr(float* restrict a, float* restrict b, float* restrict c) {
    float sum = 0.0f;
    for (int i = 0; i < N; ++i) {
        /* Conditional reduction - may use mask generation */
        if (a[i] >= b[i]) {
            sum += a[i] * b[i];
            c[i] = a[i];
        } else {
            c[i] = b[i];
        }
    }
    /* Return integer to prevent dead code elimination */
    return (int)sum;
}

/* LT_EXPR (<) test: Conditional blend with swapped logic */
void test_lt_expr(int16_t* restrict a, int16_t* restrict b, 
                  int16_t* restrict c, int16_t* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* This should trigger the std::swap in the uncovered code */
        if (a[i] < b[i]) {
            c[i] = a[i] + b[i];
            d[i] = a[i] * 3;
        } else {
            c[i] = a[i] - b[i];
            d[i] = b[i] * 2;
        }
    }
}

/* LE_EXPR (<=) test: Complex conditional with multiple uses */
int test_le_expr(double* restrict a, double* restrict b, 
                 double* restrict c, double* restrict d) {
    double sum = 0.0;
    for (int i = 0; i < N; ++i) {
        /* Multiple conditional uses to encourage bit operation transformation */
        int cond = (a[i] <= b[i]);
        if (cond) {
            c[i] = a[i] * b[i];
            sum += c[i];
        } else {
            c[i] = a[i] / (b[i] + 1.0);
        }
        /* Another conditional in the same loop */
        d[i] = (a[i] <= b[i]) ? (a[i] - b[i]) : (b[i] - a[i]);
    }
    return (int)sum;
}

/* Mixed test: Uses all operators in different loops */
void test_mixed(int* restrict a, int* restrict b, 
                int* restrict c, int* restrict d) {
    /* Loop 1: GT and GE mixed */
    for (int i = 0; i < N/2; ++i) {
        c[i] = (a[i] > b[i]) ? a[i] : b[i];
        d[i] = (a[i] >= b[N-i-1]) ? (a[i] + b[N-i-1]) : (a[i] - b[N-i-1]);
    }
    
    /* Loop 2: LT and LE mixed */
    for (int i = N/2; i < N; ++i) {
        if (a[i] < b[i]) {
            c[i] = a[i] * 2;
        }
        if (a[i] <= b[N-i-1]) {
            d[i] = b[N-i-1] * 3;
        }
    }
}

/* Initialize arrays with pattern that creates mix of true/false conditions */
void init_arrays(int* a, int* b, float* fa, float* fb, 
                 int16_t* sa, int16_t* sb, double* da, double* db) {
    for (int i = 0; i < N; ++i) {
        /* Create varying patterns to ensure all comparison paths are taken */
        a[i] = i;
        b[i] = N/2 - i % 100;  /* Creates mix of > and < conditions */
        
        fa[i] = (float)(i * 1.5f);
        fb[i] = (float)(N - i);
        
        sa[i] = (int16_t)(i % 256);
        sb[i] = (int16_t)((i + 128) % 256);
        
        da[i] = (double)i * 0.7;
        db[i] = (double)(i % 50) * 2.0;
    }
}

/* Verify results to prevent dead code elimination */
int verify_results(int* c1, int* c2, float* c3, int16_t* c4, 
                   double* c5, double* c6, int* c7, int* c8) {
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += c1[i] + c2[i] + (int)c3[i] + c4[i] + 
                   (int)c5[i] + (int)c6[i] + c7[i] + c8[i];
    }
    return checksum;
}

int main() {
    /* Allocate aligned arrays for better vectorization */
    int* a_int = (int*)aligned_alloc(N * sizeof(int));
    int* b_int = (int*)aligned_alloc(N * sizeof(int));
    int* c1 = (int*)aligned_alloc(N * sizeof(int));
    int* c2 = (int*)aligned_alloc(N * sizeof(int));
    int* d1 = (int*)aligned_alloc(N * sizeof(int));
    int* d2 = (int*)aligned_alloc(N * sizeof(int));
    
    float* a_float = (float*)aligned_alloc(N * sizeof(float));
    float* b_float = (float*)aligned_alloc(N * sizeof(float));
    float* c3 = (float*)aligned_alloc(N * sizeof(float));
    
    int16_t* a_short = (int16_t*)aligned_alloc(N * sizeof(int16_t));
    int16_t* b_short = (int16_t*)aligned_alloc(N * sizeof(int16_t));
    int16_t* c4 = (int16_t*)aligned_alloc(N * sizeof(int16_t));
    int16_t* d4 = (int16_t*)aligned_alloc(N * sizeof(int16_t));
    
    double* a_double = (double*)aligned_alloc(N * sizeof(double));
    double* b_double = (double*)aligned_alloc(N * sizeof(double));
    double* c5 = (double*)aligned_alloc(N * sizeof(double));
    double* d5 = (double*)aligned_alloc(N * sizeof(double));
    
    if (!a_int || !b_int || !c1 || !c2 || !d1 || !d2 ||
        !a_float || !b_float || !c3 ||
        !a_short || !b_short || !c4 || !d4 ||
        !a_double || !b_double || !c5 || !d5) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize all arrays */
    init_arrays(a_int, b_int, a_float, b_float, 
                a_short, b_short, a_double, b_double);
    
    /* Execute all test functions to trigger the different comparison operators */
    test_gt_expr(a_int, b_int, c1, d1);
    
    int sum_ge = test_ge_expr(a_float, b_float, c3);
    
    test_lt_expr(a_short, b_short, c4, d4);
    
    int sum_le = test_le_expr(a_double, b_double, c5, d5);
    
    test_mixed(a_int, b_int, c2, d2);
    
    /* Use results to prevent dead code elimination */
    int checksum = verify_results(c1, c2, c3, c4, c5, d5, d1, d2);
    
    /* Add reduction results to checksum */
    checksum += sum_ge + sum_le;
    
    printf("Checksum: %d\n", checksum);
    printf("All conditional tests executed.\n");
    
    /* Cleanup */
    free(a_int); free(b_int); free(c1); free(c2); free(d1); free(d2);
    free(a_float); free(b_float); free(c3);
    free(a_short); free(b_short); free(c4); free(d4);
    free(a_double); free(b_double); free(c5); free(d5);
    
    return 0;
}
