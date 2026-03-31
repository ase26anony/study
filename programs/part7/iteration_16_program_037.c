/* test_omp_clauses.c
 * This program is designed to trigger GCC's tree pretty-printer
 * for specific OpenMP clause types: for, parallel, sections, and taskgroup.
 * Compile with: gcc -O1 -fopenmp -fdump-tree-all -fdump-tree-omp test_omp_clauses.c -o test_omp_clauses
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 100

int main(int argc, char **argv) {
    /* Use argc to create input-dependent computation */
    volatile int seed = argc;
    int i, sum = 0;
    int array[ARRAY_SIZE];
    
    /* Initialize array with values based on seed */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] = (i + seed) % 100;
    }
    
    /* 1. OMP_CLAUSE_PARALLEL: Basic parallel region */
    #pragma omp parallel num_threads(2) shared(sum, array)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        sum += tid + 1;  /* Ensure work in parallel region */
    }
    
    /* 2. OMP_CLAUSE_FOR: Parallel for loop */
    #pragma omp parallel for reduction(+:sum) schedule(static)
    for (i = 0; i < ARRAY_SIZE; i++) {
        sum += array[i] * (seed + 1);
    }
    
    /* 3. OMP_CLAUSE_SECTIONS: Parallel sections */
    #pragma omp parallel sections shared(array, sum)
    {
        #pragma omp section
        {
            for (i = 0; i < ARRAY_SIZE/2; i++) {
                array[i] += 1;
            }
        }
        
        #pragma omp section
        {
            for (i = ARRAY_SIZE/2; i < ARRAY_SIZE; i++) {
                array[i] -= 1;
            }
        }
    }
    
    /* 4. OMP_CLAUSE_TASKGROUP: Taskgroup with tasks */
    #pragma omp parallel shared(array, sum)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (i = 0; i < ARRAY_SIZE; i += 10) {
                    #pragma omp task firstprivate(i) shared(array)
                    {
                        /* Each task processes a chunk of the array */
                        for (int j = i; j < i + 10 && j < ARRAY_SIZE; j++) {
                            array[j] *= 2;
                        }
                    }
                }
            } /* end taskgroup */
        } /* end single */
    } /* end parallel */
    
    /* Final computation to ensure all results are used */
    int final_sum = 0;
    #pragma omp parallel for reduction(+:final_sum)
    for (i = 0; i < ARRAY_SIZE; i++) {
        final_sum += array[i];
    }
    
    /* Print results to prevent dead code elimination */
    printf("Seed: %d, Intermediate sum: %d, Final array sum: %d\n", 
           seed, sum, final_sum);
    
    return (final_sum > 0) ? 0 : 1;
}
