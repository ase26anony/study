/* test-annotate-expr.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
static void use(int val) {
    volatile int sink = val;
    (void)sink;
}

/* External function to create side effects */
extern void external_call(int x);

int main(void) {
    const int N = 100;
    int i, j;
    int sum = 0;
    int product = 1;
    
    /* Arrays for various loop tests */
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
        /* Potential dependency that ivdep tells compiler to ignore */
        a[i] = a[i-1] + i;
        use(a[i]);  /* Prevent dead code elimination */
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
        /* Nested loop with independent inner iterations */
        for (j = 0; j < 10; j++) {
            local_sum += a[i] * j;
        }
        c[i] = local_sum;
        use(c[i]);
    }
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll */
    /* Loop with variable bound - may be infinite for unrolling */
    int iterations = 8;  /* Variable to prevent constant folding */
    #pragma GCC unroll
    for (i = 0; i < iterations; i++) {
        product *= (i + 1);
        external_call(product);  /* External call prevents optimization */
    }
    
    /* Mixed constructs and control flow */
    if (product > 0) {
        /* Another ivdep loop inside conditional */
        #pragma GCC ivdep
        for (i = 0; i < N/2; i++) {
            b[i] += a[N-i-1];
            use(b[i]);
        }
    }
    
    /* Calculate final result using all arrays */
    for (i = 0; i < N; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    /* Use volatile to ensure computation isn't optimized away */
    volatile int final_result = sum + product;
    
    printf("Result: %d\n", final_result);
    
    return 0;
}

/* Define the external function to satisfy linker */
void external_call(int x) {
    volatile int sink = x;
    (void)sink;
}
