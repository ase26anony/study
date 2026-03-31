/* test_openmp_clauses.c
 * This program is designed to trigger GCC's tree pretty-printer
 * for specific OpenMP clause types: for, parallel, sections, and taskgroup.
 * Compile with: gcc -O1 -fopenmp -fdump-tree-all -fdump-tree-omp test_openmp_clauses.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char **argv) {
    /* Use argc to create runtime-dependent computation */
    volatile int seed = argc;
    int i, sum = 0;
    const int N = 100;
    int array[N];
    
    /* Initialize array with values based on seed */
    for (i = 0; i < N; i++) {
        array[i] = (i + seed) % 37;
    }
    
    /* 1. OMP_CLAUSE_PARALLEL: Use a simple parallel region */
    #pragma omp parallel num_threads(2) shared(sum)
    {
        #pragma omp atomic
        sum += 1;  /* Count threads that enter */
    }
    
    /* 2. OMP_CLAUSE_FOR: Use parallel for loop */
    #pragma omp parallel for shared(array) private(i) reduction(+:sum)
    for (i = 0; i < N; i++) {
        array[i] += seed;  /* Modify based on seed */
        sum += array[i];   /* Accumulate sum */
    }
    
    /* 3. OMP_CLAUSE_SECTIONS: Use parallel sections */
    int section_sum1 = 0, section_sum2 = 0;
    #pragma omp parallel sections shared(array, section_sum1, section_sum2)
    {
        #pragma omp section
        {
            for (i = 0; i < N/2; i++) {
                section_sum1 += array[i];
            }
        }
        
        #pragma omp section
        {
            for (i = N/2; i < N; i++) {
                section_sum2 += array[i];
            }
        }
    }
    sum += section_sum1 + section_sum2;
    
    /* 4. OMP_CLAUSE_TASKGROUP: Use taskgroup with tasks */
    int task_sum = 0;
    #pragma omp parallel shared(array, task_sum)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (i = 0; i < N; i += 10) {
                    #pragma omp task firstprivate(i) shared(array, task_sum)
                    {
                        int j, local_sum = 0;
                        int end = (i + 10 < N) ? i + 10 : N;
                        for (j = i; j < end; j++) {
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
    
    /* Print result to prevent dead code elimination */
    printf("Final sum: %d (seed was: %d)\n", sum, seed);
    
    return 0;
}
