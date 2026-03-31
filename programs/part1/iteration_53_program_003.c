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
extern void external_call(int);

#define SIZE 1024
#define CHUNK 4

int main(void) {
    int i, j, k;
    int result = 0;
    
    /* Arrays for loop computations */
    int a[SIZE], b[SIZE], c[SIZE];
    int partial_sums[CHUNK];
    
    /* Initialize arrays */
    for (i = 0; i < SIZE; i++) {
        a[i] = i % 100;
        b[i] = 0;
        c[i] = 1;
    }
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC ivdep */
    /* Loop with potential dependencies that ivdep overrides */
    #pragma GCC ivdep
    for (i = 1; i < SIZE; i++) {
        /* Apparent dependency that ivdep tells compiler to ignore */
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
    #pragma GCC parallel
    for (i = 0; i < CHUNK; i++) {
        partial_sums[i] = 0;
        for (j = i * (SIZE/CHUNK); j < (i+1) * (SIZE/CHUNK); j++) {
            partial_sums[i] += b[j];
        }
    }
    
    /* Combine partial sums */
    for (i = 0; i < CHUNK; i++) {
        result += partial_sums[i];
        external_call(partial_sums[i]);  /* External call prevents optimization */
    }
    
    /* 4. annot_expr_maybe_infinite_kind: #pragma GCC unroll */
    /* Loop that could be unrolled indefinitely */
    int unroll_count = 8;  /* Variable to prevent compile-time unrolling */
    int product = 1;
    
    #pragma GCC unroll
    for (k = 1; k <= unroll_count; k++) {
        product *= k;
        c[k % SIZE] = product;  /* Side effect to array */
    }
    use(product);
    
    /* Nested loop with annotation */
    #pragma GCC ivdep
    for (i = 0; i < SIZE/2; i++) {
        #pragma GCC vector
        for (j = 0; j < 4; j++) {
            b[i*2 + j] += c[i] * j;
        }
    }
    
    /* Conditional block with annotated loop */
    if (result > 0) {
        #pragma GCC parallel
        for (i = 0; i < 16; i++) {
            result += i * i;
            volatile int sink = result;
            (void)sink;
        }
    } else {
        #pragma GCC unroll
        for (i = 0; i < 4; i++) {
            result -= i;
        }
    }
    
    /* Final computation and output */
    for (i = 0; i < SIZE; i++) {
        result += b[i] % 256;
    }
    
    printf("Result: %d\n", result);
    
    /* While loop with annotation */
    i = 0;
    #pragma GCC ivdep
    while (i < 100) {
        result ^= a[i % SIZE];
        i++;
    }
    
    printf("Final: %d\n", result);
    return result != 0 ? 0 : 1;
}

/* Dummy external function definition */
void external_call(int val) {
    volatile int sink = val;
    (void)sink;
}
