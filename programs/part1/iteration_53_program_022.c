/* test-annotate-expr.c
 * This program is designed to trigger the uncovered lines in tree-pretty-print.cc
 * related to ANNOTATE_EXPR nodes with loop-specific pragmas.
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

#define N 1024
#define M 8

int main(void) {
    int i, j;
    int a[N], b[N], c[N];
    int partial_sums[4] = {0};
    int product = 1;
    volatile int prevent_opt = 0;
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        a[i] = i % 100;
        b[i] = 0;
        c[i] = (i % 50) * 2;
    }
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC ivdep 
     * Loop with potential dependencies but we assert there are none */
    #pragma GCC ivdep
    for (i = 1; i < N; i++) {
        /* This looks like it has a dependency, but ivdep tells compiler to ignore */
        a[i] = a[i] + a[i-1] * prevent_opt; /* prevent_opt is volatile, so dependency isn't trivial */
    }
    use(a[N-1]);
    
    /* 2. annot_expr_vector_kind: #pragma GCC vector 
     * Simple vectorizable computation */
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 3 + c[i];
    }
    use(b[N/2]);
    
    /* 3. annot_expr_parallel_kind: #pragma GCC parallel 
     * Independent iterations accumulating into array */
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        partial_sums[i % 4] += b[i] * 2;
    }
    for (i = 0; i < 4; i++) {
        external_side_effect(partial_sums[i]);
    }
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll 
     * Loop with variable bound - compiler might think it could be infinite */
    int iterations = 7; /* Not compile-time constant */
    #pragma GCC unroll
    for (i = 0; i < iterations; i++) {
        product *= (i + 1);
        external_side_effect(product);
    }
    
    /* Nested loop with annotation on inner loop */
    #pragma GCC ivdep
    for (i = 0; i < M; i++) {
        #pragma GCC vector
        for (j = 0; j < M; j++) {
            c[i * M + j] += i * j;
        }
    }
    
    /* Loop with annotation inside conditional */
    if (prevent_opt) {
        #pragma GCC parallel
        for (i = 0; i < 10; i++) {
            external_side_effect(i);
        }
    }
    
    /* Final computation and output to ensure execution */
    int checksum = 0;
    for (i = 0; i < N; i++) {
        checksum += a[i] + b[i] + c[i];
    }
    checksum += product;
    for (i = 0; i < 4; i++) {
        checksum += partial_sums[i];
    }
    
    printf("Result: %d\n", checksum);
    return 0;
}

/* Define the external function to prevent undefined references */
void external_side_effect(int val) {
    volatile int sink = val;
    (void)sink;
}
