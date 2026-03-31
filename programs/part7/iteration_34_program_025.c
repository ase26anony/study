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
                x = omp_get_thread_num();
                printf("Section 1 executed by thread %d\n", x);
            }
            
            #pragma omp section
            {
                y = omp_get_thread_num() * 2;
                printf("Section 2 executed by thread %d\n", omp_get_thread_num());
            }
        }
        
        // 3. OMP_CLAUSE_TASKGROUP case (target uncovered lines)
        #pragma omp taskgroup
        {
            // Task with depend clause
            #pragma omp task depend(inout: z) shared(z)
            {
                z = 1;
                printf("Task 1 setting z = %d\n", z);
            }
            
            // Another task depending on the first
            #pragma omp task depend(in: z) shared(sum)
            {
                #pragma omp atomic
                sum += z;
                printf("Task 2 adding z=%d to sum\n", z);
            }
            
            // Independent task
            #pragma omp task shared(sum)
            {
                int local = omp_get_thread_num();
                #pragma omp atomic
                sum += local;
                printf("Task 3 adding thread_id=%d to sum\n", local);
            }
            
            // Wait for all tasks in this taskgroup
            #pragma omp taskwait
        }
        
        // 4. Another parallel region to ensure OMP_CLAUSE_PARALLEL case
        #pragma omp parallel num_threads(2) if(0)
        {
            // This parallel region won't execute (if(0)), but will be parsed
            printf("Nested parallel region\n");
        }
    }
    
    printf("Final sum = %d\n", sum);
    printf("Final z = %d\n", z);
    
    return 0;
}
