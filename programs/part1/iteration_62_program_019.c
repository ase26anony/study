/* test-annot-expr-kinds.c
 * 
 * This program is designed to generate ANNOTATE_EXPR nodes with specific
 * annot_expr_kind values to trigger the uncovered lines in tree-pretty-print.cc.
 * Compile with: gcc -O1 -fopenmp -fopenacc -fdump-tree-original test-annot-expr-kinds.c
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
    for (i = 0; i < n; i++) {
        if (i % 2 == 0) continue;  /* Complex flow */
        c[i] = a[i] + b[i];
        sum += c[i];
        bar(i);
    }

    /* 2. annot_expr_vector_kind: #pragma omp simd */
    #pragma omp simd reduction(+:sum)
    for (i = 0; i < n; i++) {
        c[i] = a[i] * b[i];
        sum += c[i];
    }

    /* 3. annot_expr_parallel_kind: #pragma omp parallel for */
    #pragma omp parallel for reduction(+:sum)
    for (i = 0; i < n; i++) {
        if (c[i] > 100.0f) break;  /* Early exit */
        c[i] = a[i] - b[i];
        sum += c[i];
    }

    /* 4. Nested pragmas: parallel + simd (should generate multiple annotations) */
    #pragma omp parallel
    {
        #pragma omp for simd reduction(+:sum) nowait
        for (i = 0; i < n; i++) {
            c[i] = a[i] / (b[i] + 1.0f);
            sum += c[i];
        }
    }

    printf("Intermediate sum: %.2f\n", sum);
}

void test_maybe_infinite(int limit) {
    int count = 0;
    volatile int total = 0;

    /* 5. annot_expr_maybe_infinite_kind: while loop with non-constant bound */
    #pragma omp target teams distribute parallel for
    while (count < limit) {  /* Non-constant bound */
        total += count;
        count++;
        bar(count);
        if (count > 1000) break;
    }

    /* 6. OpenACC version for maybe-infinite */
    #pragma acc kernels loop
    do {
        total -= count;
        count--;
        if (count < 0) break;
    } while (count > 0);

    printf("Total from maybe-infinite loops: %d\n", total);
}

void edge_cases(void) {
    int arr[10] = {0};
    int i, j;

    /* 7. Pragma on compound statement containing loop */
    #pragma omp parallel
    {
        #pragma omp for simd
        for (i = 0; i < 10; i++) {
            arr[i] = i * 2;
        }
    }

    /* 8. Empty loop body with pragma */
    #pragma omp simd
    for (i = 0; i < 10; i++) {
        /* Empty body - still should generate annotation */
    }

    /* 9. Control case: loop without pragma */
    for (j = 0; j < 10; j++) {
        arr[j] += j;
        bar(arr[j]);
    }

    /* 10. Invalid pragma syntax (should be ignored but may create node) */
    #pragma omp invalid_clause
    for (i = 0; i < 5; i++) {
        arr[i] = 0;
    }
}

int main(void) {
    const int N = 100;
    float *a = malloc(N * sizeof(float));
    float *b = malloc(N * sizeof(float));
    float *c = malloc(N * sizeof(float));

    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(N - i);
        c[i] = 0.0f;
    }

    /* Execute functions to generate various annotation kinds */
    process_data(N, a, b, c);
    test_maybe_infinite(N / 2);
    edge_cases();

    /* Final computation with reduction */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum)
    for (int i = 0; i < N; i++) {
        final_sum += c[i];
    }

    printf("Final result: %.2f\n", final_sum);

    free(a);
    free(b);
    free(c);
    return 0;
}

/* Dummy external function definition */
void bar(int x) {
    /* Prevent optimization */
    volatile static int counter = 0;
    counter += x;
}
