#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function to ensure computations aren't optimized away */
static int use_result(int value) {
    volatile int sink = value;
    return sink;
}

int main(int argc, char **argv) {
    /* Use argc for runtime variability to prevent optimization */
    volatile int seed = argc;
    int i, sum = 0;
    const int N = 100;
    int array[N];
    
    /* Initialize array with values based on seed */
    for (i = 0; i < N; i++) {
        array[i] = (i + seed) % 100;
    }
    
    /* 1. OMP_CLAUSE_PARALLEL: Basic parallel region */
    #pragma omp parallel num_threads(2)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        sum += tid + seed;
    }
    
    /* Use the result to keep the region live */
    seed = use_result(sum);
    
    /* 2. OMP_CLAUSE_FOR: Parallel for loop */
    #pragma omp parallel for schedule(static) reduction(+:sum)
    for (i = 0; i < N; i++) {
        array[i] += i * seed;
        sum += array[i];
    }
    
    /* 3. OMP_CLAUSE_SECTIONS: Parallel sections */
    int section_result1 = 0, section_result2 = 0;
    #pragma omp parallel sections reduction(+:sum)
    {
        #pragma omp section
        {
            for (i = 0; i < N/2; i++) {
                section_result1 += array[i];
            }
            sum += section_result1;
        }
        
        #pragma omp section
        {
            for (i = N/2; i < N; i++) {
                section_result2 += array[i] * 2;
            }
            sum += section_result2;
        }
    }
    
    /* 4. OMP_CLAUSE_TASKGROUP: Task group with tasks */
    int task_sum = 0;
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:task_sum)
            {
                for (i = 0; i < N; i += 10) {
                    #pragma omp task in_reduction(+:task_sum) firstprivate(i)
                    {
                        int local_sum = 0;
                        for (int j = i; j < i + 10 && j < N; j++) {
                            local_sum += array[j];
                        }
                        task_sum += local_sum;
                    }
                }
            }
            sum += task_sum;
        }
    }
    
    /* Final output to ensure all computations are used */
    printf("Final sum: %d (seed based on argc=%d)\n", sum, argc);
    
    return sum > 0 ? 0 : 1;
}
