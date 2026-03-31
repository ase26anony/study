#include <stdio.h>
#include <omp.h>

int main() {
    volatile int N = 100;
    int sum = 0;
    int x = 0, y = 0, z = 0;
    
    #pragma omp parallel shared(sum, x, y, z) private(N)
    {
        // OMP_CLAUSE_FOR case
        #pragma omp for schedule(dynamic)
        for (int i = 0; i < N; i++) {
            #pragma omp atomic
            sum += i;
        }
        
        // OMP_CLAUSE_SECTIONS case
        #pragma omp sections
        {
            #pragma omp section
            {
                x = 1;
                printf("Section 1 executed by thread %d\n", omp_get_thread_num());
            }
            
            #pragma omp section
            {
                y = 2;
                printf("Section 2 executed by thread %d\n", omp_get_thread_num());
            }
        }
        
        // OMP_CLAUSE_TASKGROUP case (target uncovered lines)
        #pragma omp taskgroup
        {
            // Task with depend clause
            #pragma omp task depend(inout: x)
            {
                x = x * 2;
                printf("Task 1: x = %d\n", x);
            }
            
            // Another task with depend clause
            #pragma omp task depend(in: x) depend(out: y)
            {
                y = x + 5;
                printf("Task 2: y = %d\n", y);
            }
            
            // Task without depend clause
            #pragma omp task
            {
                #pragma omp atomic
                z++;
                printf("Task 3: z = %d\n", z);
            }
            
            // Wait for all tasks in the group
            #pragma omp taskwait
        }
        
        // Additional parallel region to ensure OMP_CLAUSE_PARALLEL case
        #pragma omp parallel num_threads(2)
        {
            #pragma omp single
            {
                printf("Nested parallel region, thread %d\n", omp_get_thread_num());
            }
        }
    }
    
    printf("Final sum: %d, x=%d, y=%d, z=%d\n", sum, x, y, z);
    return 0;
}
