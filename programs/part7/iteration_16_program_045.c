/* test_openmp_clauses.c
 * This program is designed to trigger GCC's tree pretty-printer
 * for specific OpenMP clause types: for, parallel, sections, and taskgroup.
 * Compile with: gcc -O1 -fopenmp -fdump-tree-all -fdump-tree-omp test_openmp_clauses.c -o test_omp
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
    
    /* 1. OMP_CLAUSE_PARALLEL: Simple parallel region with shared variable update */
    #pragma omp parallel num_threads(2)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        sum += tid;
    }
    
    /* 2. OMP_CLAUSE_FOR: Parallel for loop with reduction */
    #pragma omp parallel for reduction(+:sum) schedule(static)
    for (i = 0; i < N; i++) {
        sum += arr[i] % 10;
    }
    
    /* 3. OMP_CLAUSE_SECTIONS: Parallel sections with different operations */
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < N/2; i++) {
                #pragma omp atomic
                arr[i] += 1;
            }
        }
        
        #pragma omp section
        {
            for (i = N/2; i < N; i++) {
                #pragma omp atomic
                arr[i] -= 1;
            }
        }
    }
    
    /* 4. OMP_CLAUSE_TASKGROUP: Taskgroup containing multiple tasks */
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
                        for (j = i; j < i + 10 && j < N; j++) {
                            #pragma omp atomic
                            sum += arr[j];
                        }
                    }
                }
            }
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final sum: %d\n", sum);
    
    return (sum > 0) ? 0 : 1;
}
