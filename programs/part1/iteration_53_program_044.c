/* test-annotate-expr.c
 * This program is designed to trigger the uncovered lines in tree-pretty-print.cc
 * that handle ANNOTATE_EXPR nodes with specific loop annotation kinds.
 * Compile with: gcc -O2 -ftree-vectorize -fdump-tree-vect -fdump-tree-optimized test-annotate-expr.c -o test-annotate-expr
 */

#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
static void use(int val) {
    volatile int sink = val;
    (void)sink;
}

/* External function to create side effects */
extern void external_side_effect(int);

#define SIZE 1024
#define CHUNK 16

int main(void) {
    int i, j, k;
    int result = 0;
    
    /* Arrays for loop computations */
    int a[SIZE], b[SIZE], c[SIZE];
    int partial_sums[CHUNK];
    
    /* Initialize arrays */
    for (i = 0; i < SIZE; i++) {
        a[i] = i % 100;
        b[i] = 0;
        c[i] = 1;
    }
    
    for (i = 0; i < CHUNK; i++) {
        partial_sums[i] = 0;
    }
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC ivdep
     * Loop with potential dependencies that ivdep tells compiler to ignore */
    #pragma GCC ivdep
    for (i = 1; i < SIZE; i++) {
        /* Potential dependency that ivdep tells compiler to ignore */
        a[i] = a[i-1] + i;
    }
    use(a[SIZE-1]);  /* Prevent dead code elimination */
    
    /* 2. annot_expr_vector_kind: #pragma GCC vector
     * Simple vectorizable computation */
    #pragma GCC vector
    for (i = 0; i < SIZE; i++) {
        b[i] = a[i] * 2 + 3;
    }
    use(b[SIZE/2]);
    
    /* 3. annot_expr_parallel_kind: #pragma GCC parallel
     * Independent iterations suitable for parallel execution */
    #pragma GCC parallel
    for (i = 0; i < SIZE; i++) {
        /* Independent accumulation into array elements */
        int idx = i % CHUNK;
        partial_sums[idx] += b[i];
    }
    
    /* Sum partial results */
    for (i = 0; i < CHUNK; i++) {
        result += partial_sums[i];
    }
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll
     * Loop with variable bound - compiler might unroll infinitely */
    int limit = 8;  /* Variable to prevent compile-time unroll determination */
    #pragma GCC unroll
    for (i = 0; i < limit; i++) {
        c[i] = c[i] * (i + 1);
        external_side_effect(c[i]);  /* External call prevents optimization */
    }
    
    /* Nested loop with annotation on inner loop */
    #pragma GCC ivdep
    for (j = 0; j < 10; j++) {
        #pragma GCC vector
        for (i = 0; i < 100; i++) {
            b[i + j*100] += j;
        }
    }
    
    /* Loop inside conditional with annotation */
    if (result > 0) {
        #pragma GCC parallel
        for (i = 0; i < 50; i++) {
            result += i;
        }
    }
    
    /* Final computation and output */
    for (i = 0; i < SIZE; i++) {
        result += b[i];
    }
    
    printf("Result: %d\n", result);
    return 0;
}

/* Dummy implementation of external function */
void external_side_effect(int val) {
    volatile int sink = val;
    (void)sink;
}
