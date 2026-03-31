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

#define N 1024
#define M 8

int main(void) {
    int i, j;
    int a[N], b[N], c[N];
    int partial_sums[4] = {0};
    int product = 1;
    volatile int prevent_opt;
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        a[i] = i % 100;
        b[i] = 0;
        c[i] = (i % 50) * 2;
    }
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC ivdep 
     * Loop with potential dependencies that we assert are safe */
    #pragma GCC ivdep
    for (i = 1; i < N; i++) {
        /* This appears to have a dependency, but we assert it's safe with ivdep */
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
        /* Simulate thread-local accumulation */
        partial_sums[i % 4] += b[i] * 2;
    }
    for (j = 0; j < 4; j++) {
        use(partial_sums[j]);
    }
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll
     * Loop with variable bound - compiler might consider infinite unrolling */
    int iterations = 12; /* Not constant, so unroll without count */
    #pragma GCC unroll
    for (i = 1; i <= iterations; i++) {
        product *= i;
        external_side_effect(product);
    }
    use(product);
    
    /* Nested loop with annotation on inner loop */
    #pragma GCC ivdep
    for (i = 0; i < M; i++) {
        #pragma GCC vector
        for (j = 0; j < M; j++) {
            prevent_opt = i * j;
            (void)prevent_opt;
        }
    }
    
    /* Annotated loop inside conditional */
    if (N > 100) {
        #pragma GCC parallel
        for (i = 0; i < 100; i++) {
            c[i] = a[i] + b[i];
        }
        use(c[99]);
    }
    
    /* While loop with annotation */
    i = 0;
    #pragma GCC unroll
    while (i < 10) {
        product += i;
        i++;
    }
    use(product);
    
    /* Calculate final checksum */
    int checksum = 0;
    for (i = 0; i < N; i++) {
        checksum += a[i] + b[i] + c[i];
    }
    checksum += product;
    for (j = 0; j < 4; j++) {
        checksum += partial_sums[j];
    }
    
    printf("Result: %d\n", checksum);
    return 0;
}

/* Dummy definition of external function */
void external_side_effect(int val) {
    volatile int sink = val;
    (void)sink;
}
