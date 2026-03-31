#include <stdio.h>
#include <omp.h>

int main() {
    volatile int N = 100;
    int sum = 0;
    int x = 0, y = 0, z = 0;
    
    #pragma omp parallel shared(sum, x, y, z) private(N)
    {
        int thread_id = omp_get_thread_num();
        
        // 1. OpenMP for loop with schedule clause
        #pragma omp for schedule(dynamic)
        for (int i = 0; i < N; i++) {
            #pragma omp atomic
            sum += i;
        }
        
        // 2. OpenMP sections block
        #pragma omp sections
        {
            #pragma omp section
            {
                x = thread_id * 10;
                printf("Section 1: Thread %d, x = %d\n", thread_id, x);
            }
            
            #pragma omp section
            {
                y = thread_id * 20;
                printf("Section 2: Thread %d, y = %d\n", thread_id, y);
            }
        }
        
        // 3. CRITICAL: OpenMP taskgroup with nested tasks
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                // Task with depend clause
                #pragma omp task depend(inout: x) shared(x)
                {
                    x += 5;
                    printf("Task 1: x = %d\n", x);
                }
                
                // Another task with different depend clause
                #pragma omp task depend(inout: y) shared(y)
                {
                    y += 10;
                    printf("Task 2: y = %d\n", y);
                }
                
                // Task depending on both previous tasks
                #pragma omp task depend(in: x, y) depend(out: z) shared(x, y, z)
                {
                    z = x + y;
                    printf("Task 3: z = x + y = %d\n", z);
                }
                
                // Independent task without depend clause
                #pragma omp task shared(sum)
                {
                    #pragma omp atomic
                    sum += 1;
                    printf("Task 4: Incremented sum\n");
                }
            } // end taskgroup
            
            // Wait for taskgroup completion
            #pragma omp taskwait
        } // end single
    } // end parallel
    
    printf("Final results: sum = %d, x = %d, y = %d, z = %d\n", sum, x, y, z);
    
    return 0;
}
