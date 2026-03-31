/* test-annotate-expr.c
 * This program is designed to trigger the ANNOTATE_EXPR pretty-printer
 * for all four uncovered annotation kinds in GCC's tree-pretty-print.cc.
 * Compile with: gcc -O2 -ftree-vectorize -fdump-tree-all -c test-annotate-expr.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
static void use(int val) {
    volatile int sink = val;
    (void)sink;
}

/* External function call to create side effects */
extern void external_side_effect(int);

#define N 1024
#define M 8

int main(void) {
    int i, j;
    int a[N], b[N], c[N];
    int partial[4] = {0};
    int prod = 1;
    volatile int result = 0;
    
    /* Initialize arrays */
    for (i = 0; i < N; ++i) {
        a[i] = i % 100;
        b[i] = (i * 2) % 100;
        c[i] = 0;
    }
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC ivdep 
     *    Loop with potential dependencies that we assert are safe */
    #pragma GCC ivdep
    for (i = 1; i < N; ++i) {
        /* This looks like a recurrence, but we assert it's safe with ivdep */
        a[i] = a[i] + a[i-1] * 0;  /* Actually independent due to *0 */
        external_side_effect(a[i]);
    }
    result += a[N-1];
    use(result);
    
    /* 2. annot_expr_vector_kind: #pragma GCC vector 
     *    Simple vectorizable computation */
    #pragma GCC vector
    for (i = 0; i < N; ++i) {
        c[i] = a[i] * 2 + b[i];
    }
    result += c[N/2];
    use(result);
    
    /* 3. annot_expr_parallel_kind: #pragma GCC parallel 
     *    Independent accumulations into array */
    #pragma GCC parallel
    for (i = 0; i < N; ++i) {
        partial[i % 4] += b[i] * 3;
    }
    for (i = 0; i < 4; ++i) {
        result += partial[i];
    }
    use(result);
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll 
     *    Loop with variable bound - compiler may try to unroll */
    {
        int limit = M;
        #pragma GCC unroll
        for (j = 0; j < limit; ++j) {
            prod *= (j + 1);
            external_side_effect(prod);
        }
        result += prod;
    }
    use(result);
    
    /* Nested loop with annotation on inner loop */
    {
        int sum = 0;
        for (i = 0; i < 10; ++i) {
            #pragma GCC ivdep
            for (j = 0; j < N/10; ++j) {
                sum += a[i*N/10 + j] * i;
            }
        }
        result += sum;
    }
    
    /* Annotated loop inside conditional */
    if (result > 0) {
        #pragma GCC vector
        for (i = 0; i < 100; ++i) {
            b[i] = b[i] / 2;
        }
        result += b[50];
    }
    
    /* Mixed: loop with multiple annotations (last one wins) */
    #pragma GCC ivdep
    #pragma GCC vector
    for (i = 0; i < N; i += 2) {
        a[i] = b[i] + c[i];
    }
    result += a[100];
    
    printf("Final result: %d\n", result);
    return 0;
}

/* Dummy definition to satisfy external reference */
void external_side_effect(int x) {
    volatile int dummy = x;
    (void)dummy;
}
