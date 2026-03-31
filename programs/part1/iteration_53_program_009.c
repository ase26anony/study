/* test-annotate-expr.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
static volatile int sink;
extern void use(int val);

void use(int val) {
    sink = val;
}

int main(void) {
    const int N = 100;
    int i, j;
    int sum = 0;
    int prod = 1;
    
    /* Arrays for loop computations */
    int a[N], b[N], c[N];
    
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
    }
    use(a[N-1]);  /* Prevent dead code elimination */
    
    /* 2. annot_expr_vector_kind: #pragma GCC vector */
    /* Simple vectorizable computation */
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 2;
    }
    use(b[N/2]);
    
    /* 3. annot_expr_parallel_kind: #pragma GCC parallel */
    /* Independent iterations suitable for parallel execution */
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
    use(c[N-1]);
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll */
    /* Loop with variable bound - compiler might unroll if it can determine bound */
    int limit = 8;  /* Variable to prevent compile-time unrolling decision */
    #pragma GCC unroll
    for (i = 0; i < limit; i++) {
        prod *= (i + 1);
    }
    use(prod);
    
    /* Nested loop with annotation on inner loop */
    #pragma GCC ivdep
    for (i = 0; i < 10; i++) {
        #pragma GCC vector
        for (j = 0; j < 10; j++) {
            a[i*10 + j] += j;
        }
    }
    use(a[50]);
    
    /* Annotated loop inside conditional */
    if (N > 50) {
        #pragma GCC parallel
        for (i = 0; i < 20; i++) {
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
    for (i = 0; i < N; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    printf("Result: sum = %d, prod = %d\n", sum, prod);
    return 0;
}
