/* tree-pretty-print-coverage.c
 * 
 * This program is designed to trigger the OMP_CLAUSE_TASKGROUP case
 * in GCC's tree pretty-printer (tree-pretty-print.cc lines 1434-1445).
 * 
 * Compile with: gcc -O1 -fopenmp -fdump-tree-all tree-pretty-print-coverage.c -o test
 * Or for more specific dumps: gcc -O0 -fopenmp -fdump-tree-omplower -fdump-tree-gimple tree-pretty-print-coverage.c
 */

#include <stdio.h>
#include <omp.h>

int main() {
    volatile int N = 100;  /* Prevent optimization */
    int sum = 0;
    int task_result1 = 0, task_result2 = 0, task_result3 = 0;
    int section_sum = 0;
    
    printf("Starting OpenMP constructs...\n");
    
    /* 1. Parallel region containing multiple OpenMP constructs */
    #pragma omp parallel shared(sum, task_result1, task_result2, task_result3, section_sum) private(N)
    {
        int thread_id = omp_get_thread_num();
        
        /* 2. For construct - triggers OMP_CLAUSE_FOR case */
        #pragma omp for schedule(dynamic) nowait
        for (int i = 0; i < N; i++) {
            #pragma omp atomic
            sum += i * 2;
        }
        
        /* 3. Sections construct - triggers OMP_CLAUSE_SECTIONS case */
        #pragma omp sections reduction(+:section_sum)
        {
            #pragma omp section
            {
                section_sum += thread_id * 10;
                printf("Section 1 executed by thread %d\n", thread_id);
            }
            
            #pragma omp section 
            {
                section_sum += thread_id * 20;
                printf("Section 2 executed by thread %d\n", thread_id);
                
                /* 4. TASKGROUP CONSTRUCT - TARGET FOR COVERAGE */
                /* This should trigger OMP_CLAUSE_TASKGROUP case in pretty-printer */
                #pragma omp taskgroup
                {
                    int local_var = 0;
                    
                    /* Task with depend clause */
                    #pragma omp task depend(inout: local_var) shared(task_result1)
                    {
                        task_result1 = thread_id * 100;
                        local_var = 1;
                        printf("Task 1 (depend): thread %d, result = %d\n", 
                               thread_id, task_result1);
                    }
                    
                    /* Another task depending on the first */
                    #pragma omp task depend(in: local_var) shared(task_result2)
                    {
                        task_result2 = task_result1 + 50;
                        printf("Task 2 (depend): thread %d, result = %d\n", 
                               thread_id, task_result2);
                    }
                    
                    /* Independent task without depend */
                    #pragma omp task shared(task_result3)
                    {
                        task_result3 = thread_id * 200;
                        printf("Task 3 (no depend): thread %d, result = %d\n", 
                               thread_id, task_result3);
                    }
                    
                    /* Wait for all tasks in this taskgroup */
                    #pragma omp taskwait
                } /* end taskgroup */
            } /* end section 2 */
            
            #pragma omp section
            {
                section_sum += thread_id * 30;
                printf("Section 3 executed by thread %d\n", thread_id);
            }
        } /* end sections */
        
        /* 5. Another parallel construct nested - triggers OMP_CLAUSE_PARALLEL case */
        #pragma omp parallel num_threads(1) if(0)  /* if(0) prevents actual nesting */
        {
            /* Empty parallel region just to create the clause */
            printf("Nested parallel region (single-threaded)\n");
        }
        
    } /* end parallel region */
    
    /* Final output to prevent dead code elimination */
    printf("Final results: sum = %d, section_sum = %d\n", sum, section_sum);
    printf("Task results: %d, %d, %d\n", task_result1, task_result2, task_result3);
    
    return 0;
}
