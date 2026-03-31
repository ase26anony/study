/* test-annotate-expr.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
static volatile int sink;
void use(int val) { sink = val; }

int main(void) {
    const int N = 100;
    int i, j;
    int a[N], b[N], c[N];
    int sum = 0, prod = 1;
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        a[i] = i + 1;
        b[i] = 0;
        c[i] = 0;
    }
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC ivdep */
    /* Loop with potential dependencies that ivdep overrides */
    #pragma GCC ivdep
    for (i = 1; i < N; i++) {
        a[i] = a[i-1] + a[i];  /* Appears to have dependency, but ivdep tells compiler it's safe */
        use(a[i]);
    }
    
    /* 2. annot_expr_vector_kind: #pragma GCC vector */
    /* Simple vectorizable computation */
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 2 + 3;
        use(b[i]);
    }
    
    /* 3. annot_expr_parallel_kind: #pragma GCC parallel */
    /* Independent iterations suitable for parallel execution */
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        int local_sum = 0;
        for (j = 0; j < 10; j++) {
            local_sum += a[i] + j;
        }
        c[i] = local_sum;
        use(c[i]);
    }
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll */
    /* Loop where unrolling might be beneficial */
    int limit = 8;  /* Variable to prevent compile-time unrolling determination */
    #pragma GCC unroll
    for (i = 0; i < limit; i++) {
        prod *= (i + 1);
        use(prod);
    }
    
    /* Nested example with pragma on inner loop */
    #pragma GCC ivdep
    for (i = 0; i < N; i++) {
        #pragma GCC vector
        for (j = 0; j < 10; j++) {
            b[i] += j;
            use(b[i]);
        }
    }
    
    /* Conditional context with annotated loop */
    if (N > 50) {
        #pragma GCC parallel
        for (i = 0; i < 20; i++) {
            sum += a[i];
            use(sum);
        }
    }
    
    /* Final computation and output to ensure execution */
    int total = 0;
    for (i = 0; i < N; i++) {
        total += a[i] + b[i] + c[i];
    }
    total += prod + sum;
    
    printf("Result: %d\n", total);
    return 0;
}
