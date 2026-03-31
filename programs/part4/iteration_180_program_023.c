/* test_vector_cond_bitops.c
 * 
 * This program creates vectorizable loops with conditional expressions
 * using >, >=, <, <= operators to trigger the bit-operation transformation
 * in GCC's tree vectorizer (tree-vect-stmts.cc lines 12216-12233).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define ALIGN 32

/* Aligned memory allocation for better vectorization */
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
        /* This conditional will be transformed to bit operations when vectorized */
        if (a[i] > b[i]) {
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] / 2;
        }
        /* Additional computation to prevent dead code elimination */
        d[i] = (a[i] > b[i]) ? (a[i] + b[i]) : (a[i] - b[i]);
    }
}

/* GE_EXPR (>=) test: Conditional reduction and masked store */
int test_ge_expr(int* restrict a, int* restrict b, int* restrict c) {
    int sum = 0;
    for (int i = 0; i < N; ++i) {
        /* Conditional increment - forces mask generation */
        if (a[i] >= b[i]) {
            sum += a[i];
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] + b[i];
        }
    }
    return sum;
}

/* LT_EXPR (<) test: Conditional with swapped operands pattern */
void test_lt_expr(float* restrict a, float* restrict b, float* restrict c) {
    for (int i = 0; i < N; ++i) {
        /* Using < operator which may trigger operand swapping */
        if (a[i] < b[i]) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] / (b[i] + 1.0f);
        }
    }
}

/* LE_EXPR (<=) test: Complex conditional with multiple uses */
void test_le_expr(short* restrict a, short* restrict b, short* restrict c, short* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Using <= operator which may trigger operand swapping */
        short mask = (a[i] <= b[i]) ? 1 : 0;
        c[i] = mask * (a[i] + b[i]);
        d[i] = (1 - mask) * (a[i] - b[i]);
        
        /* Additional conditional to ensure transformation */
        if (a[i] <= b[i]) {
            c[i] += 10;
        }
    }
}

/* Mixed test: Uses all comparison operators in different loops */
void test_mixed_operators(int* restrict a, int* restrict b, int* restrict c) {
    /* Loop 1: GT_EXPR */
    for (int i = 0; i < N/2; ++i) {
        c[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
    
    /* Loop 2: GE_EXPR */
    for (int i = N/2; i < N; ++i) {
        c[i] = (a[i] >= b[i]) ? a[i] * 2 : b[i] * 3;
    }
    
    /* Loop 3: LT_EXPR */
    for (int i = 0; i < N; i += 2) {
        if (a[i] < b[i]) {
            c[i] = -a[i];
        }
    }
    
    /* Loop 4: LE_EXPR */
    for (int i = 1; i < N; i += 2) {
        if (a[i] <= b[i]) {
            c[i] = a[i] + 100;
        }
    }
}

/* Initialize arrays with pattern that creates mix of true/false comparisons */
void init_arrays(int* a, int* b, float* fa, float* fb, short* sa, short* sb) {
    for (int i = 0; i < N; ++i) {
        /* Create varying patterns to ensure all comparison paths are taken */
        a[i] = i;
        b[i] = N/2 - i % 100;  /* Creates mix of > and < conditions */
        
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(N - i) * 0.7f;
        
        sa[i] = (short)(i % 256);
        sb[i] = (short)(128 + i % 128);
    }
}

/* Verify results to ensure computations are correct */
int verify_results(int* c_int, float* c_float, short* c_short, short* d_short) {
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += c_int[i];
        checksum += (int)c_float[i];
        checksum += c_short[i];
        checksum += d_short[i];
    }
    return checksum;
}

int main() {
    /* Allocate aligned memory for better vectorization */
    int* a_int = (int*)aligned_alloc(N * sizeof(int));
    int* b_int = (int*)aligned_alloc(N * sizeof(int));
    int* c_int = (int*)aligned_alloc(N * sizeof(int));
    int* d_int = (int*)aligned_alloc(N * sizeof(int));
    
    float* a_float = (float*)aligned_alloc(N * sizeof(float));
    float* b_float = (float*)aligned_alloc(N * sizeof(float));
    float* c_float = (float*)aligned_alloc(N * sizeof(float));
    
    short* a_short = (short*)aligned_alloc(N * sizeof(short));
    short* b_short = (short*)aligned_alloc(N * sizeof(short));
    short* c_short = (short*)aligned_alloc(N * sizeof(short));
    short* d_short = (short*)aligned_alloc(N * sizeof(short));
    
    if (!a_int || !b_int || !c_int || !d_int || 
        !a_float || !b_float || !c_float ||
        !a_short || !b_short || !c_short || !d_short) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with data that will exercise all comparison paths */
    init_arrays(a_int, b_int, a_float, b_float, a_short, b_short);
    
    /* Clear output arrays */
    memset(c_int, 0, N * sizeof(int));
    memset(d_int, 0, N * sizeof(int));
    memset(c_float, 0, N * sizeof(float));
    memset(c_short, 0, N * sizeof(short));
    memset(d_short, 0, N * sizeof(short));
    
    printf("Testing vectorizable conditionals to trigger bit-op transformation...\n");
    
    /* Execute all test functions to trigger different comparison operators */
    test_gt_expr(a_int, b_int, c_int, d_int);
    
    int sum_ge = test_ge_expr(a_int, b_int, d_int);
    printf("GE_EXPR reduction sum: %d\n", sum_ge);
    
    test_lt_expr(a_float, b_float, c_float);
    
    test_le_expr(a_short, b_short, c_short, d_short);
    
    /* Additional test with mixed operators */
    test_mixed_operators(a_int, b_int, c_int);
    
    /* Verify and print checksum to ensure computations happened */
    int checksum = verify_results(c_int, c_float, c_short, d_short);
    printf("Final checksum: %d\n", checksum);
    
    /* Free allocated memory */
    free(a_int); free(b_int); free(c_int); free(d_int);
    free(a_float); free(b_float); free(c_float);
    free(a_short); free(b_short); free(c_short); free(d_short);
    
    return 0;
}
