/* test_omp_clauses.c
 * 
 * This program is designed to trigger GCC's tree pretty-printer
 * for specific OpenMP clause types: for, parallel, sections, and taskgroup.
 * When compiled with -fdump-tree-all or similar flags, the compiler's
 * internal representation should contain OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP nodes, causing the
 * pretty-printer to output their names.
 *
 * Compile with: gcc -O1 -fopenmp -fdump-tree-all -fdump-tree-omp test_omp_clauses.c -o test_omp_clauses
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 100

int main(int argc, char **argv) {
    volatile int seed = argc;  /* Prevent optimization */
    int i, sum = 0;
    int array[ARRAY_SIZE];
    
    /* Initialize array with volatile seed to prevent dead code elimination */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] = (i + seed) % 100;
    }
    
    /* 1. OMP_CLAUSE_PARALLEL: Basic parallel region */
    #pragma omp parallel num_threads(2)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        sum += tid;  /* Use sum to prevent elimination */
    }
    
    /* 2. OMP_CLAUSE_FOR: Parallel for loop */
    #pragma omp parallel for schedule(static)
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] += (i * seed);  /* Computation using volatile seed */
    }
    
    /* 3. OMP_CLAUSE_SECTIONS: Parallel sections */
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            for (i = 0; i < ARRAY_SIZE/2; i++) {
                array[i] *= 2;
            }
        }
        
        #pragma omp section
        {
            for (i = ARRAY_SIZE/2; i < ARRAY_SIZE; i++) {
                array[i] /= 2;
            }
        }
    }
    
    /* 4. OMP_CLAUSE_TASKGROUP: Taskgroup with tasks */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (i = 0; i < ARRAY_SIZE; i += 10) {
                    #pragma omp task
                    {
                        int idx = i;
                        array[idx] = array[idx] * array[idx] + seed;
                    }
                }
            }
        }
    }
    
    /* Final computation to ensure all OpenMP regions contribute to output */
    int final_sum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        final_sum += array[i];
    }
    
    printf("Final sum: %d (seed was: %d)\n", final_sum, seed);
    return final_sum % 100;  /* Return non-constant to prevent optimization */
}
