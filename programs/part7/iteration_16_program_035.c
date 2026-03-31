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
    /* Use argc to introduce runtime variability and prevent optimization */
    volatile int seed = argc;
    int i;
    int array[ARRAY_SIZE];
    int sum = 0;
    int section_result1 = 0, section_result2 = 0;
    int taskgroup_sum = 0;
    
    /* Initialize array with values based on volatile seed */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] = (i + seed) % 100;
    }
    
    /* 1. OMP_CLAUSE_PARALLEL
     * Use a simple parallel region with num_threads clause.
     * The volatile seed ensures the region isn't optimized away.
     */
    #pragma omp parallel num_threads(2) if(seed > 0)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        sum += tid + seed;
    }
    
    /* 2. OMP_CLAUSE_FOR
     * Use parallel for with schedule clause.
     * The loop computes something based on the array and volatile seed.
     */
    #pragma omp parallel for schedule(static) private(i) reduction(+:sum)
    for (i = 0; i < ARRAY_SIZE; i++) {
        sum += array[i] * (seed + 1);
    }
    
    /* 3. OMP_CLAUSE_SECTIONS
     * Use parallel sections with two distinct section blocks.
     * Each section performs different operations on shared data.
     */
    #pragma omp parallel sections private(i) shared(array, section_result1, section_result2)
    {
        #pragma omp section
        {
            for (i = 0; i < ARRAY_SIZE/2; i++) {
                section_result1 += array[i] * 2;
            }
        }
        
        #pragma omp section
        {
            for (i = ARRAY_SIZE/2; i < ARRAY_SIZE; i++) {
                section_result2 += array[i] * 3;
            }
        }
    }
    sum += section_result1 + section_result2;
    
    /* 4. OMP_CLAUSE_TASKGROUP
     * Use a parallel region with a single construct that starts a taskgroup.
     * Inside the taskgroup, create multiple tasks that modify array elements.
     */
    #pragma omp parallel shared(array, taskgroup_sum)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (i = 0; i < ARRAY_SIZE; i++) {
                    #pragma omp task firstprivate(i) shared(array, taskgroup_sum)
                    {
                        int temp = array[i] + i + seed;
                        #pragma omp atomic
                        taskgroup_sum += temp;
                    }
                }
            }
        }
    }
    sum += taskgroup_sum;
    
    /* Print the final result to ensure all computations are used */
    printf("Final sum: %d (seed based on argc=%d)\n", sum, argc);
    
    return 0;
}
