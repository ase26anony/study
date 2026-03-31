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
                // Task with depend clause
                #pragma omp task depend(inout: z) shared(z)
                {
                    int local_z = z;
                    #pragma omp atomic
                    z += thread_id + 1;
                    printf("Task 1: thread %d, z = %d -> %d\n", 
                           thread_id, local_z, z);
                }
                
                // Another task with different depend
                #pragma omp task depend(in: z) depend(out: x) shared(x, z)
                {
                    x = z * 2;
                    printf("Task 2: thread %d, x = %d (based on z=%d)\n", 
                           thread_id, x, z);
                }
                
                // Task without depend clause
                #pragma omp task shared(y)
                {
                    #pragma omp atomic
                    y += 5;
                    printf("Task 3: thread %d, y = %d\n", thread_id, y);
                }
                
                // Wait for all tasks in the taskgroup
                #pragma omp taskwait
            } // end taskgroup
            
            printf("Taskgroup completed in thread %d\n", thread_id);
        } // end single
    } // end parallel
    
    printf("Final results: sum = %d, x = %d, y = %d, z = %d\n", sum, x, y, z);
    
    return 0;
}
