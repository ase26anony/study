/* test_openmp_clauses.c
 * 
 * This program is designed to trigger GCC's tree pretty-printer
 * for specific OpenMP clause types: for, parallel, sections, and taskgroup.
 * When compiled with -fdump-tree-all or similar flags, the compiler's
 * internal representation should contain OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP nodes, causing the
 * uncovered lines in tree-pretty-print.cc to execute during dumping.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 100

int main(int argc, char **argv) {
    /* Use argc to create input-dependent computation */
    volatile int seed = argc;
    int i;
    int array[ARRAY_SIZE];
    int sum = 0;
    
    /* Initialize array with values based on seed */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] = i * (seed + 1);
    }
    
    /* 1. OMP_CLAUSE_PARALLEL: Basic parallel region */
    printf("Starting parallel region...\n");
    #pragma omp parallel num_threads(2)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        sum += tid;
    }
    
    /* 2. OMP_CLAUSE_FOR: Parallel for loop */
    printf("Starting parallel for loop...\n");
    #pragma omp parallel for schedule(static)
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] += seed * 2;
    }
    
    /* 3. OMP_CLAUSE_SECTIONS: Parallel sections */
    printf("Starting parallel sections...\n");
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            for (i = 0; i < ARRAY_SIZE/2; i++) {
                array[i] *= 3;
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
    printf("Starting taskgroup...\n");
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (i = 0; i < ARRAY_SIZE; i += 10) {
                    #pragma omp task
                    {
                        int j;
                        for (j = i; j < i + 10 && j < ARRAY_SIZE; j++) {
                            array[j] += j;
                        }
                    }
                }
            }
        }
    }
    
    /* Final computation to ensure all OpenMP regions are used */
    for (i = 0; i < ARRAY_SIZE; i++) {
        sum += array[i];
    }
    
    printf("Final sum: %d\n", sum);
    return sum % 100;
}
