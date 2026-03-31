/* test_omp_clauses.c
 * This program is designed to trigger GCC's tree pretty-printer
 * to output the names of four specific OpenMP clauses:
 *   for, parallel, sections, taskgroup
 * Compile with: gcc -O1 -fopenmp -fdump-tree-all test_omp_clauses.c -o test
 * This will generate dump files containing pretty-printed OpenMP clauses.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char **argv) {
    /* Use argc to create input-dependent computation */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    int i, sum = 0;
    const int N = 100;
    int array[N];
    
    /* Initialize array with seed-dependent values */
    for (i = 0; i < N; i++) {
        array[i] = (i * seed) % 100;
    }
    
    /* 1. OMP_CLAUSE_PARALLEL: Simple parallel region */
    #pragma omp parallel num_threads(2)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        sum += tid + seed;
    }
    
    /* 2. OMP_CLAUSE_FOR: Parallel for loop */
    #pragma omp parallel for reduction(+:sum)
    for (i = 0; i < N; i++) {
        sum += array[i];
    }
    
    /* 3. OMP_CLAUSE_SECTIONS: Parallel sections */
    int section_result1 = 0, section_result2 = 0;
    #pragma omp parallel sections reduction(+:sum)
    {
        #pragma omp section
        {
            for (i = 0; i < N/2; i++) {
                section_result1 += array[i];
            }
            sum += section_result1;
        }
        
        #pragma omp section
        {
            for (i = N/2; i < N; i++) {
                section_result2 += array[i];
            }
            sum += section_result2;
        }
    }
    
    /* 4. OMP_CLAUSE_TASKGROUP: Taskgroup with tasks */
    int task_sum = 0;
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (i = 0; i < 10; i++) {
                    #pragma omp task shared(task_sum)
                    {
                        #pragma omp atomic
                        task_sum += array[i % N];
                    }
                }
            }
        }
    }
    sum += task_sum;
    
    /* Final output to prevent dead code elimination */
    printf("Final sum: %d (seed: %d)\n", sum, seed);
    
    return (sum > 0) ? 0 : 1;
}
