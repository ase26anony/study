/* test_omp_clauses.c
 * This program uses OpenMP clauses to trigger GCC's tree pretty-printer
 * for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP.
 * Compile with: gcc -O1 -fopenmp -fdump-tree-all -fdump-tree-omp test_omp_clauses.c -o test_omp
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
    #pragma omp parallel num_threads(2)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        sum += tid + seed;
    }
    
    /* 2. OMP_CLAUSE_FOR: Parallel for loop */
    #pragma omp parallel for reduction(+:sum) schedule(static)
    for (i = 0; i < ARRAY_SIZE; i++) {
        sum += array[i] % 10;
    }
    
    /* 3. OMP_CLAUSE_SECTIONS: Parallel sections */
    #pragma omp parallel sections private(i)
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
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (i = 0; i < ARRAY_SIZE; i += 10) {
                    #pragma omp task firstprivate(i)
                    {
                        int j;
                        for (j = i; j < i + 10 && j < ARRAY_SIZE; j++) {
                            #pragma omp atomic
                            array[j] *= 2;
                        }
                    }
                }
            }
        }
    }
    
    /* Final computation to ensure all OpenMP regions contribute to output */
    int final_sum = 0;
    #pragma omp parallel for reduction(+:final_sum)
    for (i = 0; i < ARRAY_SIZE; i++) {
        final_sum += array[i];
    }
    
    printf("Seed: %d, Sum: %d, Array sum: %d\n", seed, sum, final_sum);
    
    return final_sum > 0 ? 0 : 1;  /* Return value depends on computation */
}
