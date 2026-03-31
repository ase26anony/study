/* tree-pretty-print-annotations.c
 * 
 * This program is designed to generate ANNOTATE_EXPR nodes with specific
 * annot_expr_kind values to trigger the uncovered pretty-printing logic
 * in tree-pretty-print.cc lines 3473-3486.
 *
 * Compile with: gcc -O1 -fopenmp -fopenacc -fdump-tree-original -c tree-pretty-print-annotations.c
 * Then inspect the .original dump file for annotation expressions.
 */

#include <stdio.h>
#include <stdlib.h>

/* External function to prevent loop optimization */
extern void bar(int);

/* Function with optimization attribute to combine with annotations */
__attribute__((optimize("O3")))
void process_annotations(int n, float* arr1, float* arr2) {
    volatile int sum = 0;
    int i, j;
    
    /* 1. annot_expr_no_vector_kind: #pragma GCC unroll 0 */
    #pragma GCC unroll 0
    for (i = 0; i < n; i++) {
        sum += arr1[i] > 0.5f ? 1 : 0;
        if (sum > 1000) break;  /* Early exit to test complex flow */
    }
    
    /* 2. annot_expr_vector_kind: #pragma omp simd */
    #pragma omp simd reduction(+:sum)
    for (i = 0; i < n; i++) {
        arr2[i] = arr1[i] * 2.0f;
        sum += (int)arr2[i];
    }
    
    /* 3. annot_expr_parallel_kind: #pragma omp parallel for */
    #pragma omp parallel for private(j) reduction(+:sum)
    for (i = 0; i < n; i++) {
        int local_sum = 0;
        for (j = 0; j < 10; j++) {
            local_sum += arr1[i] * j;
        }
        sum += local_sum;
        bar(i);  /* External call to prevent optimization */
    }
    
    /* 4. annot_expr_maybe_infinite_kind: while loop with OpenACC */
    int counter = 0;
    #pragma acc kernels loop
    while (counter < n) {  /* Non-constant bound in OpenACC context */
        if (arr1[counter] < 0) continue;  /* Skip negative values */
        sum += counter;
        counter++;
        if (counter > 10000) break;  /* Safety break */
    }
    
    /* 5. Combined annotations: nested pragmas */
    #pragma omp parallel
    {
        #pragma omp for simd nowait
        for (i = 0; i < n; i++) {
            arr1[i] = arr2[i] / 2.0f;
        }
    }
    
    /* 6. Pragma on compound statement containing loop */
    #pragma omp simd
    {
        float max_val = arr1[0];
        for (i = 1; i < n; i++) {
            if (arr1[i] > max_val) max_val = arr1[i];
        }
        sum += (int)max_val;
    }
    
    /* 7. Control case: loop without pragma */
    for (i = 0; i < 5; i++) {
        sum += i * 2;
    }
    
    /* 8. Edge case: pragma on empty loop body */
    #pragma omp simd
    for (i = 0; i < n; i++) {
        /* Empty body - still should generate annotation */
    }
    
    /* 9. OpenACC parallel loop for another parallel_kind case */
    #pragma acc parallel loop reduction(+:sum)
    for (i = 0; i < n; i++) {
        sum += (int)(arr1[i] * 100.0f);
    }
    
    /* 10. do-while loop with annotation */
    i = 0;
    #pragma GCC unroll 0
    do {
        sum -= i;
        i++;
    } while (i < 5);
    
    printf("Processed sum: %d\n", sum);
}

/* Main function with various annotated loops */
int main() {
    const int N = 1000;
    float array1[N], array2[N];
    int total = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        array1[i] = (float)rand() / RAND_MAX;
        array2[i] = 0.0f;
    }
    
    /* Call function with annotated loops */
    process_annotations(N, array1, array2);
    
    /* Additional annotated loops in main */
    
    /* annot_expr_vector_kind with reduction clause */
    #pragma omp for simd reduction(+:total)
    for (int i = 0; i < N; i++) {
        total += (int)(array1[i] * 10);
    }
    
    /* annot_expr_parallel_kind with teams distribute */
    #pragma omp target teams distribute parallel for map(tofrom:total)
    for (int i = 0; i < N; i++) {
        total += i;
    }
    
    /* annot_expr_no_vector_kind on while loop */
    int k = 0;
    #pragma nounroll
    while (k < 100) {
        total += k;
        k++;
        if (k % 7 == 0) continue;  /* Test continue statement */
    }
    
    /* Final result */
    printf("Final total: %d\n", total);
    
    return 0;
}

/* Compilation instructions for maximum coverage:
 *
 * 1. Basic annotation generation:
 *    gcc -O1 -fopenmp -fdump-tree-original -c tree-pretty-print-annotations.c
 *
 * 2. With OpenACC support:
 *    gcc -O1 -fopenmp -fopenacc -fdump-tree-gimple -c tree-pretty-print-annotations.c
 *
 * 3. Full optimization with all dumps:
 *    gcc -O3 -fopenmp -fopenacc -fdump-tree-all -c tree-pretty-print-annotations.c
 *
 * 4. For C++ compilation (rename to .cc):
 *    g++ -std=c++11 -O2 -fopenmp -ftree-parallelize-loops=2 -fdump-tree-optimized -c tree-pretty-print-annotations.cc
 *
 * After compilation, check the .original, .gimple, or .optimized dump files
 * for ANNOTATE_EXPR nodes and their pretty-printed representations.
 */
