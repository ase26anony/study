/* test_omp_clauses.c
 * This program is designed to trigger GCC's tree pretty-printer
 * to output the names of four specific OpenMP clauses:
 *   for, parallel, sections, taskgroup
 * Compile with: gcc -O1 -fopenmp -fdump-tree-all test_omp_clauses.c -o test
 * This will generate dump files containing the pretty-printed OpenMP clauses.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char **argv) {
    /* Use argc to create runtime-dependent computation */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    int i, sum = 0;
    const int N = 100;
    int array[N];
    
    /* Initialize array with seed-dependent values */
    for (i = 0; i < N; i++) {
        array[i] = (i + seed) % 100;
    }
    
    /* 1. OMP_CLAUSE_PARALLEL: Simple parallel region */
    #pragma omp parallel num_threads(2)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        sum += tid;  /* Ensure work in parallel region */
    }
    
    /* 2. OMP_CLAUSE_FOR: parallel for loop */
    #pragma omp parallel for reduction(+:sum) schedule(static)
    for (i = 0; i < N; i++) {
        sum += array[i] * 2;  /* Non-trivial computation */
    }
    
    /* 3. OMP_CLAUSE_SECTIONS: parallel sections */
    int section_sum1 = 0, section_sum2 = 0;
    #pragma omp parallel sections private(i) shared(array, N)
    {
        #pragma omp section
        {
            for (i = 0; i < N/2; i++) {
                section_sum1 += array[i];
            }
        }
        
        #pragma omp section
        {
            for (i = N/2; i < N; i++) {
                section_sum2 += array[i];
            }
        }
    }
    sum += section_sum1 + section_sum2;
    
    /* 4. OMP_CLAUSE_TASKGROUP: taskgroup with tasks */
    int task_results[N];
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (i = 0; i < N; i++) {
                    #pragma omp task firstprivate(i) shared(task_results, array)
                    {
                        /* Each task computes something */
                        task_results[i] = array[i] * array[i];
                    }
                }
            } /* end taskgroup */
        } /* end single */
    } /* end parallel */
    
    /* Use task results to prevent dead code elimination */
    for (i = 0; i < N; i++) {
        sum += task_results[i];
    }
    
    /* Final output to ensure all computations are used */
    printf("Final sum: %d\n", sum);
    return (sum > 0) ? 0 : 1;  /* Return code depends on computation */
}
