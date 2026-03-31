/* test_omp_clauses.c
 * 
 * This program is designed to trigger GCC's tree pretty-printer
 * for specific OpenMP clause types: for, parallel, sections, and taskgroup.
 * When compiled with -fdump-tree-all -fopenmp, the compiler should
 * generate dump files containing pretty-printed representations
 * of these clauses in the intermediate representation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper to prevent dead code elimination */
static int use_result(int value) {
    volatile int sink = value;
    return sink;
}

int main(int argc, char **argv) {
    /* Use argc for runtime variability to prevent optimization */
    volatile int seed = argc;
    int i, sum = 0;
    const int N = 100;
    int array[N];
    
    /* Initialize array with seed-dependent values */
    for (i = 0; i < N; i++) {
        array[i] = (i * seed) % 97;
    }
    
    /* 1. OMP_CLAUSE_PARALLEL 
     * Use a simple parallel region with shared variable update */
    #pragma omp parallel num_threads(2) shared(sum)
    {
        #pragma omp atomic
        sum += seed;
    }
    
    /* 2. OMP_CLAUSE_FOR 
     * Use parallel for with reduction */
    #pragma omp parallel for reduction(+:sum) private(i)
    for (i = 0; i < N; i++) {
        sum += array[i] % 10;
    }
    
    /* 3. OMP_CLAUSE_SECTIONS 
     * Use parallel sections with different operations */
    #pragma omp parallel sections shared(array, sum)
    {
        #pragma omp section
        {
            for (i = 0; i < N/2; i++) {
                array[i] += seed;
            }
        }
        
        #pragma omp section
        {
            for (i = N/2; i < N; i++) {
                array[i] -= seed;
            }
        }
    }
    
    /* 4. OMP_CLAUSE_TASKGROUP 
     * Use taskgroup with nested tasks */
    #pragma omp parallel shared(array, sum)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (i = 0; i < N; i += 10) {
                    #pragma omp task firstprivate(i) shared(array, sum)
                    {
                        int j;
                        for (j = i; j < i + 10 && j < N; j++) {
                            #pragma omp atomic
                            array[j] += 1;
                        }
                    }
                }
            }
        }
    }
    
    /* Final computation to use all results */
    int final_sum = 0;
    for (i = 0; i < N; i++) {
        final_sum += array[i];
    }
    final_sum += sum;
    
    /* Print result to ensure all code is live */
    printf("Result: %d (seed was %d)\n", use_result(final_sum), seed);
    
    return 0;
}
