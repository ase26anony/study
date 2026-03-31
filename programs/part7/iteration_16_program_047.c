/* test_openmp_clauses.c
 * 
 * This program is designed to trigger GCC's tree pretty-printer
 * for specific OpenMP clause types: for, parallel, sections, and taskgroup.
 * When compiled with -fdump-tree-all or similar flags, the compiler's
 * internal representation will contain OMP_CLAUSE_* nodes that should
 * be printed by the uncovered code in tree-pretty-print.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char **argv) {
    /* Use argc to create runtime-dependent values that prevent
     * the compiler from optimizing away the OpenMP constructs.
     */
    volatile int seed = argc;
    int i, sum = 0;
    const int N = 100;
    int array[N];
    
    /* Initialize array with values based on seed */
    for (i = 0; i < N; i++) {
        array[i] = (i + seed) % 97;
    }
    
    /* 1. OMP_CLAUSE_PARALLEL
     * Use a simple parallel region with explicit num_threads clause
     * and a shared variable update.
     */
    int shared_counter = 0;
    #pragma omp parallel num_threads(2) shared(shared_counter)
    {
        #pragma omp atomic
        shared_counter++;
    }
    printf("Parallel region executed, shared_counter = %d\n", shared_counter);
    
    /* 2. OMP_CLAUSE_FOR
     * Use parallel for with reduction to ensure the loop is not optimized out.
     * The schedule(static) clause ensures the for clause is represented.
     */
    int loop_sum = 0;
    #pragma omp parallel for reduction(+:loop_sum) schedule(static)
    for (i = 0; i < N; i++) {
        loop_sum += array[i];
    }
    printf("Parallel for loop sum = %d\n", loop_sum);
    sum += loop_sum;
    
    /* 3. OMP_CLAUSE_SECTIONS
     * Use parallel sections with two section blocks performing
     * different operations on the array.
     */
    int section_sum1 = 0, section_sum2 = 0;
    #pragma omp parallel sections reduction(+:section_sum1, section_sum2)
    {
        #pragma omp section
        {
            for (i = 0; i < N/2; i++) {
                section_sum1 += array[i] * 2;
            }
        }
        
        #pragma omp section
        {
            for (i = N/2; i < N; i++) {
                section_sum2 += array[i] / 2;
            }
        }
    }
    printf("Sections region sums: %d + %d = %d\n", 
           section_sum1, section_sum2, section_sum1 + section_sum2);
    sum += section_sum1 + section_sum2;
    
    /* 4. OMP_CLAUSE_TASKGROUP
     * Use a taskgroup inside a parallel region to create taskgroup clause.
     * The tasks modify different portions of the array.
     */
    int taskgroup_sum = 0;
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (i = 0; i < N; i += 10) {
                    #pragma omp task firstprivate(i) shared(array, taskgroup_sum)
                    {
                        int j, local_sum = 0;
                        for (j = i; j < i + 10 && j < N; j++) {
                            local_sum += array[j] * 3;
                        }
                        #pragma omp atomic
                        taskgroup_sum += local_sum;
                    }
                }
            }
        }
    }
    printf("Taskgroup region sum = %d\n", taskgroup_sum);
    sum += taskgroup_sum;
    
    /* Final output that depends on all computations */
    printf("Total cumulative sum = %d\n", sum);
    
    /* Return value depends on the computation, preventing dead code elimination */
    return (sum > 0) ? 0 : 1;
}
