/* test_omp_clauses.c - Test program for GCC OpenMP clause pretty-printer coverage */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

volatile int global_counter = 0;

/* Test OMP_CLAUSE_FOR - Pattern A */
void test_parallel_for(void) {
    int i;
    int sum = 0;
    volatile int local_sum = 0;
    
    /* Combined parallel for directive with schedule clause */
    #pragma omp parallel for schedule(static) private(i) shared(sum) reduction(+:local_sum)
    for (i = 0; i < 100; i++) {
        local_sum += i;
    }
    
    /* Another variant with collapse */
    #pragma omp parallel for collapse(2) private(i) firstprivate(sum)
    for (i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            #pragma omp atomic
            global_counter++;
        }
    }
    
    printf("test_parallel_for completed: local_sum = %d, global_counter = %d\n", 
           local_sum, global_counter);
}

/* Test OMP_CLAUSE_PARALLEL - Pattern B */
void test_parallel(void) {
    int thread_id;
    volatile int shared_var = 0;
    
    /* Basic parallel region with private clause */
    #pragma omp parallel private(thread_id) shared(shared_var)
    {
        thread_id = omp_get_thread_num();
        
        #pragma omp critical
        {
            shared_var += thread_id;
        }
        
        /* Nested parallel region */
        #pragma omp parallel num_threads(2)
        {
            #pragma omp single
            {
                #pragma omp atomic
                global_counter++;
            }
        }
    }
    
    printf("test_parallel completed: shared_var = %d\n", shared_var);
}

/* Test OMP_CLAUSE_SECTIONS - Pattern C */
void test_sections(void) {
    int section1_result = 0;
    int section2_result = 0;
    volatile int final_result = 0;
    
    /* Parallel sections with nowait clause */
    #pragma omp parallel sections private(section1_result, section2_result) \
                                   shared(final_result) nowait
    {
        #pragma omp section
        {
            for (int i = 0; i < 50; i++) {
                section1_result += i;
            }
            #pragma omp atomic
            final_result += section1_result;
        }
        
        #pragma omp section
        {
            for (int i = 50; i < 100; i++) {
                section2_result += i;
            }
            #pragma omp atomic
            final_result += section2_result;
        }
        
        #pragma omp section
        {
            /* Empty section to ensure multiple sections */
            #pragma omp atomic
            global_counter += 10;
        }
    }
    
    printf("test_sections completed: final_result = %d\n", final_result);
}

/* Test OMP_CLAUSE_TASKGROUP - Pattern D */
void test_taskgroup(void) {
    int task_count = 0;
    volatile int task_sum = 0;
    
    /* Parallel region creating tasks with taskgroup */
    #pragma omp parallel master
    {
        #pragma omp taskgroup
        {
            for (int i = 0; i < 20; i++) {
                #pragma omp task firstprivate(i) shared(task_sum, task_count) \
                               depend(out: task_count)
                {
                    #pragma omp atomic
                    task_sum += i;
                    
                    #pragma omp atomic
                    task_count++;
                }
            }
            
            /* Another taskgroup inside the first one */
            #pragma omp taskgroup
            {
                #pragma omp task shared(global_counter)
                {
                    #pragma omp atomic
                    global_counter += 100;
                }
            }
        }
        
        /* Wait for all tasks in the taskgroup */
        #pragma omp taskwait
    }
    
    printf("test_taskgroup completed: task_sum = %d, task_count = %d\n", 
           task_sum, task_count);
}

/* Combined test with nested constructs */
void test_combined(void) {
    volatile int combined_result = 0;
    
    /* Nested: parallel -> for -> taskgroup */
    #pragma omp parallel
    {
        #pragma omp for schedule(dynamic) private(combined_result)
        for (int i = 0; i < 10; i++) {
            #pragma omp single
            {
                #pragma omp taskgroup
                {
                    #pragma omp task shared(combined_result)
                    {
                        #pragma omp atomic
                        combined_result += i;
                    }
                }
            }
        }
    }
    
    printf("test_combined completed: combined_result = %d\n", combined_result);
}

int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Initialize OpenMP */
    omp_set_num_threads(4);
    omp_set_nested(1);
    
    /* Execute all test functions */
    test_parallel_for();
    test_parallel();
    test_sections();
    test_taskgroup();
    test_combined();
    
    /* Final verification */
    printf("\nAll tests completed successfully!\n");
    printf("Final global_counter value: %d\n", global_counter);
    
    return 0;
}
