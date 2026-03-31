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
    
    /* Initialize array with volatile seed to prevent dead code elimination */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] = i * seed;
    }
    
    /* 1. OMP_CLAUSE_PARALLEL: Basic parallel region */
    #pragma omp parallel num_threads(2) shared(array, sum)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        sum += tid;  /* Use sum to prevent elimination */
    }
    
    /* 2. OMP_CLAUSE_FOR: Parallel for loop */
    #pragma omp parallel for shared(array) private(i) reduction(+:sum)
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] += i * 2;  /* Modify array */
        sum += array[i] % 10;  /* Use sum */
    }
    
    /* 3. OMP_CLAUSE_SECTIONS: Parallel sections */
    #pragma omp parallel sections shared(array, sum)
    {
        #pragma omp section
        {
            for (i = 0; i < ARRAY_SIZE/2; i++) {
                array[i] *= 2;
            }
            #pragma omp atomic
            sum += 1;
        }
        
        #pragma omp section
        {
            for (i = ARRAY_SIZE/2; i < ARRAY_SIZE; i++) {
                array[i] /= 2;
            }
            #pragma omp atomic
            sum += 2;
        }
    }
    
    /* 4. OMP_CLAUSE_TASKGROUP: Taskgroup with tasks */
    #pragma omp parallel shared(array, sum)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (i = 0; i < 5; i++) {
                    #pragma omp task firstprivate(i) shared(array, sum)
                    {
                        int idx = i * 20;
                        for (int j = idx; j < idx + 20 && j < ARRAY_SIZE; j++) {
                            array[j] += j;
                        }
                        #pragma omp atomic
                        sum += i;
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
    return final_sum % 100;  /* Return non-constant to prevent optimization */
}
