/* test-annotate-expr.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
static void use(int x) {
    volatile int sink = x;
    (void)sink;
}

/* External function to create side effects */
extern void external_call(int);

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
    /* Independent iterations - could be parallelized */
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
        use(c[i]);
    }
    
    /* Nested loop with annotation on inner loop */
    #pragma GCC ivdep
    for (i = 0; i < 10; i++) {
        #pragma GCC vector
        for (j = 0; j < 10; j++) {
            sum += i * j;
        }
    }
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll */
    /* Loop with variable bound - maybe infinite for unrolling */
    int limit = 8;
    #pragma GCC unroll
    for (i = 0; i < limit; i++) {
        prod *= (i + 1);
        use(prod);
    }
    
    /* Annotated loop inside conditional */
    if (sum > 0) {
        #pragma GCC ivdep
        for (i = 0; i < N/2; i++) {
            a[i] += b[i];
            use(a[i]);
        }
    } else {
        #pragma GCC vector
        for (i = N/2; i < N; i++) {
            b[i] -= a[i];
            use(b[i]);
        }
    }
    
    /* Final computation using all results */
    int final_result = 0;
    for (i = 0; i < N; i++) {
        final_result += a[i] + b[i] + c[i];
    }
    final_result += sum + prod;
    
    /* Print result to ensure execution */
    printf("Result: %d\n", final_result);
    
    /* Call external function to prevent dead code elimination */
    external_call(final_result);
    
    return final_result > 0 ? 0 : 1;
}

/* Define the external function to avoid linkage errors */
void external_call(int x) {
    volatile int sink = x;
    (void)sink;
}
