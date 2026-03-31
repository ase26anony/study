/* test-annotated-loops.c */
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
    int result = 0;
    
    /* Initialize arrays */
    int a[N], b[N], c[N];
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
        a[i] = a[i-1] * 2 + a[i];
        use(a[i]);
    }
    
    /* 2. annot_expr_vector_kind: #pragma GCC vector */
    /* Simple vectorizable computation */
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 3 + i;
        use(b[i]);
    }
    
    /* 3. annot_expr_parallel_kind: #pragma GCC parallel */
    /* Independent iterations suitable for parallel execution */
    int partial_sums[4] = {0, 0, 0, 0};
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        /* Independent accumulation into thread-local buckets */
        partial_sums[i % 4] += b[i] * 2;
    }
    for (i = 0; i < 4; i++) {
        result += partial_sums[i];
        use(partial_sums[i]);
    }
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll */
    /* Loop with variable bound - compiler might unroll */
    int unroll_count = 8; /* Variable to prevent compile-time unrolling decision */
    int prod = 1;
    #pragma GCC unroll
    for (i = 1; i <= unroll_count; i++) {
        prod *= i;
        use(prod);
    }
    result += prod;
    
    /* Nested loop with annotation on inner loop */
    #pragma GCC ivdep
    for (j = 0; j < 10; j++) {
        #pragma GCC vector
        for (i = 0; i < N; i++) {
            c[i] += a[i] + b[i] + j;
        }
        use(c[N-1]);
    }
    
    /* Loop inside conditional with annotation */
    if (result > 0) {
        #pragma GCC parallel
        for (i = 0; i < 50; i++) {
            result += i * i;
            use(result);
        }
    }
    
    /* Final computation and output */
    for (i = 0; i < N; i++) {
        result += c[i];
    }
    
    printf("Result: %d\n", result);
    return 0;
}
