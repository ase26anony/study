/* test-annotate-expr.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
extern void use(int val);
void use(int val) {
    volatile int sink = val;
    (void)sink;
}

/* Function to create data dependencies */
int maybe_dependent(int i, int j) {
    return (i * 1103515245 + 12345) % 100;
}

int main(void) {
    const int N = 100;
    int a[N], b[N], c[N];
    int sum = 0, prod = 1;
    volatile int prevent_opt = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = i + 1;
        b[i] = 0;
        c[i] = 0;
    }
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC ivdep */
    /* Loop with potential dependencies that ivdep tells compiler to ignore */
    #pragma GCC ivdep
    for (int i = 1; i < N; i++) {
        /* Potential dependency that ivdep says can be ignored */
        a[i] = a[i-1] + a[i] + maybe_dependent(i, i-1);
    }
    use(a[N-1]);  /* Prevent dead code elimination */
    
    /* 2. annot_expr_vector_kind: #pragma GCC vector */
    /* Simple vectorizable computation */
    #pragma GCC vector
    for (int i = 0; i < N; i++) {
        b[i] = a[i] * 2 + 1;
    }
    use(b[N/2]);
    
    /* 3. annot_expr_parallel_kind: #pragma GCC parallel */
    /* Independent iterations suitable for parallel execution */
    #pragma GCC parallel
    for (int i = 0; i < N; i++) {
        c[i] = b[i] * 3;
        sum += c[i];  /* Reduction - but pragma suggests parallelizable */
    }
    use(sum);
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll */
    /* Loop that could be unrolled (maybe infinitely) */
    int limit = 8;  /* Variable to prevent compile-time unrolling decision */
    #pragma GCC unroll
    for (int i = 0; i < limit; i++) {
        prod *= (a[i] + 1);
        prevent_opt = prod;  /* Volatile store to prevent optimization */
    }
    
    /* Nested loop with annotation */
    #pragma GCC ivdep
    for (int i = 0; i < 10; i++) {
        #pragma GCC vector
        for (int j = 0; j < 10; j++) {
            b[i*10 + j] += i * j;
        }
    }
    
    /* Loop in conditional context */
    if (sum > 0) {
        #pragma GCC parallel
        for (int i = 0; i < 20; i++) {
            c[i] = c[i] / 2;
        }
    }
    
    /* Mixed with other control flow */
    for (int outer = 0; outer < 5; outer++) {
        #pragma GCC unroll
        for (int inner = 0; inner < 4; inner++) {
            if (inner % 2 == 0) {
                a[outer*4 + inner] += prod;
            }
        }
    }
    
    /* Final computation and output */
    int final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += a[i] + b[i] + c[i];
    }
    
    printf("Result: sum=%d, prod=%d, final=%d\n", sum, prod, final_sum);
    return 0;
}
