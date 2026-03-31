/* test_omp_clauses.c
 * Targets uncovered lines in tree-pretty-print.cc:
 * OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP
 * Compile with: gcc -O1 -fopenmp -fdump-tree-all -fdump-tree-omp test_omp_clauses.c -o test_omp
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper to prevent optimization */
static volatile int global_seed = 42;

int main(int argc, char **argv) {
    int i, sum = 0;
    int array[100];
    
    /* Use argc for runtime variability to prevent dead code elimination */
    volatile int seed = argc > 1 ? atoi(argv[1]) : 12345;
    
    /* 1. OMP_CLAUSE_PARALLEL - Simple parallel region */
    #pragma omp parallel num_threads(2)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        sum += tid + seed;
    }
    
    /* Initialize array with seed-dependent values */
    for (i = 0; i < 100; i++) {
        array[i] = (i * seed) % 100;
    }
    
    /* 2. OMP_CLAUSE_FOR - Parallel for loop */
    #pragma omp parallel for reduction(+:sum) schedule(static)
    for (i = 0; i < 100; i++) {
        sum += array[i] * (i % 10);
    }
    
    /* 3. OMP_CLAUSE_SECTIONS - Parallel sections */
    #pragma omp parallel sections private(i) reduction(+:sum)
    {
        #pragma omp section
        {
            for (i = 0; i < 50; i++) {
                sum += array[i] * 2;
            }
        }
        
        #pragma omp section
        {
            for (i = 50; i < 100; i++) {
                sum += array[i] * 3;
            }
        }
        
        #pragma omp section
        {
            /* Extra section for more coverage */
            sum += seed * 4;
        }
    }
    
    /* 4. OMP_CLAUSE_TASKGROUP - Taskgroup with tasks */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (i = 0; i < 20; i++) {
                    #pragma omp task firstprivate(i) shared(sum, array)
                    {
                        int temp = array[i] * (i + 1);
                        #pragma omp atomic
                        sum += temp;
                    }
                }
            }
        }
    }
    
    /* Combined construct using multiple clauses */
    #pragma omp parallel for simd reduction(+:sum) if(seed > 0)
    for (i = 0; i < 100; i++) {
        array[i] = (array[i] + sum) % 1000;
    }
    
    /* Nested parallel region with for clause */
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (i = 0; i < 50; i++) {
            array[i] += global_seed;
        }
        
        #pragma omp single
        {
            /* Another taskgroup inside parallel region */
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    sum += array[0];
                }
                #pragma omp task
                {
                    sum += array[99];
                }
            }
        }
    }
    
    /* Use result to prevent optimization */
    printf("Final sum: %d (seed: %d)\n", sum, seed);
    
    return sum > 0 ? 0 : 1;
}
