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
    #pragma omp parallel num_threads(2) shared(sum, seed)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        sum += tid + seed;
    }
    
    /* 2. OMP_CLAUSE_FOR: Parallel for loop */
    #pragma omp parallel for shared(array, seed) private(i) reduction(+:sum)
    for (i = 0; i < N; i++) {
        array[i] += seed + i;
        sum += array[i] % 10;
    }
    
    /* 3. OMP_CLAUSE_SECTIONS: Parallel sections */
    #pragma omp parallel sections shared(array, sum, seed)
    {
        #pragma omp section
        {
            for (i = 0; i < N/2; i++) {
                array[i] = array[i] * 2 + seed;
            }
        }
        
        #pragma omp section
        {
            for (i = N/2; i < N; i++) {
                array[i] = array[i] / 2 + seed;
            }
        }
        
        #pragma omp section
        {
            int local_sum = 0;
            for (i = 0; i < N; i += 3) {
                local_sum += array[i];
            }
            #pragma omp atomic
            sum += local_sum;
        }
    }
    
    /* 4. OMP_CLAUSE_TASKGROUP: Taskgroup with tasks */
    #pragma omp parallel shared(array, sum, seed)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (i = 0; i < N; i += 5) {
                    #pragma omp task firstprivate(i) shared(array, seed)
                    {
                        array[i] = array[i] + seed + omp_get_thread_num();
                    }
                }
            }
            
            /* Additional task to ensure taskgroup is meaningful */
            #pragma omp task shared(array, sum)
            {
                int task_sum = 0;
                for (i = 0; i < N; i++) {
                    task_sum += array[i] % 7;
                }
                #pragma omp atomic
                sum += task_sum;
            }
        }
    }
    
    /* Use the results to prevent dead code elimination */
    int result = use_result(sum);
    
    /* Print result to ensure all computations are live */
    printf("Final sum: %d (seed: %d)\n", result, seed);
    
    return result % 256;
}
