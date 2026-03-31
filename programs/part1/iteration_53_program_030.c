/* test-annotate-expr.c
 * This program is designed to trigger the pretty-printing of ANNOTATE_EXPR
 * nodes with specific loop annotation kinds in GCC's tree pretty-printer.
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
     *    Loop with potential dependencies but we assert there are none */
    #pragma GCC ivdep
    for (i = 1; i < N; i++) {
        /* This appears to have a dependency, but ivdep tells compiler to ignore it */
        a[i] = a[i-1] + c[i];
    }
    use(a[N-1]);
    
    /* 2. annot_expr_vector_kind: #pragma GCC vector 
     *    Simple vectorizable computation */
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 3 + c[i];
    }
    use(b[N/2]);
    
    /* 3. annot_expr_parallel_kind: #pragma GCC parallel 
     *    Independent iterations accumulating into array */
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        /* Independent accumulation into different buckets */
        partial_sums[i % 4] += b[i];
    }
    for (i = 0; i < 4; i++) {
        prevent_opt += partial_sums[i];
    }
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll 
     *    Loop with variable bound - compiler might unroll infinitely */
    int iterations = 8; /* Variable to prevent compile-time unrolling decision */
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
            a[i*M + j] += j;
        }
    }
    
    /* Conditional block containing annotated loop */
    if (prevent_opt > 0) {
        #pragma GCC unroll
        for (i = 0; i < 4; i++) {
            partial_sums[i] *= 2;
        }
    }
    
    /* Final computation and output to ensure execution */
    int total = 0;
    for (i = 0; i < N; i++) {
        total += a[i] + b[i];
    }
    total += product;
    
    for (i = 0; i < 4; i++) {
        total += partial_sums[i];
    }
    
    printf("Result: %d\n", total);
    return 0;
}

/* Dummy definition to satisfy external reference */
void external_side_effect(int val) {
    volatile int sink = val;
    (void)sink;
}
