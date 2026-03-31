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

/* External function call to prevent dead code elimination */
extern void external_call(int);

int main(void) {
    const int N = 256;
    int i, j;
    int sum = 0;
    
    /* Arrays for loop computations */
    int a[N], b[N], c[N];
    
    /* Initialize arrays */
    for (i = 0; i < N; ++i) {
        a[i] = i + 1;
        b[i] = 0;
        c[i] = 0;
    }
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC ivdep 
     * Loop with potential dependencies that we assert are safe */
    #pragma GCC ivdep
    for (i = 1; i < N; ++i) {
        /* Potential dependency broken by ivdep pragma */
        a[i] = a[i-1] + a[i];
    }
    use(a[N-1]); /* Prevent elimination */
    
    /* 2. annot_expr_vector_kind: #pragma GCC vector 
     * Simple vectorizable computation */
    #pragma GCC vector
    for (i = 0; i < N; ++i) {
        b[i] = a[i] * 2;
    }
    use(b[N/2]);
    
    /* 3. annot_expr_parallel_kind: #pragma GCC parallel 
     * Independent iterations suitable for parallel execution */
    #pragma GCC parallel
    for (i = 0; i < N; ++i) {
        /* Independent accumulation into array */
        c[i] = b[i] + i;
    }
    use(c[N-1]);
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll 
     * Loop with variable bound - may be unrolled indefinitely */
    int limit = 8;
    int product = 1;
    #pragma GCC unroll
    for (i = 1; i <= limit; ++i) {
        product *= i;
    }
    use(product);
    
    /* Nested loop with annotation on inner loop */
    int matrix[10][10];
    for (i = 0; i < 10; ++i) {
        #pragma GCC ivdep
        for (j = 0; j < 10; ++j) {
            matrix[i][j] = i * j;
        }
    }
    use(matrix[5][5]);
    
    /* Loop inside conditional with annotation */
    if (N > 100) {
        #pragma GCC vector
        for (i = 0; i < 10; ++i) {
            sum += a[i];
        }
    }
    
    /* While loop with annotation */
    i = 0;
    #pragma GCC unroll
    while (i < 5) {
        sum += i;
        i++;
    }
    
    /* Final computation and output */
    for (i = 0; i < N; ++i) {
        sum += a[i] + b[i] + c[i];
    }
    
    printf("Result: %d\n", sum);
    external_call(sum); /* Prevent optimization of entire program */
    
    return 0;
}

/* Dummy definition to satisfy external declaration */
void external_call(int val) {
    volatile int sink = val;
    (void)sink;
}
