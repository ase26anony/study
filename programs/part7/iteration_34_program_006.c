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
                printf("Section 1 executed by thread %d\n", x);
            }
            
            #pragma omp section
            {
                y = omp_get_thread_num() * 2;
                printf("Section 2 executed by thread %d\n", omp_get_thread_num());
            }
        }
        
        // 3. OMP_CLAUSE_TASKGROUP case - TARGET CODE
        #pragma omp taskgroup
        {
            // Create tasks with dependences
            #pragma omp task depend(out: z)
            {
                z = 1;
                printf("Task 1 setting z = %d\n", z);
            }
            
            #pragma omp task depend(in: z) depend(out: x)
            {
                x = z + 10;
                printf("Task 2: x = z + 10 = %d\n", x);
            }
            
            #pragma omp task depend(in: x) depend(out: y)
            {
                y = x * 2;
                printf("Task 3: y = x * 2 = %d\n", y);
            }
            
            #pragma omp task depend(in: y)
            {
                #pragma omp atomic
                sum += y;
                printf("Task 4 adding y=%d to sum\n", y);
            }
            
            // Wait for all tasks in the taskgroup
            #pragma omp taskwait
        }
        
        // 4. Another parallel region inside to ensure tree traversal
        #pragma omp parallel num_threads(2) if(0)
        {
            // This parallel region has if(0) so it won't execute,
            // but will still be parsed and appear in tree dumps
            printf("Nested parallel region (should not print)\n");
        }
    }
    
    printf("Final sum = %d\n", sum);
    printf("Values: x=%d, y=%d, z=%d\n", x, y, z);
    
    return 0;
}
