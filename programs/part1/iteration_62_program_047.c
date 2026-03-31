/* test-annot-expr-kinds.c
 * 
 * This program is designed to trigger the creation of ANNOTATE_EXPR nodes
 * with specific annot_expr_kind values in GCC's tree representation,
 * targeting the uncovered pretty-printing switch cases in tree-pretty-print.cc.
 * 
 * Compile with: gcc -O1 -fopenmp -fopenacc -fdump-tree-original -c test-annot-expr-kinds.c
 * Then inspect the generated .original dump file for annotation expressions.
 */

#include <stdio.h>
#include <stdlib.h>

/* External function to prevent loop removal */
extern void bar(int);

/* Function with optimization attribute to combine with pragmas */
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
    #pragma omp simd reduction(+:sum)
    for (i = 0; i < 100; i++) {
        sum += arr[i] * 2;
    }
    
    /* 3. annot_expr_parallel_kind: #pragma omp parallel for */
    #pragma omp parallel for reduction(+:sum) private(j)
    for (i = 0; i < 50; i++) {
        j = i * 2;
        sum += arr[j] + arr[j + 1];
        if (sum > 1000) continue;  /* Test with continue */
    }
    
    /* 4. annot_expr_maybe_infinite_kind: while loop with OpenACC */
    int counter = 0;
    #pragma acc kernels loop
    while (counter < 100) {  /* Non-constant bound */
        sum += counter;
        counter++;
        if (counter == 50) break;  /* Test with break */
    }
    
    /* 5. Compound statement with pragma */
    #pragma omp parallel
    {
        #pragma omp for simd  /* Nested: parallel + vector */
        for (i = 0; i < 100; i++) {
            arr[i] = arr[i] * 3;
        }
    }
    
    /* 6. Do-while loop with pragma */
    i = 0;
    #pragma omp simd
    do {
        sum -= arr[i];
        i++;
    } while (i < 100);
    
    /* 7. Empty loop body with pragma (edge case) */
    #pragma GCC unroll 0
    for (i = 0; i < 10; i++) {
        /* Empty body */
    }
    
    /* 8. Control case: loop without pragma */
    for (i = 0; i < 10; i++) {
        sum += i;
    }
    
    /* 9. OpenACC parallel loop */
    #pragma acc parallel loop reduction(+:sum)
    for (i = 0; i < 100; i++) {
        sum += arr[i] / 2;
    }
    
    /* 10. Complex nesting: target teams with distribute */
    #pragma omp target teams distribute parallel for map(tofrom:sum)
    for (i = 0; i < 100; i++) {
        sum += arr[i] % 7;
    }
    
    printf("Result: %d\n", sum);
}

int main(void) {
    test_annotations();
    return 0;
}
