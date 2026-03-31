/* test-annotate-expr.c
 * A test program to trigger pretty-printing of ANNOTATE_EXPR nodes
 * with loop-specific pragma kinds in GCC's tree pretty-printer.
 */

#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
static volatile int sink = 0;
static void use(int val) { sink = val; }

int main(void) {
    const int N = 100;
    int i, j;
    int a[N], b[N], c[N];
    int sum = 0, prod = 1;
    
    /* Initialize arrays */
    for (i = 0; i < N; ++i) {
        a[i] = i + 1;
        b[i] = 0;
        c[i] = 0;
    }
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC ivdep 
     *    Loop with potential dependency that ivdep overrides */
    #pragma GCC ivdep
    for (i = 1; i < N; ++i) {
        a[i] = a[i-1] + 1;  /* Appears dependent, but ivdep tells compiler it's safe */
    }
    use(a[N-1]);
    
    /* 2. annot_expr_vector_kind: #pragma GCC vector
     *    Simple vectorizable computation */
    #pragma GCC vector
    for (i = 0; i < N; ++i) {
        b[i] = a[i] * 2;
    }
    use(b[N-1]);
    
    /* 3. annot_expr_parallel_kind: #pragma GCC parallel
     *    Independent accumulations into local array */
    #pragma GCC parallel
    for (i = 0; i < N; ++i) {
        c[i] = a[i] + b[i];  /* Independent iterations */
    }
    use(c[N-1]);
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll
     *    Loop with variable bound, hinting at possible infinite unrolling */
    int limit = 10;
    #pragma GCC unroll
    for (i = 0; i < limit; ++i) {
        prod *= (i + 1);
    }
    use(prod);
    
    /* Nested loop with pragma on inner loop */
    #pragma GCC ivdep
    for (i = 0; i < 10; ++i) {
        #pragma GCC vector
        for (j = 0; j < 10; ++j) {
            a[i*10 + j] += j;
        }
    }
    use(a[99]);
    
    /* Loop inside conditional with pragma */
    if (N > 50) {
        #pragma GCC parallel
        for (i = 0; i < 50; ++i) {
            b[i] = c[i] * 3;
        }
        use(b[49]);
    }
    
    /* Final computation and output to verify execution */
    for (i = 0; i < N; ++i) {
        sum += a[i] + b[i] + c[i];
    }
    sum += prod;
    
    printf("Result: %d (sink=%d)\n", sum, sink);
    return 0;
}
