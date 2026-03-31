/* test_omp_clauses.c
 * This program exercises specific OpenMP clauses to trigger
 * the pretty-printer logic for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP.
 * Compile with: gcc -O1 -fopenmp -fdump-tree-all test_omp_clauses.c -o test_omp_clauses
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

volatile int global_counter = 0;

/* Test function for OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int sum = 0;
    volatile int local_sum = 0;
    
    /* Direct use of 'for' clause in combined parallel for directive */
    #pragma omp parallel for schedule(static) private(i) shared(sum) reduction(+:local_sum)
    for (i = 0; i < 100; i++) {
        local_sum += i;
    }
    
    /* Another variation with collapse */
    #pragma omp parallel for collapse(2) private(i) schedule(dynamic, 4)
    for (i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            #pragma omp atomic
            global_counter++;
        }
    }
    
    printf("test_parallel_for completed: local_sum = %d, global_counter = %d\n", 
           local_sum, global_counter);
}

/* Test function for OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int thread_id;
    volatile int shared_var = 0;
    
    /* Basic parallel region with data-sharing clauses */
    #pragma omp parallel private(thread_id) shared(shared_var) default(none)
    {
        thread_id = omp_get_thread_num();
        
        #pragma omp critical
        {
            shared_var += thread_id;
        }
    }
    
    /* Parallel region with if clause and num_threads */
    #pragma omp parallel if(1) num_threads(4) private(thread_id) firstprivate(shared_var)
    {
        thread_id = omp_get_thread_num();
        #pragma omp atomic
        global_counter += thread_id + shared_var;
    }
    
    printf("test_parallel completed: shared_var = %d\n", shared_var);
}

/* Test function for OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int result1 = 0, result2 = 0, result3 = 0;
    volatile int total = 0;
    
    /* Combined parallel sections directive */
    #pragma omp parallel sections private(result1, result2, result3) shared(total)
    {
        #pragma omp section
        {
            for (int i = 0; i < 50; i++) {
                result1 += i;
            }
            #pragma omp atomic
            total += result1;
        }
        
        #pragma omp section
        {
            for (int i = 50; i < 100; i++) {
                result2 += i;
            }
            #pragma omp atomic
            total += result2;
        }
        
        #pragma omp section
        {
            for (int i = 100; i < 150; i++) {
                result3 += i;
            }
            #pragma omp atomic
            total += result3;
        }
    }
    
    /* Separate sections directive inside parallel region */
    #pragma omp parallel
    {
        #pragma omp sections nowait
        {
            #pragma omp section
            {
                #pragma omp atomic
                global_counter++;
            }
            #pragma omp section
            {
                #pragma omp atomic
                global_counter += 2;
            }
        }
    }
    
    printf("test_sections completed: total = %d\n", total);
}

/* Test function for OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_result = 0;
    volatile int group_total = 0;
    
    /* Taskgroup inside parallel master region */
    #pragma omp parallel master
    {
        #pragma omp taskgroup
        {
            #pragma omp task shared(task_result) firstprivate(group_total)
            {
                int local = 0;
                for (int i = 0; i < 100; i++) {
                    local += i;
                }
                #pragma omp atomic
                task_result += local;
            }
            
            #pragma omp task shared(task_result)
            {
                int local = 0;
                for (int i = 100; i < 200; i++) {
                    local += i;
                }
                #pragma omp atomic
                task_result += local;
            }
        }
        
        /* Another taskgroup with task_reduction */
        #pragma omp taskgroup task_reduction(+:group_total)
        {
            #pragma omp task in_reduction(+:group_total)
            {
                group_total += 10;
            }
            
            #pragma omp task in_reduction(+:group_total)
            {
                group_total += 20;
            }
        }
    }
    
    /* Taskgroup inside parallel single region */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (int i = 0; i < 5; i++) {
                    #pragma omp task firstprivate(i) shared(global_counter)
                    {
                        #pragma omp atomic
                        global_counter += i;
                    }
                }
            }
        }
    }
    
    printf("test_taskgroup completed: task_result = %d, group_total = %d\n", 
           task_result, group_total);
}

/* Additional test with nested constructs to increase coverage */
void test_nested_constructs(void) {
    volatile int nested_sum = 0;
    
    /* Nested: parallel region containing for loop */
    #pragma omp parallel
    {
        #pragma omp for schedule(guided)
        for (int i = 0; i < 100; i++) {
            #pragma omp atomic
            nested_sum += i;
        }
        
        /* Sections inside parallel region */
        #pragma omp sections
        {
            #pragma omp section
            {
                #pragma omp taskgroup
                {
                    #pragma omp task
                    {
                        #pragma omp atomic
                        global_counter++;
                    }
                }
            }
        }
    }
    
    printf("test_nested_constructs completed: nested_sum = %d\n", nested_sum);
}

int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Call all test functions to exercise different OpenMP clauses */
    test_parallel_for();
    test_parallel();
    test_sections();
    test_taskgroup();
    test_nested_constructs();
    
    printf("\nAll tests completed successfully!\n");
    printf("Final global_counter value: %d\n", global_counter);
    
    /* Verify computations were performed */
    if (global_counter > 0) {
        printf("SUCCESS: OpenMP constructs were executed.\n");
        return 0;
    } else {
        printf("ERROR: No OpenMP work performed.\n");
        return 1;
    }
}
