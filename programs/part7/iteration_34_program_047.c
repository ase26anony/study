#include <stdio.h>
#include <omp.h>

int main() {
    volatile int N = 100;
    int sum = 0;
    int x = 0, y = 0, z = 0;
    
    #pragma omp parallel shared(sum, x, y, z) private(N)
    {
        // 1. OMP_CLAUSE_FOR case
        #pragma omp for schedule(dynamic)
        for (int i = 0; i < N; i++) {
            #pragma omp atomic
            sum += i;
        }
        
        // 2. OMP_CLAUSE_SECTIONS case
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
        
        // 3. OMP_CLAUSE_TASKGROUP case (target uncovered lines)
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                // Task with depend clause
                #pragma omp task depend(inout: x) shared(x)
                {
                    x = x * 2;
                    printf("Task 1: x = %d (thread %d)\n", x, omp_get_thread_num());
                }
                
                // Another task depending on the first
                #pragma omp task depend(in: x) depend(out: y) shared(x, y)
                {
                    y = x + 10;
                    printf("Task 2: y = %d (thread %d)\n", y, omp_get_thread_num());
                }
                
                // Independent task
                #pragma omp task shared(z)
                {
                    z = 100;
                    printf("Task 3: z = %d (thread %d)\n", z, omp_get_thread_num());
                }
                
                // Wait for all tasks in the taskgroup
                #pragma omp taskwait
            }
        }
        
        // 4. Another parallel region to ensure OMP_CLAUSE_PARALLEL case
        #pragma omp parallel num_threads(2) if(0)
        {
            // This parallel region has if(0) so it won't actually execute,
            // but will still be parsed and appear in tree dumps
            printf("Nested parallel region (should not print)\n");
        }
    }
    
    printf("Final sum = %d, x = %d, y = %d, z = %d\n", sum, x, y, z);
    return 0;
}
