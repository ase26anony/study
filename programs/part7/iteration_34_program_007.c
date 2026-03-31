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
            }
            #pragma omp section
            {
                y = omp_get_num_threads();
            }
        }
        
        // 3. OMP_CLAUSE_TASKGROUP case (target uncovered lines)
        #pragma omp taskgroup
        {
            // Create task dependencies
            #pragma omp task depend(inout: x) shared(x)
            {
                x = x * 2;
            }
            
            #pragma omp task depend(in: x) depend(out: y) shared(x, y)
            {
                y = x + 1;
            }
            
            #pragma omp task depend(in: y) depend(out: z) shared(y, z)
            {
                z = y * 3;
            }
            
            #pragma omp task depend(in: z) shared(z, sum)
            {
                #pragma omp atomic
                sum += z;
            }
            
            // Additional tasks without dependencies
            #pragma omp task shared(sum)
            {
                #pragma omp atomic
                sum += 1;
            }
        }
        
        // 4. Nested taskgroup for additional coverage
        #pragma omp master
        {
            #pragma omp taskgroup
            {
                #pragma omp task shared(sum)
                {
                    #pragma omp atomic
                    sum += 100;
                }
            }
        }
    }
    
    printf("Final sum: %d\n", sum);
    printf("Values: x=%d, y=%d, z=%d\n", x, y, z);
    
    return 0;
}
