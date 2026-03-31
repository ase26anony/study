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
    int i, j, k;
    int a[N], b[N], c[N];
    int sum = 0, prod = 1;
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        a[i] = i + 1;
        b[i] = 0;
        c[i] = 0;
    }
    
    /* 1. annot_expr_no_vector_kind - #pragma GCC ivdep */
    /* Loop with potential dependencies that ivdep overrides */
    #pragma GCC ivdep
    for (i = 1; i < N; i++) {
        a[i] = a[i-1] + a[i];  /* Appears to have dependency, but ivdep tells compiler it's safe */
        use(a[i]);
    }
    
    /* 2. annot_expr_vector_kind - #pragma GCC vector */
    /* Simple vectorizable computation */
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 2 + 3;
        use(b[i]);
    }
    
    /* 3. annot_expr_parallel_kind - #pragma GCC parallel */
    /* Independent iterations suitable for parallel execution */
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
        sum += c[i];  /* Reduction - but pragma suggests parallelizable */
        use(c[i]);
    }
    
    /* Nested loop with annotation on inner loop */
    for (i = 0; i < 10; i++) {
        #pragma GCC ivdep
        for (j = 0; j < 10; j++) {
            a[i*10 + j] += i * j;
        }
    }
    
    /* 4. annot_expr_maybe_infinite_kind - #pragma GCC unroll */
    /* Loop with variable bound - compiler might think it's infinite */
    int limit = 8;  /* Variable to prevent compile-time unrolling decision */
    #pragma GCC unroll
    for (k = 0; k < limit; k++) {
        prod *= (k + 1);
        use(prod);
    }
    
    /* Annotated loop in conditional context */
    if (sum > 0) {
        #pragma GCC vector
        for (i = 0; i < 20; i++) {
            b[i] = b[i] / 2;
            use(b[i]);
        }
    }
    
    /* While loop with annotation */
    i = 0;
    #pragma GCC ivdep
    while (i < N) {
        a[i] = a[i] - 1;
        use(a[i]);
        i++;
    }
    
    /* Compute final result using all arrays */
    int final_result = 0;
    for (i = 0; i < N; i++) {
        final_result += a[i] + b[i] + c[i];
    }
    final_result += sum + prod;
    
    printf("Result: %d\n", final_result);
    return 0;
}
