/* test-annotate-expr.c */
/* Compile with: gcc -O2 -ftree-vectorize -fdump-tree-all -c test-annotate-expr.c */

#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
static volatile int sink;
static void use(int val) { sink = val; }

/* External function to create side effects */
extern void external_call(int);

#define SIZE 1024
#define CHUNK 8

int main(void) {
    int i, j, k;
    int result = 0;
    
    /* Initialize arrays */
    int a[SIZE], b[SIZE], c[SIZE];
    for (i = 0; i < SIZE; i++) {
        a[i] = i % 100;
        b[i] = 0;
        c[i] = 1;
    }
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC ivdep */
    /* Loop with potential dependencies that ivdep overrides */
    #pragma GCC ivdep
    for (i = 1; i < SIZE; i++) {
        /* Potential dependency overridden by ivdep */
        a[i] = a[i-1] + i;
    }
    use(a[SIZE-1]);  /* Prevent dead code elimination */
    
    /* 2. annot_expr_vector_kind: #pragma GCC vector */
    /* Simple vectorizable computation */
    #pragma GCC vector
    for (i = 0; i < SIZE; i++) {
        b[i] = a[i] * 2 + 7;
    }
    use(b[SIZE/2]);
    
    /* 3. annot_expr_parallel_kind: #pragma GCC parallel */
    /* Independent iterations suitable for parallel execution */
    int partial_sums[CHUNK] = {0};
    #pragma GCC parallel
    for (i = 0; i < SIZE; i++) {
        /* Independent accumulation into thread-local buckets */
        partial_sums[i % CHUNK] += b[i];
    }
    for (k = 0; k < CHUNK; k++) {
        result += partial_sums[k];
    }
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll */
    /* Loop with variable bound - may be infinite for unrolling */
    int iterations = 16;  /* Variable, not constant */
    int product = 1;
    #pragma GCC unroll
    for (j = 0; j < iterations; j++) {
        product *= (j + 1);
        external_call(product);  /* External call prevents optimization */
    }
    result += product;
    
    /* Nested loop with annotation on inner loop */
    #pragma GCC ivdep
    for (i = 0; i < SIZE/2; i++) {
        #pragma GCC vector
        for (j = 0; j < 4; j++) {
            c[i*2 + j] = a[i] * b[i*2 + j];
        }
    }
    use(c[SIZE-1]);
    
    /* Annotated loop inside conditional */
    if (result > 0) {
        #pragma GCC parallel
        for (i = 0; i < 100; i++) {
            result += i * i;
        }
    }
    
    /* While loop with annotation */
    int counter = 10;
    #pragma GCC unroll
    while (counter-- > 0) {
        result += counter;
        external_call(counter);
    }
    
    printf("Final result: %d\n", result);
    return 0;
}

/* Dummy definition to satisfy external_call reference */
void external_call(int x) {
    sink = x;
}
