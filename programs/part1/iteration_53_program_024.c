/* test-annotate-expr.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
static void use(int val) {
    volatile int sink = val;
    (void)sink;
}

/* External function to create side effects */
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
    /* Loop with potential dependencies that ivdep overrides */
    #pragma GCC ivdep
    for (i = 1; i < N; i++) {
        /* Potential dependency that ivdep tells compiler to ignore */
        a[i] = a[i-1] + i;
        use(a[i]); /* Prevent dead code elimination */
    }
    
    /* 2. annot_expr_vector_kind: #pragma GCC vector */
    /* Simple vectorizable computation */
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 2 + 3;
        use(b[i]);
    }
    
    /* Nested loop with annotation on inner loop */
    for (i = 0; i < 10; i++) {
        #pragma GCC vector
        for (j = 0; j < 10; j++) {
            c[i*10 + j] = i * j;
            use(c[i*10 + j]);
        }
    }
    
    /* 3. annot_expr_parallel_kind: #pragma GCC parallel */
    /* Independent iterations suitable for parallel execution */
    int partial_sums[4] = {0, 0, 0, 0};
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        /* Independent accumulation into thread-local buckets */
        partial_sums[i % 4] += a[i];
        use(partial_sums[i % 4]);
    }
    
    /* Combine partial sums */
    for (i = 0; i < 4; i++) {
        sum += partial_sums[i];
    }
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll */
    /* Loop with variable bound - compiler might unroll infinitely */
    int iterations = 8; /* Variable to prevent compile-time unrolling decision */
    #pragma GCC unroll
    for (i = 0; i < iterations; i++) {
        product *= (i + 1);
        use(product);
    }
    
    /* Another maybe-infinite loop in conditional context */
    if (sum > 0) {
        int local_sum = 0;
        #pragma GCC unroll
        for (k = 0; k < 5; k++) {
            local_sum += k * k;
            use(local_sum);
        }
        sum += local_sum;
    }
    
    /* Mixed: loop with multiple annotations through nesting */
    #pragma GCC ivdep
    for (i = 0; i < 20; i++) {
        #pragma GCC vector
        for (j = 0; j < 5; j++) {
            b[i*5 + j] = a[i] + j;
            use(b[i*5 + j]);
        }
    }
    
    /* Final computation and output to ensure execution */
    int final_result = sum + product;
    for (i = 0; i < N; i++) {
        final_result += b[i] % 7;
    }
    
    printf("Result: %d\n", final_result);
    
    /* Call external function to create another side effect */
    external_call(final_result);
    
    return final_result > 0 ? 0 : 1;
}

/* Define the external function to avoid linkage errors */
void external_call(int val) {
    volatile int sink = val;
    (void)sink;
}
