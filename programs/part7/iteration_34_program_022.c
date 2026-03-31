#include <stdio.h>
#include <omp.h>

int main() {
    volatile int N = 100;
    int sum = 0;
    int x = 0, y = 0, z = 0;
    
    #pragma omp parallel shared(sum, x, y, z) private(N)
    {
        int thread_id = omp_get_thread_num();
        
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
                x = thread_id * 2;
                printf("Section 1: thread %d, x = %d\n", thread_id, x);
            }
            
            #pragma omp section
            {
                y = thread_id * 3;
                printf("Section 2: thread %d, y = %d\n", thread_id, y);
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
                    z = 1;
                    printf("Task 1: setting z = %d\n", z);
                }
                
                // Another task depending on the first
                #pragma omp task depend(in: z) shared(sum)
                {
                    #pragma omp atomic
                    sum += z;
                    printf("Task 2: adding z=%d to sum=%d\n", z, sum);
                }
                
                // Independent task
                #pragma omp task shared(sum)
                {
                    #pragma omp atomic
                    sum += thread_id;
                    printf("Task 3: thread %d adding to sum=%d\n", thread_id, sum);
                }
            } // end taskgroup
        } // end single
        
        // 4. Additional parallel region to trigger OMP_CLAUSE_PARALLEL case
        #pragma omp parallel num_threads(2) if(0)  // if(0) ensures it doesn't actually spawn
        {
            // This inner parallel won't execute due to if(0)
            printf("Inner parallel region\n");
        }
    } // end outer parallel region
    
    printf("Final sum = %d\n", sum);
    printf("Final values: x=%d, y=%d, z=%d\n", x, y, z);
    
    return 0;
}
