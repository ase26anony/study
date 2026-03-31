/* test-annotate-expr.c */
/* Compile with: gcc -O2 -ftree-vectorize -fdump-tree-all -c test-annotate-expr.c */

#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
static void use(int val) {
    volatile int sink = val;
    (void)sink;
}

/* External function to create side effects */
extern void external_side_effect(int);

int main(void) {
    const int N = 100;
    int i, j;
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
    
    /* 1. Loop with annot_expr_no_vector_kind (pragma GCC ivdep) */
    /* This pragma tells GCC to ignore vector dependencies */
    #pragma GCC ivdep
    for (i = 1; i < N; i++) {
        /* Potential dependency that ivdep overrides */
        a[i] = a[i-1] + a[i];
    }
    use(a[N-1]); /* Prevent dead code elimination */
    
    /* 2. Loop with annot_expr_vector_kind (pragma GCC vector) */
    /* Hint that this loop should be vectorized */
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 2 + 3;
    }
    use(b[N/2]);
    
    /* 3. Loop with annot_expr_parallel_kind (pragma GCC parallel) */
    /* Hint that loop iterations are independent and can be parallelized */
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        /* Independent computation - each iteration writes to different location */
        c[i] = b[i] * b[i];
    }
    use(c[N/2]);
    
    /* Nested loop with pragma in inner loop */
    for (i = 0; i < 10; i++) {
        #pragma GCC ivdep
        for (j = 0; j < N; j++) {
            a[j] += i;
        }
    }
    
    /* 4. Loop with annot_expr_maybe_infinite_kind (pragma GCC unroll without count) */
    /* This pragma suggests unrolling without specifying a count */
    int iterations = 8; /* Variable to prevent compile-time unrolling determination */
    #pragma GCC unroll
    for (i = 0; i < iterations; i++) {
        product *= (i + 1);
        external_side_effect(product); /* External call prevents optimization */
    }
    
    /* Conditional block with annotated loop */
    if (product > 0) {
        #pragma GCC vector
        for (i = 0; i < N/2; i++) {
            b[i] = c[i] - a[i];
        }
    }
    
    /* Compute final result using all loops */
    sum = 0;
    for (i = 0; i < N; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    /* Final result prevents entire program from being optimized away */
    printf("Result: sum = %d, product = %d\n", sum, product);
    
    return 0;
}

/* Define the external function to avoid linkage errors */
void external_side_effect(int val) {
    volatile int sink = val;
    (void)sink;
}
