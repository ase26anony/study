#include <stdio.h>
#include <omp.h>

int main() {
    volatile int N = 100;
    int sum = 0;
    int task_result1 = 0, task_result2 = 0, task_result3 = 0;
    int section_sum = 0;
    
    // Start parallel region with multiple threads
    #pragma omp parallel shared(sum, task_result1, task_result2, task_result3, section_sum) \
                         private(N)
    {
        int thread_id = omp_get_thread_num();
        
        // 1. OMP_CLAUSE_FOR case: for loop with schedule clause
        #pragma omp for schedule(dynamic) nowait
        for (int i = 0; i < N; i++) {
            #pragma omp atomic
            sum += i * 2;
        }
        
        // 2. OMP_CLAUSE_SECTIONS case: sections construct
        #pragma omp sections reduction(+:section_sum)
        {
            #pragma omp section
            {
                section_sum += thread_id * 10;
                printf("Section 1 executed by thread %d\n", thread_id);
            }
            
            #pragma omp section
            {
                section_sum += thread_id * 20;
                printf("Section 2 executed by thread %d\n", thread_id);
            }
            
            #pragma omp section
            {
                section_sum += thread_id * 30;
                printf("Section 3 executed by thread %d\n", thread_id);
            }
        }
        
        // 3. OMP_CLAUSE_TASKGROUP case: taskgroup with tasks
        // This is the main target for coverage
        #pragma omp taskgroup
        {
            // Task with depend clause
            #pragma omp task depend(inout: task_result1) shared(task_result1)
            {
                task_result1 = thread_id * 100;
                printf("Task 1: thread %d -> result %d\n", thread_id, task_result1);
            }
            
            // Another task depending on the first
            #pragma omp task depend(in: task_result1) depend(out: task_result2) \
                             shared(task_result1, task_result2)
            {
                task_result2 = task_result1 + 50;
                printf("Task 2: depends on task1, result %d\n", task_result2);
            }
            
            // Independent task
            #pragma omp task shared(task_result3)
            {
                task_result3 = thread_id * 200;
                printf("Task 3: independent, result %d\n", task_result3);
            }
            
            // Wait for all tasks in the taskgroup
            #pragma omp taskwait
        }
        
        // 4. OMP_CLAUSE_PARALLEL case: nested parallel region
        // (Note: This creates a new team, not just a clause)
        #pragma omp parallel num_threads(2) if(0)  // if(0) prevents actual nesting for simplicity
        {
            // Empty nested parallel - just to generate the clause
        }
    }
    
    // Final output to prevent optimization
    printf("Final sum: %d\n", sum);
    printf("Section sum: %d\n", section_sum);
    printf("Task results: %d, %d, %d\n", task_result1, task_result2, task_result3);
    
    return 0;
}
