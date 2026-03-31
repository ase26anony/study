#include <stdio.h>
#include <omp.h>

int main() {
    volatile int N = 100;
    int sum = 0;
    int task_result = 0;
    int dep_var1 = 0, dep_var2 = 0;
    
    #pragma omp parallel shared(sum, dep_var1, dep_var2) private(task_result)
    {
        int thread_id = omp_get_thread_num();
        
        // OMP_CLAUSE_FOR case
        #pragma omp for schedule(dynamic) nowait
        for (int i = 0; i < N; i++) {
            sum += i;
        }
        
        // OMP_CLAUSE_SECTIONS case
        #pragma omp sections
        {
            #pragma omp section
            {
                dep_var1 = thread_id * 10;
            }
            
            #pragma omp section
            {
                dep_var2 = thread_id * 20;
            }
        }
        
        // OMP_CLAUSE_TASKGROUP case (target)
        #pragma omp taskgroup
        {
            // Create tasks with dependences
            #pragma omp task depend(inout: dep_var1) shared(dep_var1)
            {
                dep_var1 += 5;
            }
            
            #pragma omp task depend(in: dep_var1) depend(out: dep_var2) shared(dep_var1, dep_var2)
            {
                dep_var2 = dep_var1 * 2;
            }
            
            #pragma omp task depend(in: dep_var2) shared(sum, dep_var2)
            {
                #pragma omp atomic
                sum += dep_var2;
            }
            
            // Wait for all tasks in this taskgroup
            #pragma omp taskwait
        }
        
        // Additional parallel region inside to ensure tree traversal
        #pragma omp parallel num_threads(2) if(0)
        {
            // Empty parallel region - just for structure
        }
    }
    
    printf("Final sum: %d\n", sum);
    printf("dep_var1: %d, dep_var2: %d\n", dep_var1, dep_var2);
    
    return 0;
}
