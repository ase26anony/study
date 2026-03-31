/* test-annotate-expr.c */
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
    const int N = 100;
    int i, j, k;
    int sum = 0;
    int product = 1;
    
    /* Arrays for loop computations */
    int a[N], b[N], c[N];
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        a[i] = i + 1;
        b[i] = 0;
        c[i] = 0;
    }
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC ivdep */
    /* Loop with potential dependencies that ivdep tells compiler to ignore */
    #pragma GCC ivdep
    for (i = 1; i < N; i++) {
        /* Potential dependency that ivdep tells compiler to ignore */
        a[i] = a[i-1] + i;
    }
    use(a[N-1]);  /* Prevent dead code elimination */
    
    /* Nested loop with ivdep on inner loop */
    for (i = 0; i < 10; i++) {
        #pragma GCC ivdep
        for (j = 0; j < N; j++) {
            b[j] += a[j] + i;
        }
    }
    
    /* 2. annot_expr_vector_kind: #pragma GCC vector */
    /* Simple vectorizable computation */
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        c[i] = a[i] * 2 + b[i];
    }
    use(c[N/2]);
    
    /* Vector pragma in conditional context */
    if (N > 50) {
        #pragma GCC vector
        for (i = 0; i < 50; i++) {
            a[i] = c[i] / 2;
        }
    }
    
    /* 3. annot_expr_parallel_kind: #pragma GCC parallel */
    /* Independent iterations suitable for parallel execution */
    int partial_sums[4] = {0, 0, 0, 0};
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        /* Each iteration writes to different element based on modulo */
        partial_sums[i % 4] += c[i];
    }
    
    /* Combine partial sums */
    for (k = 0; k < 4; k++) {
        sum += partial_sums[k];
    }
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll */
    /* Loop with variable bound - compiler might unroll if it can determine bound */
    int iterations = 8;  /* Variable, not constant */
    #pragma GCC unroll
    for (i = 0; i < iterations; i++) {
        product *= (i + 1);
    }
    use(product);
    
    /* Unroll pragma with constant bound in nested loop */
    for (j = 0; j < 2; j++) {
        #pragma GCC unroll
        for (i = 0; i < 4; i++) {
            sum += j * 10 + i;
        }
    }
    
    /* Mixed: loop with multiple annotations through nesting */
    #pragma GCC ivdep
    for (i = 0; i < N; i++) {
        #pragma GCC vector
        for (j = 0; j < 4; j++) {
            a[i] += j;
        }
    }
    
    /* Final computation and output */
    int final_result = sum + product + a[N-1] + b[N-1] + c[N-1];
    
    /* Use external call to ensure all computations are considered "used" */
    external_call(final_result);
    
    /* Print result to ensure execution */
    printf("Result: %d\n", final_result % 1000);
    
    return 0;
}

/* Define the external function to avoid linkage errors */
void external_call(int val) {
    volatile int sink = val;
    (void)sink;
}
