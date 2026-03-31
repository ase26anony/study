/* test-annot-expr-kinds.c
 * 
 * This program is designed to generate ANNOTATE_EXPR nodes with specific
 * annot_expr_kind values to trigger the uncovered pretty-printing logic
 * in tree-pretty-print.cc lines 3473-3486.
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
void process_data(int n, float* restrict a, float* restrict b, float* restrict c) {
    volatile float sum = 0.0f;
    int i;
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC unroll 0 */
    #pragma GCC unroll 0
    for (i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
        sum += c[i];  /* Side effect to prevent removal */
        if (c[i] > 100.0f) break;  /* Early exit */
    }
    
    /* 2. annot_expr_vector_kind: #pragma omp simd */
    #pragma omp simd reduction(+:sum)
    for (i = 0; i < n; i++) {
        a[i] = b[i] * c[i];
        sum += a[i];
        bar(i);  /* External call */
    }
    
    /* 3. annot_expr_parallel_kind: #pragma omp parallel for */
    #pragma omp parallel for reduction(+:sum) private(i)
    for (i = 0; i < n; i++) {
        b[i] = a[i] / (c[i] + 1.0f);
        sum += b[i];
        if (i % 2 == 0) continue;  /* Skip some iterations */
    }
    
    /* 4. annot_expr_maybe_infinite_kind: while loop with OpenACC */
    int count = 0;
    #pragma acc kernels loop
    while (count < n) {  /* Non-constant bound */
        c[count] = a[count] - b[count];
        sum += c[count];
        count++;
        if (count > 1000) break;  /* Safety */
    }
    
    printf("Intermediate sum: %f\n", sum);
}

/* Another function with nested pragmas */
void nested_pragmas(int n, double* x, double* y) {
    double dot = 0.0;
    
    /* Nested: parallel with inner simd */
    #pragma omp parallel
    {
        #pragma omp for simd reduction(+:dot)
        for (int i = 0; i < n; i++) {
            dot += x[i] * y[i];
        }
    }
    
    /* Pragma on compound statement containing loop */
    #pragma omp simd
    {
        for (int i = 0; i < n; i++) {
            x[i] = y[i] + dot;
        }
    }
    
    /* Empty loop body with pragma */
    #pragma omp simd
    for (int i = 0; i < 10; i++) {
        /* Empty but should still generate annotation */
    }
}

/* Function with OpenACC pragmas */
void openacc_test(int n, int* arr) {
    int max_val = 0;
    
    #pragma acc parallel loop reduction(max:max_val)
    for (int i = 0; i < n; i++) {
        if (arr[i] > max_val) max_val = arr[i];
    }
    
    /* do-while loop with pragma */
    int j = 0;
    #pragma omp simd
    do {
        arr[j] += max_val;
        j++;
    } while (j < n);
}

int main() {
    const int N = 1000;
    static float a[N], b[N], c[N];
    double x[N], y[N];
    int arr[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
        c[i] = 0.0f;
        x[i] = (double)i / N;
        y[i] = (double)(i % 10);
        arr[i] = i * 3;
    }
    
    /* 5. Control case: loop without any pragma */
    float check = 0.0f;
    for (int i = 0; i < N; i++) {
        check += a[i] + b[i];
    }
    
    /* Process with various annotation kinds */
    process_data(N, a, b, c);
    nested_pragmas(N, x, y);
    openacc_test(N, arr);
    
    /* Final computation using all results */
    double final_result = (double)check;
    for (int i = 0; i < N; i++) {
        final_result += (double)c[i] + x[i] + (double)arr[i];
    }
    
    printf("Final result: %f\n", final_result);
    return 0;
}

/* Invalid pragma (should be ignored but may create annotation) */
#pragma invalid_pragma_option  /* This line tests parser behavior */

/* Additional edge case: function with attribute and pragma */
__attribute__((cold))
void cold_function(int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Minimal work */
        bar(i);
    }
}
