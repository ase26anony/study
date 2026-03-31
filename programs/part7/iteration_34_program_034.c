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
                x = omp_get_thread_num();
                printf("Section 1 executed by thread %d\n", x);
            }
            
            #pragma omp section
            {
                y = omp_get_thread_num() * 2;
                printf("Section 2 executed by thread %d\n", omp_get_thread_num());
            }
        }
        
        // 3. OMP_CLAUSE_TASKGROUP case - TARGET CODE
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                // Task with depend clause
                #pragma omp task depend(inout: z) shared(z)
                {
                    z = 1;
                    printf("Task 1 setting z = %d\n", z);
                }
                
                // Dependent task
                #pragma omp task depend(in: z) shared(z, sum)
                {
                    #pragma omp atomic
                    sum += z;
                    printf("Task 2 adding z=%d to sum\n", z);
                }
                
                // Independent task
                #pragma omp task shared(sum)
                {
                    #pragma omp atomic
                    sum += 100;
                    printf("Task 3 adding 100 to sum\n");
                }
                
                // Wait for all tasks in this taskgroup
                #pragma omp taskwait
            }
        }
    }
    
    printf("Final sum = %d\n", sum);
    printf("Final values: x=%d, y=%d, z=%d\n", x, y, z);
    
    return 0;
}
