/* test_omp_clauses.c
 * This program is designed to trigger GCC's tree pretty-printer
 * for OpenMP clauses: FOR, PARALLEL, SECTIONS, and TASKGROUP.
 * Compile with: gcc -O1 -fopenmp -fdump-tree-all -fdump-tree-omp test_omp_clauses.c -o test_omp_executable
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char **argv) {
    /* Use argc to introduce runtime variability and prevent optimization */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    int i, sum = 0;
    const int N = 100;
    int arr[N];
    
    /* Initialize array with volatile seed to prevent compile-time computation */
    for (i = 0; i < N; i++) {
        arr[i] = i + seed;
    }
    
    /* 1. OMP_CLAUSE_PARALLEL: Simple parallel region */
    #pragma omp parallel num_threads(2)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        sum += tid;  /* Use sum to create observable side effect */
    }
    
    /* 2. OMP_CLAUSE_FOR: Parallel for loop */
    #pragma omp parallel for reduction(+:sum) schedule(static)
    for (i = 0; i < N; i++) {
        sum += arr[i] % 10;  /* Non-trivial computation using volatile data */
    }
    
    /* 3. OMP_CLAUSE_SECTIONS: Parallel sections */
    #pragma omp parallel sections reduction(+:sum)
    {
        #pragma omp section
        {
            for (i = 0; i < N/2; i++) {
                sum += arr[i] * 2;
            }
        }
        
        #pragma omp section
        {
            for (i = N/2; i < N; i++) {
                sum -= arr[i] / 3;
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
                    #pragma omp task firstprivate(i) shared(arr, sum)
                    {
                        int j;
                        int local_sum = 0;
                        for (j = i; j < i + 10 && j < N; j++) {
                            local_sum += arr[j];
                        }
                        #pragma omp atomic
                        sum += local_sum;
                    }
                }
            } /* end taskgroup */
        } /* end single */
    } /* end parallel */
    
    /* Print result to ensure all computations are used */
    printf("Final sum: %d\n", sum);
    
    return (sum > 0) ? 0 : 1;  /* Return value depends on computation */
}
