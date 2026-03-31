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
                y = omp_get_thread_num();
                printf("Section 2 executed by thread %d\n", y);
            }
        }
        
        // 3. OMP_CLAUSE_TASKGROUP case (target uncovered lines)
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                // Task with depend clause
                #pragma omp task depend(inout: z) shared(z)
                {
                    #pragma omp atomic
                    z += 1;
                    printf("Task 1: z = %d\n", z);
                }
                
                // Another task depending on the first
                #pragma omp task depend(in: z) shared(sum, z)
                {
                    int local_z = z;
                    #pragma omp atomic
                    sum += local_z;
                    printf("Task 2: sum = %d, z = %d\n", sum, local_z);
                }
                
                // Independent task
                #pragma omp task shared(sum)
                {
                    #pragma omp atomic
                    sum += 100;
                    printf("Task 3: sum = %d\n", sum);
                }
            } // end taskgroup
            
            printf("Taskgroup completed\n");
        } // end single
    } // end parallel
    
    printf("Final sum = %d\n", sum);
    printf("Final z = %d\n", z);
    
    return 0;
}
