/* test_openmp_clauses.c
 * 
 * This program is designed to trigger GCC's tree pretty-printer
 * for specific OpenMP clause types: for, parallel, sections, and taskgroup.
 * When compiled with -fdump-tree-all or similar flags, the compiler's
 * internal representation should contain OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP nodes, causing the
 * pretty-printer to output their names.
 *
 * Compile with: gcc -O1 -fopenmp -fdump-tree-all -fdump-tree-omp test_openmp_clauses.c -o test_omp
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char **argv) {
    /* Use argc to introduce runtime variability and prevent optimization */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    int i, sum = 0;
    const int N = 100;
    int array[N];
    
    /* 1. OMP_CLAUSE_PARALLEL 
     * Use a simple parallel region with shared variable update */
    #pragma omp parallel num_threads(2)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        seed += tid;  /* Use volatile seed to prevent dead code elimination */
    }
    
    /* 2. OMP_CLAUSE_FOR 
     * Use parallel for loop with reduction */
    #pragma omp parallel for reduction(+:sum) private(i) schedule(static)
    for (i = 0; i < N; i++) {
        array[i] = (i * seed) % 100;  /* Computation depends on volatile seed */
        sum += array[i];
    }
    
    /* 3. OMP_CLAUSE_SECTIONS 
     * Use parallel sections with different operations */
    #pragma omp parallel sections private(i)
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
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (i = 0; i < N; i += 10) {
                    #pragma omp task firstprivate(i) shared(array)
                    {
                        /* Each task processes a chunk of the array */
                        for (int j = i; j < i + 10 && j < N; j++) {
                            array[j] = (array[j] * 2) % 1000;
                        }
                    }
                }
            } /* end taskgroup */
        } /* end single */
    } /* end parallel */
    
    /* Final computation to ensure all results are used */
    int final_sum = 0;
    for (i = 0; i < N; i++) {
        final_sum += array[i];
    }
    
    printf("Seed: %d, Initial sum: %d, Final sum: %d\n", seed, sum, final_sum);
    
    return (final_sum > 0) ? 0 : 1;  /* Return value depends on computation */
}
