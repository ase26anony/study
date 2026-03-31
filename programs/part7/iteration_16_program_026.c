/* test_omp_clauses.c
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
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    int i;
    int sum = 0;
    int array[ARRAY_SIZE];
    
    /* Initialize array with seed-dependent values */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] = (i * seed) % 100;
    }
    
    /* 1. OMP_CLAUSE_PARALLEL
     * Use a simple parallel region with shared variable update.
     */
    #pragma omp parallel num_threads(2) shared(sum, array)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        sum += tid + array[tid % ARRAY_SIZE];
    }
    
    /* 2. OMP_CLAUSE_FOR
     * Use parallel for loop with reduction.
     */
    #pragma omp parallel for reduction(+:sum) private(i) schedule(static)
    for (i = 0; i < ARRAY_SIZE; i++) {
        sum += array[i] * (seed % 10);
    }
    
    /* 3. OMP_CLAUSE_SECTIONS
     * Use parallel sections with two distinct section blocks.
     */
    #pragma omp parallel sections shared(array, sum) private(i)
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
    
    /* 4. OMP_CLAUSE_TASKGROUP
     * Use taskgroup inside a parallel region with tasks.
     */
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
    
    /* Print result to prevent dead code elimination */
    printf("Final sum: %d (seed: %d)\n", sum, seed);
    
    return 0;
}
