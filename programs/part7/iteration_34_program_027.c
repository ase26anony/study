#include <stdio.h>
#include <omp.h>

int main() {
    volatile int N = 100;
    int sum = 0;
    int x = 0, y = 0, z = 0;
    
    #pragma omp parallel shared(sum, x, y, z) private(N)
    {
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
                x = omp_get_thread_num();
                printf("Section 1 executed by thread %d\n", x);
            }
            
            #pragma omp section
            {
                y = omp_get_thread_num() * 2;
                printf("Section 2 executed by thread %d\n", omp_get_thread_num());
            }
        }
        
        // 3. OMP_CLAUSE_TASKGROUP case (target uncovered lines)
        #pragma omp taskgroup
        {
            // Create tasks with dependences
            #pragma omp task depend(inout: z) shared(z)
            {
                z = 1;
                printf("Task 1 setting z = %d\n", z);
            }
            
            #pragma omp task depend(in: z) depend(out: x) shared(x, z)
            {
                x = z + 10;
                printf("Task 2: x = %d (based on z=%d)\n", x, z);
            }
            
            #pragma omp task depend(in: x) shared(sum, x)
            {
                #pragma omp atomic
                sum += x;
                printf("Task 3 adding x=%d to sum\n", x);
            }
            
            #pragma omp taskwait  // Ensure all tasks complete
        }
        
        // Additional parallel region to ensure OMP_CLAUSE_PARALLEL case
        #pragma omp parallel num_threads(2) if(0)  // if(0) ensures it doesn't actually spawn
        {
            // Empty parallel region just for the clause
        }
    }
    
    printf("Final sum = %d\n", sum);
    printf("Values: x=%d, y=%d, z=%d\n", x, y, z);
    
    return 0;
}
