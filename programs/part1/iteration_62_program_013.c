/* test-annot-expr-kinds.c
 * 
 * This program is designed to trigger the creation of ANNOTATE_EXPR nodes
 * with specific annot_expr_kind values to cover the switch cases in
 * tree-pretty-print.cc lines 3473-3486.
 *
 * Compile with: gcc -O1 -fopenmp -fopenacc -fdump-tree-original test-annot-expr-kinds.c
 * Then inspect the .original dump file for annotation expressions.
 */

#include <stdio.h>
#include <stdlib.h>

/* External function to prevent loop removal */
extern void bar(int);

/* Function with optimization attribute */
__attribute__((optimize("O3")))
void process_data(int n, float* restrict a, float* restrict b, float* restrict c) {
    volatile float sum = 0.0f;
    int i;

    /* 1. annot_expr_no_vector_kind: #pragma GCC unroll 0 */
    #pragma GCC unroll 0
    for (i = 0; i < n; ++i) {
        c[i] = a[i] + b[i];
        sum += c[i]; /* volatile side effect */
        if (c[i] > 100.0f) break; /* early exit */
    }

    /* 2. annot_expr_vector_kind: #pragma omp simd */
    #pragma omp simd reduction(+:sum)
    for (i = 0; i < n; ++i) {
        c[i] = a[i] * b[i];
        sum += c[i];
        bar(i); /* external call */
    }

    /* 3. annot_expr_parallel_kind: #pragma omp parallel for */
    #pragma omp parallel for reduction(+:sum) private(i)
    for (i = 0; i < n; ++i) {
        c[i] = a[i] - b[i];
        sum += c[i];
        if (i % 2 == 0) continue; /* skip */
    }

    /* 4. annot_expr_maybe_infinite_kind: while loop with OpenACC */
    int j = 0;
    #pragma acc kernels loop
    while (j < n) { /* non-constant bound in general case */
        c[j] = a[j] / (b[j] + 1.0f);
        sum += c[j];
        ++j;
        if (j > 1000) break; /* safety */
    }

    printf("Intermediate sum: %.2f\n", sum);
}

/* Another function with nested pragmas */
void nested_pragmas(int n, int* arr) {
    int i, total = 0;

    /* Compound statement with pragma */
    #pragma omp parallel
    {
        #pragma omp for simd reduction(+:total) nowait
        for (i = 0; i < n; ++i) {
            arr[i] *= 2;
            total += arr[i];
        }
    }

    /* Empty loop body with pragma (edge case) */
    #pragma omp simd
    for (i = 0; i < n; ++i) {
        /* empty */
    }

    /* do-while loop with unroll */
    i = 0;
    #pragma GCC unroll 0
    do {
        arr[i] += i;
        bar(arr[i]);
        i++;
    } while (i < n);

    printf("Total: %d\n", total);
}

/* Control function without pragmas */
void control_loop(int n, double* d) {
    double acc = 0.0;
    for (int i = 0; i < n; ++i) {
        d[i] = d[i] * 1.5;
        acc += d[i];
    }
    printf("Control acc: %.2f\n", acc);
}

int main(void) {
    const int N = 1000;
    float a[N], b[N], c[N];
    int arr[N];
    double d[N];

    /* Initialize arrays */
    for (int i = 0; i < N; ++i) {
        a[i] = (float)i;
        b[i] = (float)(N - i);
        arr[i] = i % 100;
        d[i] = (double)i * 0.5;
    }

    /* Trigger various annotation kinds */
    process_data(N, a, b, c);
    nested_pragmas(N, arr);
    control_loop(N, d);

    /* Final computation with OpenACC parallel loop */
    float final_sum = 0.0f;
    #pragma acc parallel loop reduction(+:final_sum)
    for (int i = 0; i < N; ++i) {
        final_sum += a[i] + c[i];
    }

    printf("Result: %.2f\n", final_sum);
    return 0;
}
