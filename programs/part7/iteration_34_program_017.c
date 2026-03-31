/* tree-pretty-print.cc coverage test for OMP_CLAUSE_TASKGROUP */
#include <stdio.h>
#include <omp.h>

int main() {
    volatile int N = 100;
    int sum = 0;
    int x = 0, y = 0, z = 0;
    
    /* Parallel region to spawn threads */
    #pragma omp parallel shared(sum, x, y, z) firstprivate(N)
    {
        int thread_id = omp_get_thread_num();
        
        /* Loop worksharing construct - triggers OMP_CLAUSE_FOR */
        #pragma omp for schedule(dynamic) nowait
        for (int i = 0; i < N; i++) {
            #pragma omp atomic
            sum += i;
        }
        
        /* Sections construct - triggers OMP_CLAUSE_SECTIONS */
        #pragma omp sections
        {
            #pragma omp section
            {
                x = thread_id * 10;
                printf("Section 1: thread %d, x = %d\n", thread_id, x);
            }
            
            #pragma omp section
            {
                y = thread_id * 20;
                printf("Section 2: thread %d, y = %d\n", thread_id, y);
            }
        }
        
        /* TASKGROUP - the target clause for coverage */
        #pragma omp taskgroup
        {
            /* Task with depend clause */
            #pragma omp task depend(inout: x) shared(x)
            {
                int local_x = x;
                local_x += 5;
                #pragma omp atomic write
                x = local_x;
                printf("Task 1: x = %d\n", x);
            }
            
            /* Another task with different dependences */
            #pragma omp task depend(in: x) depend(out: y) shared(x, y)
            {
                int local_y = y;
                local_y = x * 2;
                #pragma omp atomic write
                y = local_y;
                printf("Task 2: y = %d (based on x=%d)\n", y, x);
            }
            
            /* Task without dependences */
            #pragma omp task shared(z)
            {
                #pragma omp atomic
                z += thread_id;
                printf("Task 3: thread %d, z = %d\n", thread_id, z);
            }
            
            /* Wait for all tasks in this taskgroup */
            #pragma omp taskwait
        }
        
        /* Additional parallel construct nested - triggers OMP_CLAUSE_PARALLEL */
        #pragma omp parallel num_threads(2) if(thread_id == 0)
        {
            if (omp_get_thread_num() == 0) {
                printf("Nested parallel region\n");
            }
        }
    }
    
    /* Final output to prevent optimization */
    printf("Final sum = %d, x = %d, y = %d, z = %d\n", sum, x, y, z);
    
    return 0;
}
