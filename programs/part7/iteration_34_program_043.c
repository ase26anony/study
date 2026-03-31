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
                int local_sum;
                #pragma omp atomic read
                local_sum = sum;
                printf("Task 3 reading sum = %d\n", local_sum);
            }
            
            // Wait for all tasks in this taskgroup
            #pragma omp taskwait
        }
        
        // 4. Nested parallel region for OMP_CLAUSE_PARALLEL case
        #pragma omp parallel num_threads(2) if(0)  // if(0) ensures it doesn't actually spawn threads
        {
            // This inner parallel region won't execute due to if(0)
            // but will still be parsed and create OMP_CLAUSE_PARALLEL nodes
            printf("Inner parallel region (should not print)\n");
        }
    }
    
    printf("Final sum = %d\n", sum);
    printf("Values: x=%d, y=%d, z=%d\n", x, y, z);
    
    return 0;
}
