/* test-annotate-expr.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
extern void use(int val);
void use(int val) {
    volatile int sink = val;
    (void)sink;
}

int main(void) {
    const int N = 100;
    int i, j, k;
    int sum = 0;
    int prod = 1;
    
    /* Arrays for loop computations */
    int a[N], b[N], c[N];
    volatile int result;
    
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
        /* Potential dependency that ivdep tells compiler to ignore */
        a[i] = a[i] + a[i-1] * 2;
    }
    result = a[N-1];
    use(result);
    
    /* 2. annot_expr_vector_kind: #pragma GCC vector */
    /* Simple vectorizable computation */
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 3 + 7;
    }
    result = b[N/2];
    use(result);
    
    /* 3. annot_expr_parallel_kind: #pragma GCC parallel */
    /* Independent iterations suitable for parallel execution */
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        int local_sum = 0;
        for (j = 0; j < 10; j++) {
            local_sum += a[i] * j;
        }
        c[i] = local_sum;
    }
    result = c[N-1];
    use(result);
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll */
    /* Loop where unrolling might be infinite (no count specified) */
    int unroll_count = 8; /* Variable to prevent compile-time unrolling */
    #pragma GCC unroll
    for (k = 0; k < unroll_count; k++) {
        prod *= (k + 2);
    }
    result = prod;
    use(result);
    
    /* Nested loop with annotation */
    #pragma GCC ivdep
    for (i = 0; i < N; i++) {
        for (j = 0; j < 5; j++) {
            b[i] += j;
        }
    }
    
    /* Annotated loop in conditional context */
    if (N > 50) {
        #pragma GCC vector
        for (i = 0; i < 20; i++) {
            a[i] += b[i];
        }
    }
    
    /* Final computation and output */
    sum = 0;
    for (i = 0; i < N; i++) {
        sum += a[i] + b[i] + c[i];
    }
    sum += prod;
    
    printf("Result: %d\n", sum);
    return 0;
}
