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
    volatile int seed = argc; /* Prevent optimization */
    int i;
    int sum = 0;
    int array[ARRAY_SIZE];
    
    /* Initialize array with volatile seed to prevent dead code elimination */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] = i * seed;
    }
    
    /* 1. OMP_CLAUSE_PARALLEL: Basic parallel region */
    #pragma omp parallel num_threads(2) shared(array, sum)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        sum += tid; /* Simple shared variable update */
    }
    
    /* 2. OMP_CLAUSE_FOR: Parallel for loop */
    #pragma omp parallel for shared(array) private(i) reduction(+:sum)
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] += seed; /* Modify array based on volatile seed */
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
                            sum += array[j];
                        }
                    }
                }
            }
        }
    }
    
    /* Use the results to prevent optimization */
    printf("Final sum: %d (seed was: %d)\n", sum, seed);
    
    return sum > 0 ? 0 : 1; /* Return value depends on computation */
}
