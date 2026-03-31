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
    int i, sum = 0;
    const int N = 100;
    int array[N];
    
    /* Use argc for runtime variability to prevent optimization */
    volatile int seed = argc;
    
    /* Initialize array with values based on seed */
    for (i = 0; i < N; i++) {
        array[i] = (i * seed) % 97;
    }
    
    /* 1. OMP_CLAUSE_PARALLEL: Basic parallel region */
    #pragma omp parallel num_threads(2) shared(array, sum)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        sum += tid + seed;
        
        /* Small computation to keep region non-trivial */
        if (tid == 0) {
            array[0] += 1;
        } else {
            array[1] += 2;
        }
    }
    
    /* 2. OMP_CLAUSE_FOR: Parallel for loop */
    #pragma omp parallel for schedule(static) shared(array) private(i) \
        reduction(+:sum)
    for (i = 0; i < N; i++) {
        array[i] = array[i] * 2 + seed;
        sum += array[i] % 10;
    }
    
    /* 3. OMP_CLAUSE_SECTIONS: Parallel sections */
    #pragma omp parallel sections shared(array, sum)
    {
        #pragma omp section
        {
            for (i = 0; i < N/2; i++) {
                array[i] = array[i] - seed;
                sum += array[i] % 7;
            }
        }
        
        #pragma omp section
        {
            for (i = N/2; i < N; i++) {
                array[i] = array[i] + seed;
                sum += array[i] % 5;
            }
        }
        
        #pragma omp section
        {
            /* Third section for more coverage */
            sum += seed * 3;
        }
    }
    
    /* 4. OMP_CLAUSE_TASKGROUP: Taskgroup with tasks */
    #pragma omp parallel shared(array, sum)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (i = 0; i < 10; i++) {
                    #pragma omp task firstprivate(i) shared(array)
                    {
                        int idx = i * 10;
                        if (idx < N) {
                            array[idx] = array[idx] / (seed + 1);
                            #pragma omp atomic
                            sum += array[idx];
                        }
                    }
                }
            } /* end taskgroup */
        } /* end single */
    } /* end parallel */
    
    /* Use the results to prevent dead code elimination */
    int result = use_result(sum);
    
    /* Print something so compiler can't optimize everything away */
    printf("Final sum: %d (seed based on argc=%d)\n", result, argc);
    
    return result % 100;
}
