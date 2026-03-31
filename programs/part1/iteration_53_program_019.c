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

#define N 1024
#define M 8

int main(void) {
    int i, j;
    int a[N], b[N], c[N];
    int partial_sums[4] = {0};
    int product = 1;
    volatile int result = 0;
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        a[i] = i % 100;
        b[i] = 0;
        c[i] = (i % 50) * 2;
    }
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC ivdep 
     * Loop with potential dependencies that ivdep overrides */
    #pragma GCC ivdep
    for (i = 1; i < N; i++) {
        /* This appears to have a dependency, but ivdep tells compiler 
         * it's safe to ignore for vectorization */
        a[i] = a[i-1] + c[i];
    }
    use(a[N-1]);
    
    /* 2. annot_expr_vector_kind: #pragma GCC vector
     * Simple vectorizable computation */
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 3 + 7;
    }
    use(b[N/2]);
    
    /* 3. annot_expr_parallel_kind: #pragma GCC parallel
     * Independent iterations accumulating into thread-local storage (simulated) */
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        /* Simulate thread-local accumulation: use modulo to distribute */
        partial_sums[i % 4] += b[i] * 2;
    }
    for (j = 0; j < 4; j++) {
        result += partial_sums[j];
    }
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll
     * Loop with variable bound - compiler might consider it for unrolling */
    int iterations = M;
    #pragma GCC unroll
    for (i = 0; i < iterations; i++) {
        product *= (i + 2);
        external_side_effect(product);
    }
    result += product;
    
    /* Nested loop with annotation on inner loop */
    #pragma GCC ivdep
    for (i = 0; i < 10; i++) {
        #pragma GCC vector
        for (j = 0; j < 10; j++) {
            c[i*10 + j] = i * j;
        }
    }
    use(c[50]);
    
    /* Annotated loop inside conditional */
    if (result > 0) {
        #pragma GCC parallel
        for (i = 0; i < 100; i++) {
            partial_sums[i % 4] -= i;
        }
    }
    
    /* While loop with annotation */
    i = 0;
    #pragma GCC unroll
    while (i < 5) {
        product += i;
        i++;
    }
    result += product;
    
    /* Print final result to ensure execution */
    printf("Result: %d\n", result);
    
    return 0;
}

/* Dummy definition to satisfy external reference */
void external_side_effect(int val) {
    volatile int sink = val;
    (void)sink;
}
