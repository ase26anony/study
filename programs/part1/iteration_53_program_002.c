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
    int product = 1;
    
    /* Arrays for various loop tests */
    int a[N], b[N], c[N];
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        a[i] = i + 1;
        b[i] = 0;
        c[i] = 1;
    }
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC ivdep */
    /* Loop with potential dependencies that ivdep tells compiler to ignore */
    #pragma GCC ivdep
    for (i = 1; i < N; i++) {
        a[i] = a[i-1] + i;  /* Appears to have dependency, but ivdep says it's safe */
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
    /* Independent iterations suitable for parallel execution */
    int partial_sums[4] = {0, 0, 0, 0};
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        partial_sums[i % 4] += b[i];  /* Independent accumulations */
    }
    
    /* Combine partial sums */
    for (i = 0; i < 4; i++) {
        sum += partial_sums[i];
    }
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll */
    /* Loop with variable bound - compiler might think it's infinite */
    int iterations = 8;  /* Variable, not constant */
    #pragma GCC unroll
    for (i = 0; i < iterations; i++) {
        product *= (i + 1);
        use(product);
    }
    
    /* Nested loop with annotation */
    #pragma GCC ivdep
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            c[i * 10 + j] = i * j;
            use(c[i * 10 + j]);
        }
    }
    
    /* Conditional context with annotated loop */
    if (sum > 0) {
        #pragma GCC vector
        for (i = 0; i < 20; i++) {
            b[i] = b[i] / 2;
            use(b[i]);
        }
    }
    
    /* Final computation and output */
    int result = sum + product;
    for (i = 0; i < 10; i++) {
        result += c[i];
    }
    
    printf("Result: %d\n", result);
    
    /* Additional test with while loop */
    i = 0;
    #pragma GCC unroll
    while (i < 5) {
        result += i;
        external_call(i);  /* External call prevents optimization */
        i++;
    }
    
    printf("Final result: %d\n", result);
    
    return 0;
}

/* Define the external function to avoid linkage errors */
void external_call(int x) {
    volatile int sink = x;
    (void)sink;
}
