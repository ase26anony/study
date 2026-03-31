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
void test_annotations(void) {
    volatile int sum = 0;
    int arr[100];
    int i, j;
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        arr[i] = i + 1;
    }
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC unroll 0 */
    #pragma GCC unroll 0
    for (i = 0; i < 100; i++) {
        sum += arr[i];
        bar(i);  /* Prevent optimization */
    }
    
    /* 2. annot_expr_vector_kind: #pragma omp simd */
    sum = 0;
    #pragma omp simd reduction(+:sum)
    for (i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    /* 3. annot_expr_parallel_kind: #pragma omp parallel for */
    sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (i = 0; i < 100; i++) {
        sum += arr[i] * 2;
    }
    
    /* 4. annot_expr_maybe_infinite_kind: while loop with OpenACC */
    sum = 0;
    i = 0;
    #pragma acc kernels loop
    while (i < 100) {  /* Non-constant bound */
        sum += arr[i];
        i++;
        if (i > 150) break;  /* Safety */
    }
    
    /* 5. Nested pragmas: parallel with inner simd */
    sum = 0;
    #pragma omp parallel
    {
        #pragma omp for simd reduction(+:sum)
        for (i = 0; i < 100; i++) {
            sum += arr[i] * 3;
        }
    }
    
    /* 6. Pragma on compound statement containing loop */
    sum = 0;
    #pragma omp simd
    {
        for (i = 0; i < 100; i++) {
            sum += arr[i] * 4;
        }
    }
    
    /* 7. do-while loop with pragma */
    sum = 0;
    i = 0;
    #pragma omp simd
    do {
        sum += arr[i];
        i++;
    } while (i < 100);
    
    /* 8. Loop with early exit and pragma */
    sum = 0;
    #pragma omp simd
    for (i = 0; i < 100; i++) {
        if (arr[i] > 50) break;
        sum += arr[i];
    }
    
    /* 9. Empty loop body with pragma */
    #pragma omp simd
    for (i = 0; i < 100; i++) {
        /* Empty but should still generate annotation */
    }
    
    /* 10. Control case: loop without pragma */
    sum = 0;
    for (i = 0; i < 100; i++) {
        sum += arr[i] * 5;
    }
    
    /* 11. OpenACC parallel loop for parallel_kind */
    sum = 0;
    #pragma acc parallel loop reduction(+:sum)
    for (i = 0; i < 100; i++) {
        sum += arr[i] * 6;
    }
    
    /* 12. #pragma omp for simd for vector_kind */
    sum = 0;
    #pragma omp for simd reduction(+:sum)
    for (i = 0; i < 100; i++) {
        sum += arr[i] * 7;
    }
    
    /* 13. #pragma nounroll for no-vector_kind */
    sum = 0;
    #pragma nounroll
    for (i = 0; i < 100; i++) {
        sum += arr[i] * 8;
    }
    
    /* 14. Complex loop with multiple exits and pragma */
    sum = 0;
    #pragma omp simd
    for (i = 0; i < 100; i++) {
        if (i % 10 == 0) continue;
        sum += arr[i];
        if (sum > 1000) break;
    }
    
    /* 15. #pragma omp target teams distribute parallel for for maybe-infinite */
    sum = 0;
    #pragma omp target teams distribute parallel for reduction(+:sum)
    for (i = 0; i < 100; i++) {
        sum += arr[i] * 9;
    }
    
    printf("Final sum: %d\n", sum);
}

int main(void) {
    test_annotations();
    return 0;
}

/* Dummy implementation of bar to satisfy external reference */
void bar(int x) {
    /* Do nothing, just prevent optimization */
    volatile int dummy = x;
    (void)dummy;
}
