/* test-annotated-loops.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
extern void use(int val);
void use(int val) {
    volatile int sink = val;
    (void)sink;
}

/* Function to create data dependencies */
int maybe_dependent(int i, int j) {
    return (i * 1103515245 + 12345) % j;
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
        c[i] = 0;
    }
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC ivdep */
    /* Loop with potential dependencies that ivdep overrides */
    #pragma GCC ivdep
    for (int i = 1; i < N; i++) {
        /* Potential dependency broken by ivdep */
        a[i] = a[i-1] + a[i] + maybe_dependent(i, 7);
    }
    use(a[N-1]);
    
    /* 2. annot_expr_vector_kind: #pragma GCC vector */
    /* Vectorizable computation */
    #pragma GCC vector
    for (int i = 0; i < N; i++) {
        b[i] = a[i] * 2 + 1;
    }
    use(b[N/2]);
    
    /* 3. annot_expr_parallel_kind: #pragma GCC parallel */
    /* Independent accumulations */
    int partial_sums[4] = {0, 0, 0, 0};
    #pragma GCC parallel
    for (int i = 0; i < N; i++) {
        /* Independent per-iteration work */
        partial_sums[i % 4] += b[i] * 3;
    }
    for (int i = 0; i < 4; i++) {
        sum += partial_sums[i];
    }
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll */
    /* Loop with variable bound for unrolling */
    int unroll_bound = 8;
    prod = 1;
    #pragma GCC unroll
    for (int i = 1; i <= unroll_bound; i++) {
        prod *= i + sum % 17;
        prevent_opt = prod; /* Volatile use */
    }
    
    /* Nested loop with annotation */
    if (sum > 0) {
        #pragma GCC ivdep
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                c[i*10 + j] = i * j + prevent_opt;
            }
        }
        use(c[50]);
    }
    
    /* Mixed: vector annotation in conditional context */
    int threshold = 500;
    #pragma GCC vector
    for (int i = 0; i < N; i++) {
        if (a[i] > threshold) {
            b[i] /= 2;
        } else {
            b[i] *= 2;
        }
    }
    
    /* Final computation using all results */
    int final_result = 0;
    for (int i = 0; i < N; i++) {
        final_result += a[i] + b[i] + c[i];
    }
    final_result += sum + prod;
    
    printf("Result: %d\n", final_result);
    return final_result > 0 ? 0 : 1;
}
