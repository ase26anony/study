/* test_omp_clauses.c */
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
        array[i] = i + seed;
    }
    
    /* 1. OMP_CLAUSE_PARALLEL: Simple parallel region */
    #pragma omp parallel num_threads(2)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        sum += tid + seed;
    }
    
    /* 2. OMP_CLAUSE_FOR: Parallel for loop */
    #pragma omp parallel for reduction(+:sum) schedule(static)
    for (i = 0; i < N; i++) {
        sum += array[i] % 10;
    }
    
    /* 3. OMP_CLAUSE_SECTIONS: Parallel sections */
    #pragma omp parallel sections reduction(+:sum)
    {
        #pragma omp section
        {
            for (i = 0; i < N/2; i++) {
                sum += array[i] * 2;
            }
        }
        
        #pragma omp section
        {
            for (i = N/2; i < N; i++) {
                sum += array[i] / 2;
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
                for (i = 0; i < 10; i++) {
                    #pragma omp task firstprivate(i) shared(array, sum)
                    {
                        int idx = i * 10;
                        int local_sum = 0;
                        for (int j = idx; j < idx + 10 && j < N; j++) {
                            local_sum += array[j];
                        }
                        #pragma omp atomic
                        sum += local_sum;
                    }
                }
            }
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final sum: %d (seed was: %d)\n", sum, seed);
    
    return sum > 0 ? 0 : 1;
}
