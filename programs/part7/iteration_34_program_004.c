#include <stdio.h>
#include <omp.h>

int main() {
    volatile int N = 100;
    int sum = 0;
    int x = 0, y = 0, z = 0;
    
    #pragma omp parallel shared(sum, x, y, z) private(N)
    {
        // 1. OMP_CLAUSE_FOR case
        #pragma omp for schedule(dynamic)
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
            }
            
            #pragma omp section
            {
                y = omp_get_num_threads();
            }
        }
        
        // 3. OMP_CLAUSE_TASKGROUP case (target uncovered lines)
        #pragma omp taskgroup
        {
            // Create tasks with dependences
            #pragma omp task depend(inout: z)
            {
                z = 1;
            }
            
            #pragma omp task depend(in: z) depend(out: x)
            {
                x = z + 2;
            }
            
            #pragma omp task depend(in: x) depend(out: y)
            {
                y = x * 3;
            }
            
            // Task without dependences
            #pragma omp task
            {
                #pragma omp atomic
                sum += 1;
            }
        }
        
        // 4. OMP_CLAUSE_PARALLEL case (already in parallel region)
        // The outer #pragma omp parallel covers this case
    }
    
    printf("Result: sum = %d, x = %d, y = %d, z = %d\n", sum, x, y, z);
    return 0;
}
