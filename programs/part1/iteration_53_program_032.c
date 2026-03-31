/* test-annotate-expr.c
 * This program is designed to trigger the ANNOTATE_EXPR pretty-printer
 * for all four uncovered annotation kinds in GCC's tree-pretty-print.cc.
 * Compile with: gcc -O2 -ftree-vectorize -fdump-tree-vect -fdump-tree-optimized test-annotate-expr.c -o test
 * Or for more dumps: gcc -O1 -fdump-tree-all -c test-annotate-expr.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
static volatile int global_sink = 0;
extern void dummy_extern(int);

void dummy_extern(int x) {
    global_sink += x;
}

int main(void) {
    const int N = 256;
    int i, j;
    int sum = 0;
    
    /* Arrays for loop computations */
    int a[N], b[N], c[N];
    int partial_sums[4] = {0};
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        a[i] = i + 1;
        b[i] = 0;
        c[i] = (i % 3) + 1;
    }
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC ivdep
     *    Loop with potential dependencies that we assert are safe */
    #pragma GCC ivdep
    for (i = 1; i < N; i++) {
        /* This appears to have a dependency, but we assert it's safe */
        a[i] = a[i-1] + c[i];
    }
    dummy_extern(a[N-1]);  /* Prevent dead code elimination */
    
    /* 2. annot_expr_vector_kind: #pragma GCC vector
     *    Simple vectorizable computation */
    #pragma GCC vector
    for (i = 0; i < N; i++) {
        b[i] = a[i] * 2 + 5;
    }
    dummy_extern(b[0]);  /* Use result */
    
    /* 3. annot_expr_parallel_kind: #pragma GCC parallel
     *    Independent iterations accumulating into thread-local storage */
    #pragma GCC parallel
    for (i = 0; i < N; i++) {
        /* Simulate thread-local accumulation */
        partial_sums[i % 4] += b[i] * 3;
    }
    for (j = 0; j < 4; j++) {
        sum += partial_sums[j];
    }
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll
     *    Loop with variable bound - compiler may consider unrolling */
    int limit = 8;
    int product = 1;
    #pragma GCC unroll
    for (i = 1; i <= limit; i++) {
        product *= i;
    }
    sum += product;
    
    /* Nested loop with annotation on inner loop */
    #pragma GCC ivdep
    for (i = 0; i < 10; i++) {
        #pragma GCC vector
        for (j = 0; j < 10; j++) {
            c[i*10 + j] += i * j;
        }
    }
    dummy_extern(c[0]);
    
    /* Loop inside conditional with annotation */
    if (sum > 0) {
        #pragma GCC parallel
        for (i = 0; i < 100; i++) {
            sum += i % 7;
        }
    }
    
    /* Mixed: unroll with no count followed by vector */
    #pragma GCC unroll
    for (i = 0; i < 5; i++) {
        sum += i * 2;
    }
    
    #pragma GCC vector
    for (i = 0; i < 10; i++) {
        sum -= i;
    }
    
    /* Final output to ensure execution */
    printf("Result: %d\n", sum);
    return 0;
}
