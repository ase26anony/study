/* test-annotated-loops.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
static volatile int sink;
static void use(int val) { sink = val; }

int main(void) {
    const int N = 100;
    int i, j;
    int a[N], b[N], c[N];
    int sum = 0, prod = 1;
    
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
        a[i] = a[i-1] + a[i];  /* Apparent dependency, but ivdep tells compiler to ignore */
        use(a[i]);
    }
    
    /* 2. annot_expr_vector_kind: #pragma GCC vector */
    /* Simple vectorizable computation */
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 2;
        use(b[i]);
    }
    
    /* 3. annot_expr_parallel_kind: #pragma GCC parallel */
    /* Independent iterations suitable for parallel execution */
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        c[i] = a[i] + b[i];  /* Independent computation */
        use(c[i]);
    }
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll */
    /* Loop where unrolling might be beneficial */
    int limit = 8;  /* Variable to prevent compile-time unrolling determination */
    #pragma GCC unroll
    for (i = 0; i < limit; i++) {
        prod *= (i + 1);
        use(prod);
    }
    
    /* Nested loop with annotation on inner loop */
    #pragma GCC ivdep
    for (i = 0; i < 10; i++) {
        #pragma GCC vector
        for (j = 0; j < 10; j++) {
            a[i*10 + j] += j;
            use(a[i*10 + j]);
        }
    }
    
    /* Annotated loop inside conditional */
    if (N > 0) {
        #pragma GCC parallel
        for (i = 0; i < N/2; i++) {
            sum += a[i];
            use(sum);
        }
    }
    
    /* Final computation and output to ensure execution */
    int total = 0;
    for (i = 0; i < N; i++) {
        total += a[i] + b[i] + c[i];
    }
    total += sum + prod;
    
    printf("Result: %d (sink=%d)\n", total, sink);
    return total == 0 ? 1 : 0;  /* Non-zero return if all loops executed */
}
