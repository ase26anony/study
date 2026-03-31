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
        
        // 3. OMP_CLAUSE_TASKGROUP case - THE TARGET
        #pragma omp taskgroup
        {
            // Task with depend clause
            #pragma omp task depend(inout: z) firstprivate(thread_id)
            {
                int local_z = z;
                local_z += thread_id;
                #pragma omp atomic
                z += local_z;
                printf("Task 1: thread %d updated z\n", thread_id);
            }
            
            // Another task with different dependences
            #pragma omp task depend(in: z) depend(out: x)
            {
                x = z * 2;
                printf("Task 2: x = %d (based on z=%d)\n", x, z);
            }
            
            // Task without dependences
            #pragma omp task
            {
                #pragma omp atomic
                sum += 1;
                printf("Task 3: thread %d incremented sum\n", thread_id);
            }
            
            // Wait for all tasks in this taskgroup
            #pragma omp taskwait
        }
        
        // 4. Additional parallel region inside (optional)
        #pragma omp parallel num_threads(2) if(0)  // if(0) prevents actual parallelization
        {
            // This creates OMP_CLAUSE_PARALLEL case
            printf("Nested parallel region (may not execute)\n");
        }
    }
    
    printf("Final results: sum = %d, x = %d, y = %d, z = %d\n", sum, x, y, z);
    return 0;
}
