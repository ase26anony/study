/* test_omp_clauses.c
 * This program is designed to trigger GCC's tree pretty-printer
 * for OpenMP clauses: FOR, PARALLEL, SECTIONS, and TASKGROUP.
 * Compile with: gcc -O1 -fopenmp -fdump-tree-all -fdump-tree-omp test_omp_clauses.c -o test_omp_executable
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
    
    /* Initialize array with seed-dependent values */
    for (i = 0; i < N; i++) {
        array[i] = (i * seed) % 97;
    }
    
    /* 1. OMP_CLAUSE_PARALLEL: Basic parallel region */
    #pragma omp parallel num_threads(2)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        sum += tid;  /* Use sum to prevent dead code elimination */
    }
    
    /* 2. OMP_CLAUSE_FOR: Parallel for loop */
    #pragma omp parallel for reduction(+:sum) schedule(static)
    for (i = 0; i < N; i++) {
        sum += array[i] % 10;  /* Perform some computation */
    }
    
    /* 3. OMP_CLAUSE_SECTIONS: Parallel sections */
    int section_sum1 = 0, section_sum2 = 0;
    #pragma omp parallel sections reduction(+:sum)
    {
        #pragma omp section
        {
            for (i = 0; i < N/2; i++) {
                section_sum1 += array[i];
            }
            sum += section_sum1;
        }
        
        #pragma omp section
        {
            for (i = N/2; i < N; i++) {
                section_sum2 += array[i] * 2;
            }
            sum += section_sum2;
        }
    }
    
    /* 4. OMP_CLAUSE_TASKGROUP: Taskgroup with tasks */
    int task_results[N];
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (i = 0; i < N; i++) {
                    #pragma omp task firstprivate(i) shared(task_results)
                    {
                        task_results[i] = array[i] * 3;
                    }
                }
            } /* end taskgroup */
        } /* end single */
    } /* end parallel */
    
    /* Use task_results to prevent optimization */
    for (i = 0; i < N; i++) {
        sum += task_results[i] % 5;
    }
    
    /* Print result to ensure all code is live */
    printf("Final sum: %d (seed: %d)\n", sum, seed);
    
    return sum % 256;  /* Return value depends on all computations */
}
