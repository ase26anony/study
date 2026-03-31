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
int unpredictable(int i) {
    return (i * 1103515245 + 12345) & 0x7fffffff;
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
        c[i] = unpredictable(i) % 100;
    }
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC ivdep */
    /* Loop with potential dependencies that ivdep overrides */
    #pragma GCC ivdep
    for (int i = 1; i < N-1; i++) {
        /* Potential dependency that ivdep tells compiler to ignore */
        a[i] = a[i-1] + a[i+1] + c[i];
        use(a[i]); /* Prevent dead code elimination */
    }
    prevent_opt += a[N/2];
    
    /* 2. annot_expr_vector_kind: #pragma GCC vector */
    /* Vectorizable computation */
    #pragma GCC vector
    for (int i = 0; i < N; i++) {
        b[i] = a[i] * 2 + 3;
        use(b[i]);
    }
    prevent_opt += b[N/3];
    
    /* 3. annot_expr_parallel_kind: #pragma GCC parallel */
    /* Independent iterations suitable for parallel execution */
    int partial_sums[4] = {0, 0, 0, 0};
    #pragma GCC parallel
    for (int i = 0; i < N; i++) {
        /* Independent accumulation into thread-local buckets */
        partial_sums[i % 4] += b[i] * unpredictable(i);
    }
    for (int i = 0; i < 4; i++) {
        sum += partial_sums[i];
        use(partial_sums[i]);
    }
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll */
    /* Loop that could be unrolled indefinitely */
    int unroll_count = 8; /* Variable to prevent compile-time unrolling */
    #pragma GCC unroll
    for (int i = 0; i < unroll_count; i++) {
        prod *= (i + 1);
        use(prod);
    }
    
    /* Nested loop with annotation */
    if (sum > 0) {
        #pragma GCC ivdep
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                a[i*10 + j] += i * j;
                use(a[i*10 + j]);
            }
        }
    }
    
    /* Mixed control flow with annotated loop */
    for (int outer = 0; outer < 3; outer++) {
        if (outer % 2 == 0) {
            #pragma GCC vector
            for (int i = 0; i < 20; i++) {
                b[i] += outer * unpredictable(i);
                use(b[i]);
            }
        }
    }
    
    /* Final computation and output */
    int result = sum + prod + prevent_opt;
    for (int i = 0; i < N; i++) {
        result += a[i] + b[i];
    }
    
    printf("Result: %d\n", result);
    return result == 0 ? 0 : 1; /* Non-zero return to verify execution */
}
