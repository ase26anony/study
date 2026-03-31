#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function to ensure computations aren't optimized away */
static int use_result(int value) {
    volatile int sink = value;
    return sink;
}

int main(int argc, char **argv) {
    int i, sum = 0;
    volatile int seed = argc; /* Prevent constant propagation */
    
    /* 1. OMP_CLAUSE_PARALLEL: Basic parallel region */
    #pragma omp parallel num_threads(2)
    {
        int thread_id = omp_get_thread_num();
        #pragma omp atomic
        sum += thread_id + seed;
    }
    
    /* Use the result to prevent dead code elimination */
    sum = use_result(sum);
    
    /* 2. OMP_CLAUSE_FOR: Parallel for loop */
    int array[100];
    #pragma omp parallel for schedule(static)
    for (i = 0; i < 100; i++) {
        array[i] = (i * seed) % 100;
    }
    
    /* 3. OMP_CLAUSE_SECTIONS: Parallel sections */
    int section_result[2] = {0, 0};
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            for (i = 0; i < 50; i++) {
                section_result[0] += array[i];
            }
        }
        
        #pragma omp section
        {
            for (i = 50; i < 100; i++) {
                section_result[1] += array[i];
            }
        }
    }
    
    sum += section_result[0] + section_result[1];
    sum = use_result(sum);
    
    /* 4. OMP_CLAUSE_TASKGROUP: Taskgroup with tasks */
    int task_array[50];
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (i = 0; i < 50; i++) {
                    #pragma omp task firstprivate(i) shared(task_array)
                    {
                        task_array[i] = (i + seed) * 2;
                    }
                }
            }
        }
    }
    
    /* Final computation using all results */
    int final_sum = 0;
    for (i = 0; i < 50; i++) {
        final_sum += task_array[i];
    }
    final_sum += sum;
    
    printf("Result: %d (seed was: %d)\n", final_sum, seed);
    
    return final_sum > 0 ? 0 : 1;
}
