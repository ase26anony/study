/* test-annotate-expr.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
static void use(int val) {
    volatile int sink = val;
    (void)sink;
}

/* External function to create side effects */
extern void external_func(int);

int main(void) {
    const int N = 100;
    int i, j;
    int sum = 0;
    
    /* Arrays for loop computations */
    int a[N], b[N], c[N];
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = 0;
        c[i] = 0;
    }
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC ivdep */
    /* Loop with potential dependencies that ivdep overrides */
    #pragma GCC ivdep
    for (i = 1; i < N; i++) {
        /* Potential dependency that ivdep tells compiler to ignore */
        a[i] = a[i-1] + i;
    }
    use(a[N-1]);  /* Prevent dead code elimination */
    
    /* 2. annot_expr_vector_kind: #pragma GCC vector */
    /* Simple vectorizable computation */
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 2 + 7;
    }
    use(b[N/2]);
    
    /* 3. annot_expr_parallel_kind: #pragma GCC parallel */
    /* Independent iterations suitable for parallel execution */
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        int local_sum = 0;
        for (j = 0; j < 10; j++) {
            local_sum += a[i] + j;
        }
        c[i] = local_sum;
    }
    use(c[N-1]);
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll */
    /* Loop with variable bound - may be infinite for unrolling */
    int iterations = 8;  /* Variable, not constant */
    int product = 1;
    
    #pragma GCC unroll
    for (i = 1; i <= iterations; i++) {
        product *= i;
        external_func(product);  /* External call prevents optimization */
    }
    use(product);
    
    /* Nested loop with annotation */
    #pragma GCC ivdep
    for (i = 0; i < N; i++) {
        for (j = 0; j < 5; j++) {
            b[i] += j;
        }
    }
    
    /* Annotated loop inside conditional */
    if (N > 50) {
        #pragma GCC vector
        for (i = 0; i < 20; i++) {
            a[i] += b[i];
        }
    }
    
    /* Final computation and output */
    for (i = 0; i < N; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    printf("Result: %d\n", sum + product);
    return 0;
}

/* Dummy external function definition */
void external_func(int x) {
    volatile int sink = x;
    (void)sink;
}
