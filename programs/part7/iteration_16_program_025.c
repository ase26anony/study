/* test_omp_clauses.c */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function to ensure computations aren't optimized away */
static int use_result(int val) {
    volatile int sink = val;
    return sink;
}

int main(int argc, char **argv) {
    /* Use argc to create input-dependent computation */
    volatile int seed = argc;
    int i, sum = 0;
    const int N = 100;
    int array[N];
    
    /* Initialize array with seed-dependent values */
    for (i = 0; i < N; i++) {
        array[i] = (i * seed) % 97;
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
    for (i = 0; i < N; i++) {
        sum += array[i];
    }
    
    /* 3. OMP_CLAUSE_SECTIONS: Parallel sections */
    int section_sum1 = 0, section_sum2 = 0;
    #pragma omp parallel sections private(i)
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
    
    /* 4. OMP_CLAUSE_TASKGROUP: Taskgroup with tasks */
    int task_results[N];
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (i = 0; i < N; i++) {
                    #pragma omp task firstprivate(i) shared(task_results)
                    {
                        task_results[i] = array[i] * 2;
                    }
                }
            }
        }
    }
    
    /* Use task results to prevent optimization */
    for (i = 0; i < N; i++) {
        sum += task_results[i];
    }
    
    /* Final result depends on all OpenMP computations */
    int final_result = use_result(sum);
    printf("Result: %d (seed: %d)\n", final_result, seed);
    
    return final_result % 2;
}
