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
    volatile int seed = argc;  /* Prevent optimization */
    int i, sum = 0;
    int array[ARRAY_SIZE];
    
    /* Initialize array with volatile-dependent values */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] = i * (seed + 1);
    }
    
    /* 1. OMP_CLAUSE_PARALLEL: Simple parallel region */
    #pragma omp parallel num_threads(2) shared(sum, array)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        sum += tid;  /* Use sum to prevent dead code elimination */
    }
    
    /* 2. OMP_CLAUSE_FOR: Parallel for loop */
    #pragma omp parallel for schedule(static) shared(array) reduction(+:sum)
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] += seed;  /* Modify array based on volatile seed */
        sum += array[i] % 10;
    }
    
    /* 3. OMP_CLAUSE_SECTIONS: Parallel sections */
    #pragma omp parallel sections shared(array, sum)
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
    #pragma omp parallel shared(array, sum)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (i = 0; i < ARRAY_SIZE; i += 10) {
                    #pragma omp task firstprivate(i) shared(array, sum)
                    {
                        int j;
                        for (j = i; j < i + 10 && j < ARRAY_SIZE; j++) {
                            #pragma omp atomic
                            array[j] += 1;
                        }
                    }
                }
            } /* end taskgroup */
        } /* end single */
    } /* end parallel */
    
    /* Final computation to use all results */
    int final_sum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        final_sum += array[i];
    }
    
    printf("Final sum: %d (seed was: %d)\n", final_sum, seed);
    return final_sum % 100;  /* Return non-zero to ensure all code is live */
}
