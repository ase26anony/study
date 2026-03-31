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
                printf("Task 2: x = z + 10 = %d\n", x);
            }
            
            #pragma omp task depend(in: x) depend(out: y) shared(y, x)
            {
                y = x * 2;
                printf("Task 3: y = x * 2 = %d\n", y);
            }
            
            #pragma omp task depend(in: y) shared(sum, y)
            {
                #pragma omp atomic
                sum += y;
                printf("Task 4 adding y=%d to sum\n", y);
            }
        }
        
        // Ensure taskgroup completes
        #pragma omp taskwait
    }
    
    printf("Final sum = %d, x = %d, y = %d, z = %d\n", sum, x, y, z);
    return 0;
}
