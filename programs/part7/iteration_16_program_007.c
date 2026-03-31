/* test_omp_clauses.c
 * Targets uncovered lines in tree-pretty-print.cc:
 * OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP
 * Compile with: gcc -O1 -fopenmp -fdump-tree-all -fdump-tree-omp test_omp_clauses.c -o test_omp
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 100

/* Helper to prevent optimization */
static volatile int global_seed = 42;

int main(int argc, char **argv) {
    int i, sum = 0;
    int array[ARRAY_SIZE];
    
    /* Use argc for runtime variability to prevent dead code elimination */
    volatile int seed = argc > 1 ? atoi(argv[1]) : 12345;
    
    /* 1. OMP_CLAUSE_PARALLEL - Simple parallel region */
    #pragma omp parallel num_threads(2) shared(global_seed)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        global_seed += tid;
    }
    
    /* Initialize array with seed-dependent values */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] = (i * seed) % 100;
    }
    
    /* 2. OMP_CLAUSE_FOR - Parallel for loop */
    #pragma omp parallel for reduction(+:sum) schedule(static)
    for (i = 0; i < ARRAY_SIZE; i++) {
        sum += array[i];
        array[i] = (array[i] + global_seed) % 1000;
    }
    
    /* 3. OMP_CLAUSE_SECTIONS - Parallel sections */
    int section_result1 = 0, section_result2 = 0;
    #pragma omp parallel sections shared(array, section_result1, section_result2)
    {
        #pragma omp section
        {
            for (i = 0; i < ARRAY_SIZE/2; i++) {
                section_result1 += array[i];
            }
        }
        
        #pragma omp section
        {
            for (i = ARRAY_SIZE/2; i < ARRAY_SIZE; i++) {
                section_result2 += array[i];
            }
        }
    }
    sum += section_result1 + section_result2;
    
    /* 4. OMP_CLAUSE_TASKGROUP - Taskgroup with tasks */
    int task_sum = 0;
    #pragma omp parallel shared(task_sum, array)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (i = 0; i < ARRAY_SIZE; i += 10) {
                    #pragma omp task firstprivate(i) shared(task_sum, array)
                    {
                        int j, local_sum = 0;
                        for (j = i; j < i + 10 && j < ARRAY_SIZE; j++) {
                            local_sum += array[j];
                        }
                        #pragma omp atomic
                        task_sum += local_sum;
                    }
                }
            }
        }
    }
    sum += task_sum;
    
    /* Use the result to prevent optimization */
    printf("Final sum: %d (seed: %d, global_seed: %d)\n", 
           sum, seed, global_seed);
    
    return sum > 0 ? 0 : 1;
}
