#include <stdio.h>
#include <omp.h>

int main() {
    volatile int N = 100;
    int sum = 0;
    int x = 0, y = 0, z = 0;
    
    #pragma omp parallel shared(sum, x, y, z) private(N)
    {
        int thread_id = omp_get_thread_num();
        
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
                x = thread_id * 10;
                printf("Section 1: thread %d, x = %d\n", thread_id, x);
            }
            
            #pragma omp section
            {
                y = thread_id * 20;
                printf("Section 2: thread %d, y = %d\n", thread_id, y);
            }
        }
        
        // 3. OMP_CLAUSE_TASKGROUP case (target uncovered lines)
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                // Create tasks with dependences
                #pragma omp task depend(out: z)
                {
                    z = 42;
                    printf("Task 1: Setting z = %d\n", z);
                }
                
                #pragma omp task depend(in: z) depend(out: x)
                {
                    x = z + 1;
                    printf("Task 2: x = z + 1 = %d\n", x);
                }
                
                #pragma omp task depend(in: x) depend(in: z)
                {
                    #pragma omp atomic
                    sum += x + z;
                    printf("Task 3: sum updated to %d\n", sum);
                }
                
                // Task without dependences
                #pragma omp task
                {
                    printf("Task 4: Independent task from thread %d\n", thread_id);
                }
            } // end taskgroup
            
            printf("Taskgroup completed in thread %d\n", thread_id);
        } // end single
    } // end parallel
    
    printf("Final sum = %d\n", sum);
    printf("Values: x = %d, y = %d, z = %d\n", x, y, z);
    
    return 0;
}
