/* test-annot-expr-kinds.c
 * 
 * This program is designed to trigger the creation of ANNOTATE_EXPR nodes
 * with specific annot_expr_kind values in GCC's tree representation.
 * The goal is to cover the switch cases in tree-pretty-print.cc lines 3473-3486.
 * 
 * Compile with: gcc -O1 -fopenmp -fopenacc -fdump-tree-original test-annot-expr-kinds.c
 * Then examine the .original dump file for annotation expressions.
 */

#include <stdio.h>
#include <stdlib.h>

/* External function to prevent loop removal */
extern void bar(int);

/* Function with optimization attribute */
__attribute__((optimize("O3")))
void process_data(int n, float* a, float* b, float* c) {
    volatile float sum = 0.0f;
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC unroll 0 */
    #pragma GCC unroll 0
    for (int i = 0; i < n; ++i) {
        if (i % 2 == 0) continue;  /* Complex flow */
        sum += a[i];
        bar(i);
    }
    
    /* 2. annot_expr_vector_kind: #pragma omp simd */
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; ++i) {
        sum += b[i] * c[i];
    }
    
    /* 3. annot_expr_parallel_kind: #pragma omp parallel for */
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; ++i) {
        if (i == n/2) break;  /* Early exit */
        sum += a[i] + b[i];
    }
    
    /* 4. Control case: no pragma */
    for (int i = 0; i < 10; ++i) {
        sum += i;
    }
    
    printf("Intermediate sum: %.2f\n", sum);
}

/* Function with nested pragmas */
void nested_pragmas(int n, int* arr) {
    int total = 0;
    
    /* Nested: parallel with inner simd */
    #pragma omp parallel
    {
        #pragma omp for simd reduction(+:total) nowait
        for (int i = 0; i < n; ++i) {
            arr[i] = i * 2;
            total += arr[i];
        }
    }
    
    /* Pragma on compound statement containing loop */
    #pragma omp parallel
    {
        #pragma omp for
        {
            for (int i = 0; i < n; i += 2) {
                arr[i] += total;
            }
        }
    }
}

/* Function targeting annot_expr_maybe_infinite_kind */
void maybe_infinite_loops(int limit) {
    int count = 0;
    volatile int accumulator = 0;
    
    /* 5. annot_expr_maybe_infinite_kind: while with non-constant bound */
    int x = limit;
    #pragma omp target teams distribute parallel for
    while (x-- > 0) {
        accumulator += x;
        if (accumulator > 1000) break;
        bar(count++);
    }
    
    /* OpenACC version */
    int y = limit * 2;
    #pragma acc kernels loop
    do {
        accumulator -= y;
        y--;
    } while (y > 0 && accumulator > -1000);
    
    printf("Accumulator: %d\n", accumulator);
}

/* Edge cases */
void edge_cases(void) {
    /* Pragma on empty loop */
    #pragma omp simd
    for (int i = 0; i < 5; ++i) {
        /* Empty body */
    }
    
    /* Pragma with invalid syntax (should be ignored) */
    #pragma omp invalid_clause
    for (int i = 0; i < 3; ++i) {
        bar(i);
    }
    
    /* Different loop forms */
    int j = 0;
    #pragma omp simd
    while (j < 10) {
        bar(j++);
    }
    
    int k = 0;
    #pragma GCC unroll 0
    do {
        bar(k++);
    } while (k < 5);
}

int main(void) {
    const int N = 100;
    float a[N], b[N], c[N];
    int arr[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; ++i) {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
        c[i] = (float)(i * 3);
        arr[i] = i;
    }
    
    /* Execute functions to generate annotation expressions */
    process_data(N, a, b, c);
    nested_pragmas(N, arr);
    maybe_infinite_loops(50);
    edge_cases();
    
    /* Final computation with reduction */
    int final_sum = 0;
    #pragma omp parallel for reduction(+:final_sum)
    for (int i = 0; i < N; ++i) {
        final_sum += arr[i];
    }
    
    printf("Final result: %d\n", final_sum);
    return 0;
}

/* Dummy implementation of bar() to satisfy extern declaration */
void bar(int x) {
    /* Prevent optimization */
    volatile static int counter = 0;
    counter += x;
}
