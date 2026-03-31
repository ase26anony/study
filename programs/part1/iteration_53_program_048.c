/* test_annotate_expr.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
extern void use(int val);
void use(int val) {
    volatile int sink = val;
    (void)sink;
}

/* Function to create data dependencies */
int maybe_dependent(int i, int n) {
    return (i * 7) % n;
}

int main(void) {
    const int N = 100;
    int a[N], b[N], c[N];
    int sum = 0, prod = 1;
    volatile int sink;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = i + 1;
        b[i] = 0;
        c[i] = 0;
    }
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC ivdep */
    /* Loop with potential dependencies that ivdep overrides */
    #pragma GCC ivdep
    for (int i = 1; i < N; i++) {
        /* Potential dependency that ivdep tells compiler to ignore */
        a[i] = a[i] + a[maybe_dependent(i, N)];
        use(a[i]);
    }
    
    /* 2. annot_expr_vector_kind: #pragma GCC vector */
    /* Simple vectorizable computation */
    #pragma GCC vector
    for (int i = 0; i < N; i++) {
        b[i] = a[i] * 2 + 3;
        use(b[i]);
    }
    
    /* 3. annot_expr_parallel_kind: #pragma GCC parallel */
    /* Independent iterations suitable for parallel execution */
    int partial_sums[4] = {0, 0, 0, 0};
    #pragma GCC parallel
    for (int i = 0; i < N; i++) {
        /* Each iteration writes to different element based on i % 4 */
        partial_sums[i % 4] += b[i] * (i + 1);
    }
    
    /* Combine partial sums */
    for (int i = 0; i < 4; i++) {
        sum += partial_sums[i];
        use(partial_sums[i]);
    }
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll */
    /* Loop with variable bound - compiler might think it's infinite */
    int iterations = 8; /* Variable, not constant */
    #pragma GCC unroll
    for (int i = 0; i < iterations; i++) {
        prod *= (i + 2);
        use(prod);
    }
    
    /* Nested loop with annotation on inner loop */
    #pragma GCC ivdep
    for (int i = 0; i < 10; i++) {
        #pragma GCC vector
        for (int j = 0; j < 10; j++) {
            c[i * 10 + j] = i * j;
            use(c[i * 10 + j]);
        }
    }
    
    /* Conditional block with annotated loop */
    if (sum > 0) {
        #pragma GCC parallel
        for (int i = 0; i < 20; i++) {
            sum += i;
            use(sum);
        }
    }
    
    /* Final computation and output */
    int result = sum + prod;
    for (int i = 0; i < N; i++) {
        result += a[i] + b[i];
    }
    
    printf("Result: %d\n", result);
    
    /* Additional test with while loop */
    int count = 5;
    #pragma GCC unroll
    while (count-- > 0) {
        result += count * 10;
        use(result);
    }
    
    printf("Final result: %d\n", result);
    
    return 0;
}
