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
    
    /* Initialize array with values based on seed */
    for (i = 0; i < N; i++) {
        array[i] = (i * seed) % 100;
    }
    
    /* 1. OMP_CLAUSE_PARALLEL: Basic parallel region */
    #pragma omp parallel num_threads(4) shared(array, sum, seed)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        sum += tid + seed;
    }
    
    /* 2. OMP_CLAUSE_FOR: Parallel for loop */
    #pragma omp parallel for schedule(static) shared(array) reduction(+:sum) \
        private(i) firstprivate(seed)
    for (i = 0; i < N; i++) {
        array[i] += (i % 10) + seed;
        sum += array[i];
    }
    
    /* 3. OMP_CLAUSE_SECTIONS: Parallel sections */
    #pragma omp parallel sections shared(array, sum, seed) private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < N/2; i++) {
                array[i] *= 2;
            }
        }
        
        #pragma omp section
        {
            for (i = N/2; i < N; i++) {
                array[i] += seed;
            }
        }
        
        #pragma omp section
        {
            int local_sum = 0;
            for (i = 0; i < N; i += 4) {
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
                for (i = 0; i < N; i += 10) {
                    #pragma omp task firstprivate(i) shared(array, seed)
                    {
                        int j;
                        for (j = i; j < i + 10 && j < N; j++) {
                            array[j] = (array[j] + seed) % 1000;
                        }
                    }
                }
                
                /* Additional task to ensure taskgroup is non-trivial */
                #pragma omp task shared(sum, array)
                {
                    int temp_sum = 0;
                    for (i = 0; i < N; i++) {
                        temp_sum += array[i];
                    }
                    #pragma omp atomic
                    sum += temp_sum;
                }
            }
        }
    }
    
    /* Use the results to prevent dead code elimination */
    int result = use_result(sum);
    
    /* Print result to ensure all computations are used */
    printf("Final sum: %d (seed based on argc=%d)\n", result, argc);
    
    return result > 0 ? 0 : 1;
}
