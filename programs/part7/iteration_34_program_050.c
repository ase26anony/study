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
                    int local_z;
                    #pragma omp atomic read
                    local_z = z;
                    local_z += 5;
                    #pragma omp atomic write
                    z = local_z;
                    printf("Task 1: z = %d\n", z);
                }
                
                // Another task depending on the first
                #pragma omp task depend(inout: z) shared(z)
                {
                    int local_z;
                    #pragma omp atomic read
                    local_z = z;
                    local_z *= 2;
                    #pragma omp atomic write
                    z = local_z;
                    printf("Task 2: z = %d\n", z);
                }
                
                // Independent task
                #pragma omp task shared(sum)
                {
                    #pragma omp atomic
                    sum += 100;
                    printf("Task 3: updated sum\n");
                }
            } // end taskgroup
            
            printf("Taskgroup completed: final z = %d\n", z);
        } // end single
    } // end parallel
    
    printf("Final sum = %d, final z = %d\n", sum, z);
    
    return 0;
}
