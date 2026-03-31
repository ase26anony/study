/* test_openmp_clauses.c
 * 
 * This program is designed to trigger GCC's tree pretty-printer
 * for specific OpenMP clause types: for, parallel, sections, and taskgroup.
 * When compiled with -fdump-tree-all or similar flags, the compiler's
 * internal representation should contain OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP nodes, causing the
 * pretty-printer to output their names.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 100

int main(int argc, char **argv) {
    /* Use argc to create input-dependent computation */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    int i;
    int sum = 0;
    int array[ARRAY_SIZE];
    
    /* Initialize array with seed-dependent values */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] = (i * seed) % 100;
    }
    
    /* 1. OMP_CLAUSE_PARALLEL: Use a simple parallel region */
    printf("Starting parallel region...\n");
    #pragma omp parallel num_threads(2)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        sum += tid + seed;
    }
    
    /* 2. OMP_CLAUSE_FOR: Use parallel for loop */
    printf("Starting parallel for loop...\n");
    #pragma omp parallel for reduction(+:sum) schedule(static)
    for (i = 0; i < ARRAY_SIZE; i++) {
        sum += array[i];
    }
    
    /* 3. OMP_CLAUSE_SECTIONS: Use parallel sections */
    printf("Starting parallel sections...\n");
    #pragma omp parallel sections reduction(+:sum)
    {
        #pragma omp section
        {
            for (i = 0; i < ARRAY_SIZE/2; i++) {
                sum += array[i] * 2;
            }
        }
        
        #pragma omp section
        {
            for (i = ARRAY_SIZE/2; i < ARRAY_SIZE; i++) {
                sum += array[i] / 2;
            }
        }
    }
    
    /* 4. OMP_CLAUSE_TASKGROUP: Use taskgroup with tasks */
    printf("Starting taskgroup...\n");
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
                            array[j] += 1;
                        }
                    }
                }
            }
        }
    }
    
    /* Final computation using modified array to prevent optimization */
    int final_sum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        final_sum += array[i];
    }
    
    printf("Final results: sum = %d, array_sum = %d\n", sum, final_sum);
    return (sum + final_sum) % 100;
}
