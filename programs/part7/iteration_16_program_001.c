#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function to ensure computations aren't optimized away */
static int use_result(int val) {
    volatile int sink = val;
    return sink;
}

int main(int argc, char **argv) {
    /* Use argc for runtime variability to prevent optimization */
    volatile int seed = argc;
    int i, sum = 0;
    const int N = 100;
    int array[N];
    
    /* Initialize array with volatile seed to prevent constant propagation */
    for (i = 0; i < N; i++) {
        array[i] = i + seed;
    }
    
    /* 1. OMP_CLAUSE_PARALLEL: Basic parallel region */
    #pragma omp parallel num_threads(2)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        sum += tid + seed;
    }
    
    /* 2. OMP_CLAUSE_FOR: Parallel for loop */
    #pragma omp parallel for schedule(static) reduction(+:sum)
    for (i = 0; i < N; i++) {
        sum += array[i] % 10;
    }
    
    /* 3. OMP_CLAUSE_SECTIONS: Parallel sections */
    #pragma omp parallel sections private(i) reduction(+:sum)
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
                for (i = 0; i < N; i += 10) {
                    #pragma omp task firstprivate(i) shared(array, sum)
                    {
                        int j;
                        for (j = i; j < i + 10 && j < N; j++) {
                            #pragma omp atomic
                            array[j] += 1;
                        }
                    }
                }
            }
        }
    }
    
    /* Final computation using all modified values */
    int final_sum = 0;
    #pragma omp parallel for reduction(+:final_sum)
    for (i = 0; i < N; i++) {
        final_sum += array[i];
    }
    
    /* Use results to prevent dead code elimination */
    final_sum = use_result(final_sum + sum);
    
    printf("Result: %d (argc=%d)\n", final_sum, argc);
    
    return final_sum > 0 ? 0 : 1;
}
