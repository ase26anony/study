/* test_vectorize_cond_bitops.c
 * 
 * This program is designed to trigger auto-vectorization of loops with
 * conditional expressions using >, >=, <, <= operators, which should
 * cause GCC's vectorizer to convert comparisons to bit operations
 * (targeting lines 12216-12233 in tree-vect-stmts.cc).
 *
 * Compile with: gcc -O3 -ftree-vectorize -fno-vect-cost-model -march=native
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define N 1024
#define ALIGN 32

/* Aligned allocations for better vectorization */
static float *a_f, *b_f, *c_f, *x_f, *y_f;
static int *a_i, *b_i, *c_i, *x_i, *y_i;

__attribute__((noinline))
void init_data(void) {
    /* Allocate aligned memory for floating-point arrays */
    posix_memalign((void**)&a_f, ALIGN, N * sizeof(float));
    posix_memalign((void**)&b_f, ALIGN, N * sizeof(float));
    posix_memalign((void**)&c_f, ALIGN, N * sizeof(float));
    posix_memalign((void**)&x_f, ALIGN, N * sizeof(float));
    posix_memalign((void**)&y_f, ALIGN, N * sizeof(float));
    
    /* Allocate aligned memory for integer arrays */
    posix_memalign((void**)&a_i, ALIGN, N * sizeof(int));
    posix_memalign((void**)&b_i, ALIGN, N * sizeof(int));
    posix_memalign((void**)&c_i, ALIGN, N * sizeof(int));
    posix_memalign((void**)&x_i, ALIGN, N * sizeof(int));
    posix_memalign((void**)&y_i, ALIGN, N * sizeof(int));
    
    /* Initialize with varying patterns to ensure mix of true/false comparisons */
    for (int i = 0; i < N; i++) {
        /* Floating-point arrays: a_f increases, b_f oscillates */
        a_f[i] = (float)i;
        b_f[i] = (float)((i % 64) - 32);
        x_f[i] = (float)(i * 2);
        y_f[i] = (float)(i * 3);
        
        /* Integer arrays: similar pattern but with different values */
        a_i[i] = i;
        b_i[i] = (i % 128) - 64;
        x_i[i] = i * 5;
        y_i[i] = i * 7;
    }
}

__attribute__((noinline))
void cleanup_data(void) {
    free(a_f); free(b_f); free(c_f); free(x_f); free(y_f);
    free(a_i); free(b_i); free(c_i); free(x_i); free(y_i);
}

/* Test GT_EXPR (>) transformation to BIT_NOT_EXPR + BIT_AND_EXPR */
__attribute__((noinline))
void test_gt_expr(void) {
    /* Pattern 1: Conditional assignment with > */
    for (int i = 0; i < N; i++) {
        c_f[i] = (a_f[i] > b_f[i]) ? x_f[i] : y_f[i];
    }
    
    /* Pattern 2: Masked operation with > */
    for (int i = 0; i < N; i++) {
        if (a_i[i] > b_i[i]) {
            c_i[i] = a_i[i] * b_i[i];
        } else {
            c_i[i] = a_i[i] + b_i[i];
        }
    }
}

/* Test GE_EXPR (>=) transformation to BIT_NOT_EXPR + BIT_IOR_EXPR */
__attribute__((noinline))
void test_ge_expr(void) {
    /* Pattern 1: Conditional assignment with >= */
    for (int i = 0; i < N; i++) {
        c_f[i] = (a_f[i] >= b_f[i]) ? x_f[i] : y_f[i];
    }
    
    /* Pattern 2: Reduction with >= (forces mask generation) */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += (a_i[i] >= b_i[i]) ? x_i[i] : 0;
    }
    /* Use sum to prevent dead code elimination */
    c_i[0] += sum;
}

/* Test LT_EXPR (<) transformation to BIT_NOT_EXPR + BIT_AND_EXPR with swap */
__attribute__((noinline))
void test_lt_expr(void) {
    /* Pattern 1: Conditional assignment with < */
    for (int i = 0; i < N; i++) {
        c_f[i] = (a_f[i] < b_f[i]) ? x_f[i] : y_f[i];
    }
    
    /* Pattern 2: Complex conditional with < */
    for (int i = 0; i < N; i++) {
        if (a_i[i] < b_i[i]) {
            c_i[i] = a_i[i] - b_i[i];
        } else {
            c_i[i] = b_i[i] - a_i[i];
        }
    }
}

/* Test LE_EXPR (<=) transformation to BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
__attribute__((noinline))
void test_le_expr(void) {
    /* Pattern 1: Conditional assignment with <= */
    for (int i = 0; i < N; i++) {
        c_f[i] = (a_f[i] <= b_f[i]) ? x_f[i] : y_f[i];
    }
    
    /* Pattern 2: Blended operation with <= */
    for (int i = 0; i < N; i++) {
        c_i[i] = (a_i[i] <= b_i[i]) ? (a_i[i] | b_i[i]) : (a_i[i] & b_i[i]);
    }
}

/* Additional test with mixed comparisons in same loop */
__attribute__((noinline))
void test_mixed_comparisons(void) {
    /* This loop contains multiple comparison types */
    for (int i = 0; i < N; i++) {
        float val1 = (a_f[i] > b_f[i]) ? a_f[i] : b_f[i];   /* GT */
        float val2 = (a_f[i] >= b_f[i]) ? val1 : -val1;     /* GE */
        float val3 = (a_f[i] < b_f[i]) ? val2 : val1;       /* LT */
        c_f[i] = (a_f[i] <= b_f[i]) ? val3 : val2;          /* LE */
    }
}

/* Verify results to ensure computations are correct */
__attribute__((noinline))
int verify_results(void) {
    int errors = 0;
    
    /* Recompute one test sequentially to verify */
    for (int i = 0; i < N; i++) {
        float expected = (a_f[i] > b_f[i]) ? x_f[i] : y_f[i];
        if (c_f[i] != expected) {
            errors++;
        }
    }
    
    return errors;
}

int main(void) {
    init_data();
    
    /* Clear output arrays */
    memset(c_f, 0, N * sizeof(float));
    memset(c_i, 0, N * sizeof(int));
    
    /* Execute all test functions */
    test_gt_expr();      /* Should trigger GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR */
    test_ge_expr();      /* Should trigger GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR */
    test_lt_expr();      /* Should trigger LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
    test_le_expr();      /* Should trigger LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
    test_mixed_comparisons(); /* Additional stress test */
    
    /* Verify and print results */
    int errors = verify_results();
    if (errors == 0) {
        printf("All tests passed successfully.\n");
        
        /* Compute checksum to ensure computations aren't optimized away */
        int checksum = 0;
        for (int i = 0; i < N; i++) {
            checksum += c_i[i];
            checksum += (int)c_f[i];
        }
        printf("Final checksum: %d\n", checksum);
    } else {
        printf("Found %d errors in verification.\n", errors);
    }
    
    cleanup_data();
    return errors;
}
