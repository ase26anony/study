/* test_omp_clauses.c
 * 
 * This program is designed to trigger GCC's tree pretty-printer
 * for the specific OpenMP clause types: for, parallel, sections, and taskgroup.
 * When compiled with -fdump-tree-all or similar flags, the compiler's
 * internal representation should contain OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP nodes, causing the
 * uncovered lines in tree-pretty-print.cc to be executed during dumping.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 100

int main(int argc, char **argv) {
    /* Use argc to create input-dependent computation, preventing optimization */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    int i;
    int sum = 0;
    int array[ARRAY_SIZE];
    
    /* Initialize array with seed-dependent values */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] = (i + seed) % 100;
    }
    
    /* 1. OMP_CLAUSE_PARALLEL: Simple parallel region */
    #pragma omp parallel num_threads(2)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        sum += tid;  /* Use atomic to avoid data race warnings */
    }
    
    /* 2. OMP_CLAUSE_FOR: parallel for loop */
    #pragma omp parallel for reduction(+:sum)
    for (i = 0; i < ARRAY_SIZE; i++) {
        sum += array[i] * 2;
    }
    
    /* 3. OMP_CLAUSE_SECTIONS: parallel sections */
    #pragma omp parallel sections reduction(+:sum)
    {
        #pragma omp section
        {
            for (i = 0; i < ARRAY_SIZE/2; i++) {
                sum += array[i] * 3;
            }
        }
        
        #pragma omp section
        {
            for (i = ARRAY_SIZE/2; i < ARRAY_SIZE; i++) {
                sum += array[i] * 5;
            }
        }
    }
    
    /* 4. OMP_CLAUSE_TASKGROUP: taskgroup with tasks */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (i = 0; i < ARRAY_SIZE; i += 10) {
                    #pragma omp task firstprivate(i) shared(array, sum)
                    {
                        int j;
                        int local_sum = 0;
                        for (j = i; j < i + 10 && j < ARRAY_SIZE; j++) {
                            local_sum += array[j];
                        }
                        #pragma omp atomic
                        sum += local_sum;
                    }
                }
            }
        }
    }
    
    /* Print result to ensure all computations are used */
    printf("Final sum: %d (seed: %d)\n", sum, seed);
    
    return 0;
}
