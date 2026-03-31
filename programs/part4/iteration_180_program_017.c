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

/* Aligned allocations for better vectorization */
static float *a_f, *b_f, *c_f, *d_f;
static int *a_i, *b_i, *c_i, *d_i;

/* Initialize test data with patterns that create mixed true/false comparisons */
static void init_data(void) {
    a_f = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    b_f = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    c_f = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    d_f = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    
    a_i = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    b_i = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    c_i = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    d_i = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    
    /* Create data patterns that ensure both true and false comparisons */
    for (int i = 0; i < N; i++) {
        /* Float patterns: a_f increases, b_f oscillates around N/2 */
        a_f[i] = (float)i;
        b_f[i] = (float)((i % 3 == 0) ? N/2 + i%10 : N/2 - i%7);
        
        /* Integer patterns: similar but with integers */
        a_i[i] = i;
        b_i[i] = (i % 3 == 0) ? N/2 + i%10 : N/2 - i%7;
        
        /* Initialize output arrays */
        c_f[i] = 0.0f;
        d_f[i] = 0.0f;
        c_i[i] = 0;
        d_i[i] = 0;
    }
}

/* Clean up allocated memory */
static void cleanup(void) {
    free(a_f); free(b_f); free(c_f); free(d_f);
    free(a_i); free(b_i); free(c_i); free(d_i);
}

/* Test 1: GT_EXPR (>) transformations
 * Pattern: Conditional assignment using > comparison
 * Should trigger: bitop1 = BIT_NOT_EXPR; bitop2 = BIT_AND_EXPR;
 */
static void test_gt_expr(void) {
    /* Float version - conditional assignment */
    for (int i = 0; i < N; i++) {
        if (a_f[i] > b_f[i]) {
            c_f[i] = a_f[i] * b_f[i];  /* Non-trivial operation */
        } else {
            c_f[i] = a_f[i] + b_f[i];
        }
    }
    
    /* Integer version - masked operation */
    for (int i = 0; i < N; i++) {
        /* This ternary form often gets converted to bit operations */
        d_i[i] = (a_i[i] > b_i[i]) ? (a_i[i] & b_i[i]) : (a_i[i] | b_i[i]);
    }
}

/* Test 2: GE_EXPR (>=) transformations
 * Pattern: Conditional reduction and assignment
 * Should trigger: bitop1 = BIT_NOT_EXPR; bitop2 = BIT_IOR_EXPR;
 */
static void test_ge_expr(void) {
    /* Float version - conditional accumulation */
    float sum_f = 0.0f;
    for (int i = 0; i < N; i++) {
        if (a_f[i] >= b_f[i]) {
            sum_f += a_f[i] - b_f[i];
            c_f[i] = a_f[i];  /* Store when condition true */
        }
    }
    
    /* Prevent dead code elimination */
    d_f[0] = sum_f;
    
    /* Integer version - conditional blend */
    for (int i = 0; i < N; i++) {
        c_i[i] = (a_i[i] >= b_i[i]) ? (a_i[i] << 1) : (b_i[i] >> 1);
    }
}

/* Test 3: LT_EXPR (<) transformations  
 * Pattern: Conditional assignment with potential operand swap
 * Should trigger: bitop1 = BIT_NOT_EXPR; bitop2 = BIT_AND_EXPR;
 *                 std::swap(cond_expr0, cond_expr1);
 */
static void test_lt_expr(void) {
    /* Float version - direct comparison */
    for (int i = 0; i < N; i++) {
        if (a_f[i] < b_f[i]) {
            c_f[i] = a_f[i] * 2.0f;
        } else {
            c_f[i] = b_f[i] / 2.0f;
        }
    }
    
    /* Integer version - using ternary operator */
    for (int i = 0; i < N; i++) {
        /* This pattern may cause operand swapping */
        d_i[i] = (a_i[i] < b_i[i]) ? (a_i[i] + b_i[i]) : (a_i[i] - b_i[i]);
    }
}

/* Test 4: LE_EXPR (<=) transformations
 * Pattern: Conditional operation with potential operand swap
 * Should trigger: bitop1 = BIT_NOT_EXPR; bitop2 = BIT_IOR_EXPR;
 *                 std::swap(cond_expr0, cond_expr1);
 */
static void test_le_expr(void) {
    /* Float version - conditional with arithmetic */
    for (int i = 0; i < N; i++) {
        if (a_f[i] <= b_f[i]) {
            c_f[i] = (a_f[i] + b_f[i]) * 0.5f;
        }
    }
    
    /* Integer version - reduction with condition */
    int sum_i = 0;
    for (int i = 0; i < N; i++) {
        /* Conditional increment pattern */
        sum_i += (a_i[i] <= b_i[i]) ? a_i[i] : 0;
    }
    
    /* Prevent dead code elimination */
    d_i[0] = sum_i;
}

/* Combined test that uses all comparison types in one loop
 * This might trigger multiple transformations in one vectorized loop
 */
static void test_combined(void) {
    for (int i = 0; i < N; i++) {
        /* Use all four comparison operators */
        if (a_f[i] > b_f[i]) {
            c_f[i] += 1.0f;
        }
        if (a_f[i] >= b_f[i]) {
            c_f[i] += 2.0f;
        }
        if (a_f[i] < b_f[i]) {
            c_f[i] += 4.0f;
        }
        if (a_f[i] <= b_f[i]) {
            c_f[i] += 8.0f;
        }
    }
}

/* Verify results to ensure computations are not optimized away */
static int verify_results(void) {
    int errors = 0;
    
    /* Simple checksum verification */
    float sum_f = 0.0f;
    int sum_i = 0;
    
    for (int i = 0; i < N; i++) {
        sum_f += c_f[i] + d_f[i];
        sum_i += c_i[i] + d_i[i];
    }
    
    /* Print checksums (non-zero indicates computations happened) */
    printf("Float checksum: %f\n", sum_f);
    printf("Integer checksum: %d\n", sum_i);
    
    return errors;
}

int main(void) {
    printf("Testing vectorization transformations for comparison operators\n");
    printf("Array size: %d elements\n\n", N);
    
    init_data();
    
    printf("Testing GT_EXPR (>) transformations...\n");
    test_gt_expr();
    
    printf("Testing GE_EXPR (>=) transformations...\n");
    test_ge_expr();
    
    printf("Testing LT_EXPR (<) transformations...\n");
    test_lt_expr();
    
    printf("Testing LE_EXPR (<=) transformations...\n");
    test_le_expr();
    
    printf("Testing combined comparisons...\n");
    test_combined();
    
    printf("\nVerifying results...\n");
    int errors = verify_results();
    
    cleanup();
    
    if (errors == 0) {
        printf("\nAll tests completed successfully.\n");
        printf("If compiled with -O3 -ftree-vectorize, this should trigger\n");
        printf("the comparison-to-bit-op transformations in tree-vect-stmts.cc\n");
    }
    
    return errors;
}
