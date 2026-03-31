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
        
        // 3. OMP_CLAUSE_TASKGROUP case (target)
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                // Task with depend clause
                #pragma omp task depend(inout: z) shared(z)
                {
                    z = 10;
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
                    sum += 5;
                    printf("Task 3: sum = %d (thread %d)\n", sum, omp_get_thread_num());
                }
            } // end taskgroup
            
            printf("Taskgroup completed\n");
        }
        
        // 4. Another parallel region inside (OMP_CLAUSE_PARALLEL case)
        #pragma omp parallel num_threads(2) if(0)  // if(0) ensures it doesn't actually spawn
        {
            // This inner parallel won't execute due to if(0), but creates the clause
            printf("Inner parallel (should not print)\n");
        }
    }
    
    printf("Final sum: %d\n", sum);
    printf("x=%d, y=%d, z=%d\n", x, y, z);
    
    return 0;
}
