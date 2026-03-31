/* test_taskgroup.c - Program to trigger OMP_CLAUSE_TASKGROUP pretty-printing */
#include <stdio.h>
#include <omp.h>

int main() {
    volatile int N = 100;
    int sum = 0;
    int a = 0, b = 0, c = 0;
    
    /* Parallel region to spawn threads */
    #pragma omp parallel shared(sum, a, b, c) firstprivate(N)
    {
        int thread_id = omp_get_thread_num();
        
        /* OMP_CLAUSE_FOR case */
        #pragma omp for schedule(dynamic) nowait
        for (int i = 0; i < N; i++) {
            #pragma omp atomic
            sum += i % 10;
        }
        
        /* OMP_CLAUSE_SECTIONS case */
        #pragma omp sections
        {
            #pragma omp section
            {
                a = thread_id * 2;
                printf("Section 1: thread %d, a = %d\n", thread_id, a);
            }
            
            #pragma omp section
            {
                b = thread_id * 3;
                printf("Section 2: thread %d, b = %d\n", thread_id, b);
            }
        }
        
        /* OMP_CLAUSE_TASKGROUP case - TARGET FOR COVERAGE */
        #pragma omp taskgroup
        {
            /* Task with depend clause */
            #pragma omp task depend(inout: c) shared(c)
            {
                int local_c;
                #pragma omp atomic read
                local_c = c;
                local_c += 1;
                #pragma omp atomic write
                c = local_c;
                printf("Task 1: c = %d\n", c);
            }
            
            /* Another task with different depend clause */
            #pragma omp task depend(inout: c) shared(c)
            {
                int local_c;
                #pragma omp atomic read
                local_c = c;
                local_c *= 2;
                #pragma omp atomic write
                c = local_c;
                printf("Task 2: c = %d\n", c);
            }
            
            /* Task without depend clause */
            #pragma omp task shared(sum)
            {
                #pragma omp atomic
                sum += 5;
                printf("Task 3: sum = %d\n", sum);
            }
        } /* end taskgroup */
        
        /* Additional parallel region inside to trigger OMP_CLAUSE_PARALLEL case */
        #pragma omp parallel num_threads(2) if(thread_id == 0)
        {
            #pragma omp single
            {
                printf("Nested parallel region\n");
            }
        }
        
    } /* end parallel region */
    
    printf("Final sum = %d, c = %d\n", sum, c);
    return 0;
}
