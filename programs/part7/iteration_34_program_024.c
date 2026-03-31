/* tree-pretty-print-target.c
 * 
 * This program is designed to trigger the uncovered lines in tree-pretty-print.cc
 * Specifically targeting the OMP_CLAUSE_TASKGROUP case in the pretty-printer.
 * 
 * Compile with: gcc -O1 -fopenmp -fdump-tree-all tree-pretty-print-target.c -o target
 * Or for GIMPLE focus: gcc -O2 -fopenmp -fdump-tree-gimple tree-pretty-print-target.c -o target
 * Or for OpenMP-specific: gcc -O0 -fopenmp -fdump-tree-omplower -fdump-tree-optimized tree-pretty-print-target.c -o target
 */

#include <stdio.h>
#include <omp.h>

int main() {
    volatile int N = 100;  /* Prevent optimization */
    int i, sum = 0;
    int section_result1 = 0, section_result2 = 0;
    int task_result1 = 0, task_result2 = 0, task_result3 = 0;
    int dep_var1 = 0, dep_var2 = 0, dep_var3 = 0;
    
    printf("Starting OpenMP test with taskgroup...\n");
    
    /* 1. Parallel region to spawn threads */
    #pragma omp parallel shared(sum, N) private(i)
    {
        int thread_id = omp_get_thread_num();
        
        /* 2. For loop with schedule clause */
        #pragma omp for schedule(dynamic) reduction(+:sum)
        for (i = 0; i < N; i++) {
            sum += i * 2;
        }
        
        /* 3. Sections block */
        #pragma omp sections
        {
            #pragma omp section
            {
                section_result1 = thread_id * 10;
                printf("Section 1 executed by thread %d: result = %d\n", 
                       thread_id, section_result1);
            }
            
            #pragma omp section
            {
                section_result2 = thread_id * 20;
                printf("Section 2 executed by thread %d: result = %d\n", 
                       thread_id, section_result2);
            }
        }
        
        /* 4. TASKGROUP - THE TARGET CONSTRUCT */
        /* This should trigger OMP_CLAUSE_TASKGROUP in the pretty-printer */
        #pragma omp taskgroup
        {
            /* Task with depend clause */
            #pragma omp task depend(inout: dep_var1) shared(task_result1)
            {
                dep_var1 = 5;
                task_result1 = dep_var1 * 2;
                printf("Task 1: dep_var1 = %d, result = %d\n", 
                       dep_var1, task_result1);
            }
            
            /* Another task depending on the first */
            #pragma omp task depend(in: dep_var1) depend(out: dep_var2) shared(task_result2)
            {
                dep_var2 = dep_var1 + 10;
                task_result2 = dep_var2 * 3;
                printf("Task 2: dep_var2 = %d, result = %d\n", 
                       dep_var2, task_result2);
            }
            
            /* Independent task */
            #pragma omp task shared(task_result3)
            {
                task_result3 = 100;
                printf("Task 3: independent, result = %d\n", task_result3);
            }
            
            /* Wait for all tasks in the taskgroup */
            #pragma omp taskwait
        } /* end taskgroup */
        
        /* Nested taskgroup inside a single section for more complexity */
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    printf("Nested taskgroup task from thread %d\n", thread_id);
                }
            }
        }
        
    } /* end parallel region */
    
    /* Final computation to prevent dead code elimination */
    int final_result = sum + section_result1 + section_result2 + 
                       task_result1 + task_result2 + task_result3;
    
    printf("Final result: %d\n", final_result);
    printf("Sum from parallel for: %d\n", sum);
    
    return 0;
}
