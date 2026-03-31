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
    /* Loop with potential dependency that ivdep overrides */
    #pragma GCC ivdep
    for (i = 1; i < N; i++) {
        a[i] = a[i-1] + i;  /* Appears to have dependency, but ivdep tells compiler it's safe */
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
        /* Nested loop to make it more interesting */
        for (j = 0; j < 10; j++) {
            local_sum += a[i] + j;
        }
        c[i] = local_sum;
        use(c[i]);
    }
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll */
    /* Loop with variable bound - compiler might think it's infinite */
    int iterations = 8;  /* Variable, not constant */
    #pragma GCC unroll
    for (i = 0; i < iterations; i++) {
        prod *= (i + 1);
        use(prod);
    }
    
    /* Mixed constructs: annotated loop inside conditional */
    if (N > 50) {
        #pragma GCC ivdep
        for (i = 0; i < 20; i++) {
            b[i] += a[i];
            use(b[i]);
        }
    }
    
    /* Calculate final result using all arrays */
    int final_sum = 0;
    for (i = 0; i < N; i++) {
        final_sum += a[i] + b[i] + c[i];
    }
    final_sum += prod;
    
    printf("Result: %d\n", final_sum);
    return 0;
}
