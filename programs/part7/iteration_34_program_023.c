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
            sum += i * 2;
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
        
        // 3. OMP_CLAUSE_TASKGROUP case (TARGET)
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                // Task with depend clause
                #pragma omp task depend(inout: z) shared(z)
                {
                    z = 1;
                    printf("Task 1: setting z = %d\n", z);
                }
                
                // Dependent task
                #pragma omp task depend(in: z) shared(z, sum)
                {
                    #pragma omp atomic
                    sum += z * 100;
                    printf("Task 2: sum = %d\n", sum);
                }
                
                // Independent task
                #pragma omp task shared(sum)
                {
                    #pragma omp atomic
                    sum += 42;
                    printf("Task 3: adding 42, sum = %d\n", sum);
                }
                
                // Wait for all tasks in the taskgroup
                #pragma omp taskwait
            }
        }
        
        // 4. Another parallel region inside (nested if enabled)
        #pragma omp master
        {
            #pragma omp taskloop grainsize(10) shared(sum)
            for (int i = 0; i < 50; i++) {
                #pragma omp atomic
                sum += i;
            }
        }
    }
    
    printf("Final sum = %d\n", sum);
    return 0;
}
