#include <stdio.h>
#include <omp.h>

int main() {
    volatile int N = 100;
    int sum = 0;
    int x = 0, y = 0, z = 0;
    
    #pragma omp parallel shared(sum, x, y, z) private(N)
    {
        int thread_id = omp_get_thread_num();
        
        // OMP_CLAUSE_FOR case
        #pragma omp for schedule(dynamic) nowait
        for (int i = 0; i < N; i++) {
            #pragma omp atomic
            sum += i;
        }
        
        // OMP_CLAUSE_SECTIONS case
        #pragma omp sections
        {
            #pragma omp section
            {
                x = thread_id * 2;
                printf("Section 1: thread %d, x = %d\n", thread_id, x);
            }
            
            #pragma omp section
            {
                y = thread_id * 3;
                printf("Section 2: thread %d, y = %d\n", thread_id, y);
            }
        }
        
        // OMP_CLAUSE_TASKGROUP case - the target uncovered code
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
                    local_z += 1;
                    #pragma omp atomic write
                    z = local_z;
                    printf("Task 1: z = %d\n", z);
                }
                
                // Another task with different depend clause
                #pragma omp task depend(in: z) depend(out: x) shared(x, z)
                {
                    x = z * 10;
                    printf("Task 2: x = %d (based on z = %d)\n", x, z);
                }
                
                // Task without depend clause
                #pragma omp task shared(y)
                {
                    #pragma omp atomic
                    y += 5;
                    printf("Task 3: y = %d\n", y);
                }
                
                // Wait for all tasks in the taskgroup
                #pragma omp taskwait
            }
        }
        
        // OMP_CLAUSE_PARALLEL case (already in parallel region)
        #pragma omp barrier
    }
    
    printf("Final results: sum = %d, x = %d, y = %d, z = %d\n", sum, x, y, z);
    
    return 0;
}
