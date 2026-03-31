/* test-annotate-expr.c
 * This program is designed to trigger the uncovered lines in tree-pretty-print.cc
 * that handle ANNOTATE_EXPR nodes for loop-specific pragmas.
 * Compile with: gcc -O2 -ftree-vectorize -fdump-tree-vect -fdump-tree-optimized -c test-annotate-expr.c
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

int main(void) {
    const int N = 256;
    int i, j;
    int sum = 0;
    
    /* Arrays for loop computations */
    int a[N], b[N], c[N];
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
        c[i] = 0;
    }
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC ivdep */
    /* Loop with potential dependencies that ivdep overrides */
    #pragma GCC ivdep
    for (i = 1; i < N; i++) {
        a[i] = a[i-1] + b[i];  /* Appears to have dependency, but ivdep tells compiler it's safe */
        external_side_effect(a[i]);  /* Side effect to prevent removal */
    }
    use(a[N-1]);
    
    /* 2. annot_expr_vector_kind: #pragma GCC vector */
    /* Simple vectorizable computation */
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        c[i] = a[i] * 2 + b[i];
    }
    use(c[N/2]);
    
    /* 3. annot_expr_parallel_kind: #pragma GCC parallel */
    /* Independent iterations suitable for parallelization */
    int partial_sums[4] = {0, 0, 0, 0};
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        partial_sums[i % 4] += c[i] * 3;  /* Independent accumulations */
    }
    for (i = 0; i < 4; i++) {
        sum += partial_sums[i];
    }
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll */
    /* Loop with variable bound - maybe infinite unrolling */
    int unroll_count = 8;  /* Variable to prevent constant folding */
    int product = 1;
    #pragma GCC unroll
    for (i = 1; i <= unroll_count; i++) {
        product *= i;
        external_side_effect(product);
    }
    sum += product;
    
    /* Nested loop with annotation on inner loop */
    #pragma GCC ivdep
    for (i = 0; i < 10; i++) {
        #pragma GCC vector
        for (j = 0; j < 10; j++) {
            c[i*10 + j] += i * j;
        }
    }
    
    /* Loop inside conditional with annotation */
    if (sum > 0) {
        #pragma GCC parallel
        for (i = 0; i < 100; i++) {
            sum += i % 7;
        }
    }
    
    /* While loop with annotation */
    i = 0;
    #pragma GCC unroll
    while (i < 5) {
        sum += i * 10;
        i++;
    }
    
    /* Final result to ensure all loops contribute */
    printf("Result: %d\n", sum);
    
    /* Additional test: multiple annotations in sequence */
    volatile int trigger;
    for (int k = 0; k < 3; k++) {
        #pragma GCC ivdep
        for (i = 0; i < 20; i++) {
            trigger = a[i] + k;
        }
        
        #pragma GCC vector
        for (i = 0; i < 20; i++) {
            trigger = b[i] * k;
        }
    }
    
    return sum > 0 ? 0 : 1;
}

/* Definition of external function */
void external_side_effect(int val) {
    volatile int sink = val;
    (void)sink;
}
