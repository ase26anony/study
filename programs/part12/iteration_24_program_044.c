/* test_omp_clauses.c - Comprehensive OpenMP test for GCC pretty-printer coverage */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_sum = 0;

/* Function 1: Tests OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int local_sum = 0;
    int n = 100;
    
    /* Use combined parallel for clause with scheduling */
    #pragma omp parallel for schedule(static) private(i) shared(local_sum, n) reduction(+:global_sum)
    for (i = 0; i < n; i++) {
        int temp = i * i;
        local_sum += temp;
        #pragma omp atomic
        global_sum += temp;
    }
    
    printf("test_parallel_for completed: local_sum = %d, global_sum = %d\n", 
           local_sum, global_sum);
}

/* Function 2: Tests OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int thread_id;
    int local_counter = 0;
    
    /* Basic parallel region with data-sharing clauses */
    #pragma omp parallel private(thread_id) shared(global_counter) firstprivate(local_counter)
    {
        thread_id = omp_get_thread_num();
        local_counter = thread_id + 1;
        
        #pragma omp atomic
        global_counter += local_counter;
        
        #pragma omp critical
        {
            printf("Thread %d: local_counter = %d\n", thread_id, local_counter);
        }
    }
    
    printf("test_parallel completed: global_counter = %d\n", global_counter);
}

/* Function 3: Tests OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section_a_result = 0;
    int section_b_result = 0;
    int section_c_result = 0;
    
    /* Combined parallel sections construct */
    #pragma omp parallel sections private(section_a_result, section_b_result, section_c_result) \
            shared(global_sum)
    {
        #pragma omp section
        {
            for (int i = 0; i < 50; i++) {
                section_a_result += i;
            }
            #pragma omp atomic
            global_sum += section_a_result;
            printf("Section A: result = %d\n", section_a_result);
        }
        
        #pragma omp section
        {
            for (int i = 50; i < 100; i++) {
                section_b_result += i;
            }
            #pragma omp atomic
            global_sum += section_b_result;
            printf("Section B: result = %d\n", section_b_result);
        }
        
        #pragma omp section
        {
            for (int i = 100; i < 150; i++) {
                section_c_result += i;
            }
            #pragma omp atomic
            global_sum += section_c_result;
            printf("Section C: result = %d\n", section_c_result);
        }
    }
    
    printf("test_sections completed: total = %d\n", 
           section_a_result + section_b_result + section_c_result);
}

/* Function 4: Tests OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_results[10] = {0};
    
    /* Create a parallel region with master directive */
    #pragma omp parallel
    {
        #pragma omp master
        {
            printf("Master thread starting taskgroup...\n");
            
            /* Taskgroup clause */
            #pragma omp taskgroup
            {
                for (int i = 0; i < 10; i++) {
                    /* Create tasks within taskgroup */
                    #pragma omp task firstprivate(i) shared(task_results)
                    {
                        int result = i * 100;
                        for (int j = 0; j < 100; j++) {
                            result += j;
                        }
                        task_results[i] = result;
                        
                        #pragma omp atomic
                        global_counter++;
                    }
                }
                
                /* Wait for all tasks in the group */
                #pragma omp taskwait
            }
        }
    }
    
    /* Verify task results */
    int total = 0;
    for (int i = 0; i < 10; i++) {
        total += task_results[i];
    }
    printf("test_taskgroup completed: total = %d, tasks completed = %d\n", 
           total, global_counter);
}

/* Function 5: Tests nested constructs for additional coverage */
void test_nested_constructs(void) {
    int i, j;
    
    /* Nested parallel regions with for clauses */
    #pragma omp parallel private(i) shared(global_sum)
    {
        #pragma omp for schedule(dynamic) nowait
        for (i = 0; i < 10; i++) {
            int inner_sum = 0;
            
            /* Nested loop with another parallel region */
            #pragma omp parallel for private(j) reduction(+:inner_sum)
            for (j = 0; j < 100; j++) {
                inner_sum += i * j;
            }
            
            #pragma omp atomic
            global_sum += inner_sum;
        }
    }
    
    printf("test_nested_constructs completed: global_sum = %d\n", global_sum);
}

/* Main function that calls all test functions */
int main(void) {
    printf("Starting OpenMP clause coverage test...\n\n");
    
    /* Reset global counters */
    global_counter = 0;
    global_sum = 0;
    
    /* Execute all test functions */
    test_parallel();
    printf("---\n");
    
    test_parallel_for();
    printf("---\n");
    
    test_sections();
    printf("---\n");
    
    test_taskgroup();
    printf("---\n");
    
    test_nested_constructs();
    printf("---\n");
    
    /* Final verification */
    printf("\nAll tests completed successfully!\n");
    printf("Final counters: global_counter = %d, global_sum = %d\n", 
           global_counter, global_sum);
    
    return 0;
}
