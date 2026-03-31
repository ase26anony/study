/* test-annotate-expr.c
 * This program is designed to trigger the uncovered lines in tree-pretty-print.cc
 * related to ANNOTATE_EXPR nodes with loop-specific annotation kinds.
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
#define CHUNK 8

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
     * Loop with potential dependencies that we assert are safe */
    #pragma GCC ivdep
    for (i = 1; i < SIZE; i++) {
        /* Potential dependency broken by ivdep pragma */
        a[i] = a[i-1] + a[i];
    }
    use(a[SIZE-1]);
    
    /* 2. annot_expr_vector_kind: #pragma GCC vector
     * Simple vectorizable computation */
    #pragma GCC vector
    for (i = 0; i < SIZE; i++) {
        b[i] = a[i] * 2 + 7;
    }
    use(b[SIZE/2]);
    
    /* 3. annot_expr_parallel_kind: #pragma GCC parallel
     * Independent iterations accumulating into thread-local storage */
    #pragma GCC parallel
    for (i = 0; i < SIZE; i++) {
        /* Simulate thread-local accumulation */
        partial_sums[i % CHUNK] += b[i] * 3;
    }
    
    /* Combine partial sums */
    for (i = 0; i < CHUNK; i++) {
        result += partial_sums[i];
        external_side_effect(partial_sums[i]);
    }
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll
     * Loop with variable bound - compiler might unroll */
    int limit = 16;
    #pragma GCC unroll
    for (k = 0; k < limit; k++) {
        c[k] = result * k;
        /* Volatile store to prevent dead code elimination */
        volatile int vsink = c[k];
        (void)vsink;
    }
    
    /* Nested loop with annotation on inner loop */
    for (j = 0; j < 4; j++) {
        #pragma GCC ivdep
        for (i = 0; i < 100; i++) {
            a[i] += j;
        }
    }
    
    /* Conditional block with annotated loop */
    if (result > 0) {
        #pragma GCC vector
        for (i = 0; i < 50; i++) {
            b[i] = a[i] / 2;
        }
    } else {
        #pragma GCC parallel
        for (i = 0; i < 50; i++) {
            b[i] = a[i] * 2;
        }
    }
    
    /* Final computation and output */
    int final_sum = 0;
    for (i = 0; i < SIZE; i++) {
        final_sum += a[i] + b[i];
    }
    for (i = 0; i < limit; i++) {
        final_sum += c[i];
    }
    
    printf("Result: %d\n", final_sum);
    return 0;
}

/* Dummy definition for external function */
void external_side_effect(int val) {
    volatile int sink = val;
    (void)sink;
}
