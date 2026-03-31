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
        array[i] = (i * seed) % 100;
    }
    
    /* 1. OMP_CLAUSE_PARALLEL: Basic parallel region */
    #pragma omp parallel num_threads(4) shared(array, seed)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        array[tid % N] += tid + seed;
    }
    
    /* 2. OMP_CLAUSE_FOR: Parallel for loop */
    #pragma omp parallel for schedule(static) shared(array, seed) private(i)
    for (i = 0; i < N; i++) {
        array[i] += (i * seed) / (seed + 1);
    }
    
    /* 3. OMP_CLAUSE_SECTIONS: Parallel sections */
    #pragma omp parallel sections shared(array, sum, seed)
    {
        #pragma omp section
        {
            int local_sum = 0;
            for (i = 0; i < N/2; i++) {
                local_sum += array[i];
            }
            #pragma omp atomic
            sum += local_sum * seed;
        }
        
        #pragma omp section
        {
            int local_sum = 0;
            for (i = N/2; i < N; i++) {
                local_sum += array[i];
            }
            #pragma omp atomic
            sum += local_sum / (seed + 1);
        }
    }
    
    /* 4. OMP_CLAUSE_TASKGROUP: Task group with tasks */
    #pragma omp parallel shared(array, sum, seed)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (i = 0; i < 10; i++) {
                    #pragma omp task firstprivate(i) shared(array)
                    {
                        int idx = (i * 7) % N;
                        #pragma omp atomic
                        array[idx] += (i + seed) % 5;
                    }
                }
            }
        }
    }
    
    /* Final computation to use all results */
    int final_sum = 0;
    for (i = 0; i < N; i++) {
        final_sum += array[i];
    }
    
    /* Use volatile and printf to prevent dead code elimination */
    volatile int result = use_result(final_sum + sum);
    printf("Result: %d (seed: %d)\n", result, seed);
    
    return result % 100;
}
