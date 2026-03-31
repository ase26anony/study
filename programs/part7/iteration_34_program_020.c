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
                
                // Another task depending on the first
                #pragma omp task depend(in: z) shared(sum)
                {
                    #pragma omp atomic
                    sum += z;
                    printf("Task 2: thread %d, added z=%d to sum\n", 
                           thread_id, z);
                }
                
                // Independent task
                #pragma omp task shared(sum)
                {
                    #pragma omp atomic
                    sum += 1000;
                    printf("Task 3: thread %d, added 1000 to sum\n", thread_id);
                }
            } // end taskgroup
            
            printf("Taskgroup completed in thread %d\n", thread_id);
        } // end single
    } // end parallel
    
    printf("Final sum = %d\n", sum);
    printf("Final values: x=%d, y=%d, z=%d\n", x, y, z);
    
    return 0;
}
