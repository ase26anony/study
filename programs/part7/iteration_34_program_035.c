/* test_taskgroup.c - Program to trigger OMP_CLAUSE_TASKGROUP pretty-printing */
#include <stdio.h>
#include <omp.h>

int main() {
    volatile int N = 100;
    int sum = 0;
    int x = 0, y = 0, z = 0;
    
    printf("Starting OpenMP test with taskgroup...\n");
    
    #pragma omp parallel shared(sum, x, y, z) firstprivate(N)
    {
        int thread_id = omp_get_thread_num();
        
        /* OMP_CLAUSE_FOR case */
        #pragma omp for schedule(dynamic) nowait
        for (int i = 0; i < N; i++) {
            #pragma omp atomic
            sum += i;
        }
        
        /* OMP_CLAUSE_SECTIONS case */
        #pragma omp sections
        {
            #pragma omp section
            {
                x = thread_id * 10;
                printf("Section 1: thread %d, x = %d\n", thread_id, x);
            }
            
            #pragma omp section
            {
                y = thread_id * 20;
                printf("Section 2: thread %d, y = %d\n", thread_id, y);
            }
        }
        
        /* OMP_CLAUSE_TASKGROUP case - TARGET FOR COVERAGE */
        #pragma omp taskgroup
        {
            /* Task with depend clause */
            #pragma omp task depend(inout: z) shared(z)
            {
                int local_z;
                #pragma omp atomic read
                local_z = z;
                local_z += 1;
                #pragma omp atomic write
                z = local_z;
                printf("Task 1: thread %d updated z\n", omp_get_thread_num());
            }
            
            /* Another task with different depend clause */
            #pragma omp task depend(in: z) depend(out: x) shared(x, z)
            {
                #pragma omp atomic
                x += z;
                printf("Task 2: thread %d, x = %d, z = %d\n", 
                       omp_get_thread_num(), x, z);
            }
            
            /* Task without depend clause */
            #pragma omp task shared(sum)
            {
                #pragma omp atomic
                sum += 100;
                printf("Task 3: thread %d added to sum\n", omp_get_thread_num());
            }
            
            #pragma omp taskwait
        } /* end taskgroup */
        
        /* Nested parallel region for OMP_CLAUSE_PARALLEL case */
        #pragma omp parallel num_threads(1) if(0)
        {
            /* Empty nested parallel region just to generate the clause */
            printf("Nested parallel region (if(0) so may not execute)\n");
        }
        
    } /* end parallel region */
    
    printf("Final sum = %d, z = %d\n", sum, z);
    printf("Test completed.\n");
    
    return 0;
}
