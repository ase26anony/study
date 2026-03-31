#include <stdio.h>
#include <omp.h>

int main() {
    volatile int N = 100;
    int sum = 0;
    int x = 0, y = 0, z = 0;
    
    #pragma omp parallel shared(sum, x, y, z) firstprivate(N)
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
                printf("Task 1: thread %d updated z\n", omp_get_thread_num());
            }
            
            // Another task with different depend clause
            #pragma omp task depend(in: z) depend(out: x) shared(x, z)
            {
                int local_z;
                #pragma omp atomic read
                local_z = z;
                x = local_z * 2;
                printf("Task 2: thread %d set x = %d based on z = %d\n", 
                       omp_get_thread_num(), x, local_z);
            }
            
            // Task without depend clause
            #pragma omp task shared(y)
            {
                #pragma omp atomic
                y += 1;
                printf("Task 3: thread %d incremented y\n", omp_get_thread_num());
            }
            
            // Wait for all tasks in this taskgroup
            #pragma omp taskwait
        }
        
        // 4. Another OMP_CLAUSE_PARALLEL case (nested)
        #pragma omp master
        {
            #pragma omp parallel num_threads(2) shared(sum)
            {
                #pragma omp atomic
                sum += omp_get_thread_num();
            }
        }
    }
    
    printf("Final results: sum = %d, x = %d, y = %d, z = %d\n", sum, x, y, z);
    
    return 0;
}
