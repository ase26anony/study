/* test-annot-expr-kinds.c
 * 
 * This program is designed to trigger the creation of ANNOTATE_EXPR nodes
 * with specific annot_expr_kind values in GCC's tree representation,
 * targeting the uncovered switch cases in tree-pretty-print.cc lines 3473-3486.
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
        c[i] = a[i] + b[i];
        sum += c[i];  /* Side effect to prevent removal */
        if (i % 10 == 0) continue;  /* Complex flow */
    }
    
    /* 2. annot_expr_vector_kind: #pragma omp simd */
    #pragma omp simd reduction(+:sum)
    for (i = 0; i < n; i++) {
        c[i] = a[i] * b[i];
        sum += c[i];
        bar(i);  /* External call */
    }
    
    /* 3. annot_expr_parallel_kind: #pragma omp parallel for */
    #pragma omp parallel for reduction(+:sum) private(i)
    for (i = 0; i < n; i++) {
        c[i] = a[i] - b[i];
        sum += c[i];
        if (c[i] < 0) break;  /* Early exit */
    }
    
    /* 4. Control case: no pragma */
    for (i = 0; i < n; i++) {
        c[i] = a[i] / (b[i] + 1.0f);
        sum += c[i];
    }
}

void test_maybe_infinite(int limit) {
    int count = 0;
    volatile int total = 0;
    
    /* 5. annot_expr_maybe_infinite_kind: non-constant bound with OpenACC */
    #pragma acc kernels loop
    while (count < limit) {  /* Non-constant bound */
        total += count;
        count++;
        if (count > 1000) break;  /* Safety */
    }
    
    /* 6. Another maybe-infinite case with OpenMP teams */
    int j = 0;
    #pragma omp target teams distribute parallel for
    for (j = 0; j < limit; j++) {  /* limit is parameter, not constant */
        total += j * 2;
        bar(j);
    }
}

void nested_pragmas(int n, int* arr) {
    int i, j;
    
    /* 7. Nested pragmas: parallel with inner simd */
    #pragma omp parallel
    {
        #pragma omp for simd nowait
        for (i = 0; i < n; i++) {
            arr[i] *= 2;
            for (j = 0; j < 5; j++) {
                arr[i] += j;
            }
        }
    }
    
    /* 8. Pragmas on compound statements */
    #pragma omp simd
    {
        for (i = 0; i < n; i++) {
            arr[i] += i % 7;
        }
    }
    
    /* 9. Empty loop body with pragma */
    #pragma GCC unroll 0
    for (i = 0; i < n; i++) {
        /* Empty but annotated */
    }
}

/* Edge case: invalid pragma syntax (should be ignored but may create node) */
void invalid_pragma_case(int n, float* data) {
    #pragma omp invalid_clause  /* Invalid, but parser might still annotate */
    for (int i = 0; i < n; i++) {
        data[i] = data[i] * 0.5f;
    }
    
    /* Mixed loop forms */
    int k = 0;
    #pragma omp parallel for
    do {
        data[k] += k;
        k++;
    } while (k < n);
}

int main() {
    const int N = 1000;
    float* a = (float*)malloc(N * sizeof(float));
    float* b = (float*)malloc(N * sizeof(float));
    float* c = (float*)malloc(N * sizeof(float));
    int* arr = (int*)malloc(N * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(N - i);
        c[i] = 0.0f;
        arr[i] = i;
    }
    
    /* Trigger all annotation kinds */
    process_data(N, a, b, c);
    test_maybe_infinite(N / 2);
    nested_pragmas(N, arr);
    invalid_pragma_case(N, c);
    
    /* Compute final result */
    double final_sum = 0.0;
    for (int i = 0; i < N; i++) {
        final_sum += c[i] + arr[i];
    }
    
    printf("Result: %.2f\n", final_sum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(arr);
    
    return 0;
}
