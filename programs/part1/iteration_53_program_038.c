/* test-annotate-expr.c
 * This program is designed to trigger the uncovered lines in tree-pretty-print.cc
 * related to ANNOTATE_EXPR nodes with loop-specific annotation kinds.
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
        a[i] = i;
        b[i] = N - i;
        c[i] = 0;
    }
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC ivdep 
     * Loop with potential dependencies that we assert are safe */
    #pragma GCC ivdep
    for (i = 1; i < N; i++) {
        /* This has a potential dependency, but we assert it's safe with ivdep */
        a[i] = a[i-1] + b[i];
    }
    use(a[N-1]);
    
    /* 2. annot_expr_vector_kind: #pragma GCC vector
     * Simple vectorizable computation */
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        c[i] = a[i] * 2 + b[i];
    }
    use(c[N/2]);
    
    /* 3. annot_expr_parallel_kind: #pragma GCC parallel
     * Independent iterations accumulating into thread-local storage (simulated) */
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        /* Simulate thread-local accumulation */
        partial_sums[i % 4] += c[i];
    }
    for (i = 0; i < 4; i++) {
        prevent_opt += partial_sums[i];
    }
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll
     * Loop with variable bound - compiler might consider it for unrolling */
    int unroll_bound = M;
    #pragma GCC unroll
    for (i = 0; i < unroll_bound; i++) {
        product *= (i + 1);
        external_side_effect(i);
    }
    use(product);
    
    /* Nested loop with annotation on inner loop */
    #pragma GCC ivdep
    for (i = 0; i < N/2; i++) {
        #pragma GCC vector
        for (j = 0; j < 4; j++) {
            int idx = i * 4 + j;
            if (idx < N) {
                a[idx] += j;
            }
        }
    }
    use(a[0]);
    
    /* Annotated loop inside conditional */
    if (prevent_opt > 0) {
        #pragma GCC parallel
        for (i = 0; i < 100; i++) {
            b[i] = a[i] * 3;
        }
        use(b[99]);
    }
    
    /* While loop with annotation */
    i = 0;
    #pragma GCC unroll
    while (i < 10) {
        product += i;
        i++;
    }
    use(product);
    
    /* Final computation and output to ensure execution */
    int sum = 0;
    for (i = 0; i < N; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    printf("Result: %d (checksum: %d)\n", sum, product);
    return 0;
}

/* Dummy definition to satisfy external reference */
void external_side_effect(int x) {
    volatile int dummy = x;
    (void)dummy;
}
