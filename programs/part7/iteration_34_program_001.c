#include <stdio.h>
#include <omp.h>

int main() {
    volatile int N = 100;
    int sum = 0;
    int task_result1 = 0, task_result2 = 0;
    int section_sum = 0;
    
    // Use volatile variables to prevent optimization
    volatile int sync_var1 = 0, sync_var2 = 0;
    
    #pragma omp parallel shared(sum, section_sum, task_result1, task_result2) \
                         private(N)
    {
        int thread_id = omp_get_thread_num();
        
        // 1. OpenMP for loop with schedule clause
        #pragma omp for schedule(dynamic) nowait
        for (int i = 0; i < N; i++) {
            #pragma omp atomic
            sum += i;
        }
        
        // 2. OpenMP sections block
        #pragma omp sections
        {
            #pragma omp section
            {
                int local_sum = 0;
                for (int i = 0; i < 10; i++) {
                    local_sum += i;
                }
                #pragma omp atomic
                section_sum += local_sum;
                printf("Section 1 from thread %d: local_sum = %d\n", 
                       thread_id, local_sum);
            }
            
            #pragma omp section
            {
                int local_prod = 1;
                for (int i = 1; i <= 5; i++) {
                    local_prod *= i;
                }
                #pragma omp atomic
                section_sum += local_prod;
                printf("Section 2 from thread %d: local_prod = %d\n", 
                       thread_id, local_prod);
            }
        }
        
        // 3. OpenMP taskgroup with nested tasks and dependences
        // This is the key construct to trigger OMP_CLAUSE_TASKGROUP
        #pragma omp taskgroup
        {
            // First task with depend clause
            #pragma omp task depend(out: sync_var1) shared(task_result1)
            {
                task_result1 = thread_id * 100;
                sync_var1 = 1;  // Signal completion
                printf("Task 1 from thread %d: result = %d\n", 
                       thread_id, task_result1);
            }
            
            // Second task depends on first
            #pragma omp task depend(in: sync_var1) depend(out: sync_var2) \
                             shared(task_result2)
            {
                task_result2 = task_result1 + 50;
                sync_var2 = 1;  // Signal completion
                printf("Task 2 from thread %d: result = %d\n", 
                       thread_id, task_result2);
            }
            
            // Third task depends on second
            #pragma omp task depend(in: sync_var2) shared(sum)
            {
                #pragma omp atomic
                sum += task_result2;
                printf("Task 3 from thread %d: added %d to sum\n", 
                       thread_id, task_result2);
            }
            
            // Wait for all tasks in this taskgroup
            #pragma omp taskwait
        }
        
        // Additional parallel for to ensure more tree nodes
        #pragma omp for schedule(static) reduction(+:sum)
        for (int i = 0; i < 20; i++) {
            sum += i * 2;
        }
    }
    
    // Final output to prevent dead code elimination
    printf("Final sum = %d\n", sum);
    printf("Section sum = %d\n", section_sum);
    printf("Task results: %d, %d\n", task_result1, task_result2);
    
    return 0;
}
