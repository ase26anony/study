/* test_omp_clauses.c
 * This program is designed to trigger GCC's tree pretty-printer
 * to output the names of four specific OpenMP clauses:
 *   for, parallel, sections, taskgroup
 * Compile with: gcc -O1 -fopenmp -fdump-tree-all test_omp_clauses.c -o test
 * The dump files (e.g., *.original, *.ompexp) should contain pretty-printed
 * representations of these clauses.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char **argv)
{
    /* Use argc to introduce runtime variability and prevent optimization */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    int i, sum = 0;
    const int N = 100;
    int array[N];
    
    /* Initialize array with seed-dependent values */
    for (i = 0; i < N; i++) {
        array[i] = (i + seed) % 97;
    }
    
    /* 1. OMP_CLAUSE_PARALLEL
     * Use a simple parallel region with num_threads clause.
     */
    {
        int shared_counter = 0;
        #pragma omp parallel num_threads(2) shared(shared_counter)
        {
            #pragma omp atomic
            shared_counter++;
        }
        sum += shared_counter;  /* Use result to keep code live */
    }
    
    /* 2. OMP_CLAUSE_FOR
     * Use parallel for with reduction clause.
     */
    {
        int local_sum = 0;
        #pragma omp parallel for reduction(+:local_sum) schedule(static)
        for (i = 0; i < N; i++) {
            local_sum += array[i] * 2;
        }
        sum += local_sum;
    }
    
    /* 3. OMP_CLAUSE_SECTIONS
     * Use parallel sections with two section blocks.
     */
    {
        int section_sum = 0;
        #pragma omp parallel sections reduction(+:section_sum)
        {
            #pragma omp section
            {
                for (i = 0; i < N/2; i++) {
                    section_sum += array[i];
                }
            }
            #pragma omp section
            {
                for (i = N/2; i < N; i++) {
                    section_sum += array[i] * 3;
                }
            }
        }
        sum += section_sum;
    }
    
    /* 4. OMP_CLAUSE_TASKGROUP
     * Use taskgroup inside a parallel region with tasks.
     */
    {
        int task_sum = 0;
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp taskgroup
                {
                    for (i = 0; i < N; i += 10) {
                        #pragma omp task firstprivate(i) shared(task_sum)
                        {
                            int j, chunk_sum = 0;
                            for (j = i; j < i+10 && j < N; j++) {
                                chunk_sum += array[j];
                            }
                            #pragma omp atomic
                            task_sum += chunk_sum;
                        }
                    }
                }
            }
        }
        sum += task_sum;
    }
    
    /* Print the final sum to ensure all computations are used */
    printf("Final sum: %d\n", sum);
    
    return (sum > 0) ? 0 : 1;  /* Return value depends on computation */
}
