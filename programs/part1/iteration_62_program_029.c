/* test-annot-expr-kinds.c
 * 
 * This program is designed to trigger the creation of ANNOTATE_EXPR nodes
 * with specific annot_expr_kind values in GCC's tree representation.
 * When compiled with -fdump-tree-original, the pretty-printer should
 * traverse these nodes and hit each case in the uncovered switch block
 * in tree-pretty-print.cc lines 3473-3486.
 */

#include <stdio.h>
#include <stdlib.h>

/* External function to prevent loop removal */
extern void bar(int);

/* Function with optimization attribute to combine with pragmas */
__attribute__((optimize("O3")))
void test_annotations(int n, float *restrict a, float *restrict b) {
    volatile int check = 0;
    float sum = 0.0f;
    int i;
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC unroll 0 */
    printf("Testing no-vector annotation...\n");
    #pragma GCC unroll 0
    for (i = 0; i < n && i < 100; i++) {
        /* Non-trivial body with early exit possibility */
        if (a[i] < 0) break;
        sum += a[i] * b[i];
        bar(i);
    }
    check += (int)sum;
    
    /* 2. annot_expr_vector_kind: #pragma omp simd */
    printf("Testing vector annotation...\n");
    sum = 0.0f;
    #pragma omp simd reduction(+:sum)
    for (i = 0; i < n; i++) {
        sum += a[i] * 2.0f;
        /* Continue to test annotation with complex flow */
        if (i % 2 == 0) continue;
        b[i] = sum;
    }
    check += (int)sum;
    
    /* 3. annot_expr_parallel_kind: #pragma omp parallel for */
    printf("Testing parallel annotation...\n");
    sum = 0.0f;
    #pragma omp parallel for reduction(+:sum) private(i)
    for (i = 0; i < n; i++) {
        sum += a[i] * 3.0f;
        bar(i);
    }
    check += (int)sum;
    
    /* 4. annot_expr_maybe_infinite_kind: while loop with non-constant bound */
    printf("Testing maybe-infinite annotation...\n");
    sum = 0.0f;
    int count = 0;
    #pragma omp target teams distribute parallel for
    for (i = 0; i < n; i++) {
        sum += a[i];
        count++;
    }
    check += (int)sum;
    
    /* Additional test: nested pragmas */
    printf("Testing nested annotations...\n");
    sum = 0.0f;
    #pragma omp parallel
    {
        #pragma omp for simd reduction(+:sum)
        for (i = 0; i < n; i++) {
            sum += a[i] * b[i];
        }
    }
    check += (int)sum;
    
    /* Test with compound statement block */
    printf("Testing annotation on compound block...\n");
    sum = 0.0f;
    #pragma omp simd
    {
        for (i = 0; i < n && i < 50; i++) {
            sum += a[i];
        }
    }
    check += (int)sum;
    
    /* Test do-while loop with pragma */
    printf("Testing do-while with annotation...\n");
    sum = 0.0f;
    i = 0;
    #pragma omp simd
    do {
        sum += a[i];
        bar(i);
        i++;
    } while (i < n && i < 30);
    check += (int)sum;
    
    /* Control case: loop without pragma */
    printf("Control case (no pragma)...\n");
    sum = 0.0f;
    for (i = 0; i < n && i < 10; i++) {
        sum += a[i];
    }
    check += (int)sum;
    
    /* Use check to prevent dead code elimination */
    printf("Check value: %d\n", check);
}

/* Test with OpenACC pragmas as well */
void test_openacc_annotations(int n, double *restrict c, double *restrict d) {
    double sum = 0.0;
    int i;
    
    /* OpenACC parallel kind */
    #pragma acc parallel loop reduction(+:sum) copy(c[0:n]) copy(d[0:n])
    for (i = 0; i < n; i++) {
        sum += c[i] * d[i];
    }
    
    /* OpenACC kernels loop - may generate maybe-infinite kind */
    #pragma acc kernels loop
    for (i = 0; i < n; i++) {
        c[i] = d[i] * 2.0;
    }
    
    printf("OpenACC sum: %f\n", sum);
}

/* Edge case: pragma on empty loop */
void test_edge_cases(int n, int *arr) {
    int i;
    
    /* Pragma with empty loop body */
    #pragma omp simd
    for (i = 0; i < n; i++) {
        /* Empty body */
    }
    
    /* Pragma with invalid syntax (should be ignored but may create node) */
    #pragma omp invalid_clause
    for (i = 0; i < n; i++) {
        arr[i] = i;
    }
    
    /* While loop with non-constant bound and pragma */
    i = 0;
    #pragma omp target teams distribute parallel for
    while (i < n && arr[i] > 0) {
        arr[i] *= 2;
        i++;
        bar(i);
    }
}

int main() {
    const int N = 1000;
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    double *c = (double*)malloc(N * sizeof(double));
    double *d = (double*)malloc(N * sizeof(double));
    int *arr = (int*)malloc(N * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i % 100) * 0.1f;
        b[i] = (float)(i % 50) * 0.2f;
        c[i] = (double)(i % 75) * 0.3;
        d[i] = (double)(i % 25) * 0.4;
        arr[i] = i;
    }
    
    /* Run tests */
    test_annotations(N, a, b);
    test_openacc_annotations(N, c, d);
    test_edge_cases(N, arr);
    
    /* Compute and print final result */
    float final_sum = 0.0f;
    for (int i = 0; i < N; i++) {
        final_sum += a[i] + b[i] + (float)(c[i] + d[i]) + (float)arr[i];
    }
    
    printf("Final result: %f\n", final_sum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(arr);
    
    return 0;
}

/* Dummy implementation of bar to satisfy extern declaration */
void bar(int x) {
    /* Prevent optimization */
    volatile static int counter = 0;
    counter += x;
}
