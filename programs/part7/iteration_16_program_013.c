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
    /* Use argc to create runtime-dependent values */
    volatile int seed = argc;
    int i, sum = 0;
    int array[ARRAY_SIZE];
    
    /* Initialize array with values based on seed */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] = i + seed;
    }
    
    /* 1. OMP_CLAUSE_PARALLEL: Basic parallel region */
    #pragma omp parallel num_threads(2) shared(sum)
    {
        #pragma omp atomic
        sum += 1;  /* Each thread increments sum */
    }
    
    /* 2. OMP_CLAUSE_FOR: Parallel for loop */
    #pragma omp parallel for reduction(+:sum) schedule(static)
    for (i = 0; i < ARRAY_SIZE; i++) {
        sum += array[i] % 10;  /* Some computation */
    }
    
    /* 3. OMP_CLAUSE_SECTIONS: Parallel sections */
    #pragma omp parallel sections shared(array, sum)
    {
        #pragma omp section
        {
            for (i = 0; i < ARRAY_SIZE/2; i++) {
                #pragma omp atomic
                array[i] += 1;
            }
        }
        
        #pragma omp section
        {
            for (i = ARRAY_SIZE/2; i < ARRAY_SIZE; i++) {
                #pragma omp atomic
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
                    #pragma omp task firstprivate(i) shared(array, sum)
                    {
                        int j;
                        for (j = i; j < i + 10 && j < ARRAY_SIZE; j++) {
                            #pragma omp atomic
                            array[j] *= 2;
                        }
                        #pragma omp atomic
                        sum += 1;
                    }
                }
            } /* end taskgroup */
        } /* end single */
    } /* end parallel */
    
    /* Final computation to ensure all results are used */
    int final_sum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        final_sum += array[i];
    }
    
    printf("Final sum: %d (seed was: %d)\n", final_sum, seed);
    return final_sum > 0 ? 0 : 1;  /* Return value depends on computation */
}
