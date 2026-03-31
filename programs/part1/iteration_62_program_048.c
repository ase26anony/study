/* tree-pretty-print-annotations.c
 * 
 * This program is designed to generate ANNOTATE_EXPR nodes with specific
 * annot_expr_kind values to trigger the uncovered pretty-printing logic
 * in tree-pretty-print.cc lines 3473-3486.
 *
 * Compile with: gcc -O1 -fopenmp -fopenacc -fdump-tree-original -c tree-pretty-print-annotations.c
 */

#include <stdio.h>
#include <stdlib.h>

/* External function to prevent loop optimization */
extern void bar(int);

/* Function with optimization attribute */
__attribute__((optimize("O3")))
void annotated_loops(int n, float* results) {
    volatile int vsum = 0;
    int arr[100];
    int i, j;
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        arr[i] = i + 1;
    }
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC unroll 0 */
    #pragma GCC unroll 0
    for (i = 0; i < n; i++) {
        vsum += arr[i % 100];
        if (i == 50) continue;  /* Test with continue */
        bar(i);
    }
    results[0] = vsum;
    
    /* 2. annot_expr_vector_kind: #pragma omp simd */
    vsum = 0;
    #pragma omp simd reduction(+:vsum)
    for (i = 0; i < 100; i++) {
        vsum += arr[i];
        if (i % 10 == 0) bar(i);  /* Non-trivial body */
    }
    results[1] = vsum;
    
    /* 3. annot_expr_parallel_kind: #pragma omp parallel for */
    vsum = 0;
    #pragma omp parallel for reduction(+:vsum)
    for (i = 0; i < 100; i++) {
        vsum += arr[i] * 2;
        /* Early exit test */
        if (i == 75) {
            /* break would exit only this thread's iteration */
            bar(1000);
        }
    }
    results[2] = vsum;
    
    /* 4. annot_expr_maybe_infinite_kind: while loop with non-constant bound */
    vsum = 0;
    i = 0;
    #pragma omp target teams distribute parallel for
    while (i < n) {  /* Non-constant bound */
        vsum += arr[i % 100];
        i++;
        if (vsum > 1000) break;  /* Early exit */
    }
    results[3] = vsum;
    
    /* 5. Nested pragmas: parallel with inner simd */
    vsum = 0;
    #pragma omp parallel
    {
        #pragma omp for simd reduction(+:vsum)
        for (i = 0; i < 100; i++) {
            vsum += arr[i] / 2;
        }
    }
    results[4] = vsum;
    
    /* 6. Compound statement with pragma */
    vsum = 0;
    #pragma omp simd
    {
        for (j = 0; j < 50; j++) {
            vsum += arr[j] + arr[99 - j];
        }
    }
    results[5] = vsum;
    
    /* 7. OpenACC pragma for parallel kind */
    vsum = 0;
    #pragma acc parallel loop reduction(+:vsum)
    for (i = 0; i < 100; i++) {
        vsum += arr[i] * 3;
    }
    results[6] = vsum;
    
    /* 8. do-while loop with pragma */
    vsum = 0;
    i = 0;
    #pragma omp simd
    do {
        vsum += arr[i];
        i++;
    } while (i < 20);
    results[7] = vsum;
    
    /* 9. Empty loop body with pragma (edge case) */
    vsum = 0;
    #pragma GCC unroll 0
    for (i = 0; i < 10; i++) {
        /* Empty body */
    }
    results[8] = vsum;
    
    /* 10. Control case: no pragma */
    vsum = 0;
    for (i = 0; i < 100; i++) {
        vsum += arr[i] % 7;
    }
    results[9] = vsum;
}

int main() {
    float results[10] = {0};
    int n = 100;
    
    /* Call function with annotated loops */
    annotated_loops(n, results);
    
    /* Compute and print predictable result */
    float total = 0;
    for (int i = 0; i < 10; i++) {
        total += results[i];
    }
    
    printf("Result: %f\n", total);
    
    /* Additional test with while loop and OpenACC for maybe-infinite */
    volatile int test = 0;
    int counter = 0;
    
    #pragma acc kernels loop
    while (counter < 50) {  /* Non-constant bound in kernel context */
        test += counter;
        counter++;
        if (test > 100) break;
    }
    printf("While loop result: %d\n", test);
    
    return 0;
}

/* Invalid pragma (should be ignored but may create annotation) */
#pragma invalid_pragma_option  /* This will be ignored */
void unused_func() {
    int x = 0;
}
