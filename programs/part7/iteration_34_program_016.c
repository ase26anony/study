#include <stdio.h>
#include <omp.h>

int main() {
    volatile int N = 100;
    int sum = 0;
    int x = 0, y = 0, z = 0;
    
    #pragma omp parallel shared(sum, x, y, z) private(N)
    {
        // 1. OMP_CLAUSE_FOR case
        #pragma omp for schedule(dynamic) nowait
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
        
        // 3. OMP_CLAUSE_TASKGROUP case (TARGET)
        #pragma omp taskgroup
        {
            // Task with depend clause
            #pragma omp task depend(inout: z) shared(z)
            {
                z = x + y;
                printf("Task 1: z = %d (thread %d)\n", z, omp_get_thread_num());
            }
            
            // Dependent task
            #pragma omp task depend(in: z) shared(sum, z)
            {
                #pragma omp atomic
                sum += z;
                printf("Task 2: sum = %d (thread %d)\n", sum, omp_get_thread_num());
            }
            
            // Independent task
            #pragma omp task shared(sum)
            {
                #pragma omp atomic
                sum += 100;
                printf("Task 3: added 100 to sum (thread %d)\n", omp_get_thread_num());
            }
            
            // Wait for all tasks in this taskgroup
            #pragma omp taskwait
        }
        
        // 4. Another parallel region inside (optional, for more coverage)
        #pragma omp parallel num_threads(2) if(0)  // if(0) prevents actual parallelization
        {
            // Empty parallel region just to generate the clause
        }
    }
    
    printf("Final sum: %d\n", sum);
    return 0;
}
