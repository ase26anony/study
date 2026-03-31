/* test_omp_clauses.c
 * This program tests various OpenMP clauses to trigger pretty-printer
 * coverage for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, 
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_sum = 0;

/* Test function for OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int local_sum = 0;
    
    /* Use combined parallel for directive with schedule clause */
    #pragma omp parallel for schedule(static) private(i) reduction(+:local_sum)
    for (i = 0; i < 100; i++) {
        local_sum += i;
    }
    
    #pragma omp atomic
    global_sum += local_sum;
    
    printf("test_parallel_for completed: sum = %d\n", local_sum);
}

/* Test function for OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int thread_id;
    
    /* Basic parallel region with private and shared clauses */
    #pragma omp parallel private(thread_id) shared(global_counter)
    {
        thread_id = omp_get_thread_num();
        
        #pragma omp atomic
        global_counter++;
        
        #pragma omp critical
        {
            printf("Thread %d in parallel region\n", thread_id);
        }
    }
    
    printf("test_parallel completed: %d threads executed\n", global_counter);
}

/* Test function for OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section1_result = 0;
    int section2_result = 0;
    int section3_result = 0;
    
    /* Combined parallel sections directive */
    #pragma omp parallel sections private(section1_result, section2_result, section3_result) \
                shared(global_sum)
    {
        #pragma omp section
        {
            for (int i = 0; i < 50; i++) {
                section1_result += i;
            }
            #pragma omp atomic
            global_sum += section1_result;
        }
        
        #pragma omp section
        {
            for (int i = 50; i < 100; i++) {
                section2_result += i;
            }
            #pragma omp atomic
            global_sum += section2_result;
        }
        
        #pragma omp section
        {
            for (int i = 100; i < 150; i++) {
                section3_result += i;
            }
            #pragma omp atomic
            global_sum += section3_result;
        }
    }
    
    printf("test_sections completed: results = %d, %d, %d\n", 
           section1_result, section2_result, section3_result);
}

/* Test function for OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_sum = 0;
    
    /* Create a parallel region with tasks inside a taskgroup */
    #pragma omp parallel
    {
        #pragma omp master
        {
            /* Taskgroup clause */
            #pragma omp taskgroup
            {
                for (int i = 0; i < 10; i++) {
                    #pragma omp task firstprivate(i) shared(task_sum)
                    {
                        int local_val = i * 10;
                        #pragma omp atomic
                        task_sum += local_val;
                    }
                }
            }
            
            /* Alternative: using single directive with tasks */
            #pragma omp single
            {
                #pragma omp taskgroup
                {
                    #pragma omp task
                    {
                        #pragma omp atomic
                        task_sum += 100;
                    }
                    
                    #pragma omp task
                    {
                        #pragma omp atomic
                        task_sum += 200;
                    }
                }
            }
        }
    }
    
    #pragma omp atomic
    global_sum += task_sum;
    
    printf("test_taskgroup completed: task_sum = %d\n", task_sum);
}

/* Additional test with nested constructs */
void test_nested_constructs(void) {
    int nested_sum = 0;
    
    /* Nested parallel region with for loop */
    #pragma omp parallel
    {
        #pragma omp for schedule(dynamic) reduction(+:nested_sum)
        for (int i = 0; i < 100; i++) {
            nested_sum += i;
        }
        
        /* Sections inside parallel region */
        #pragma omp sections
        {
            #pragma omp section
            {
                nested_sum += 1;
            }
            #pragma omp section
            {
                nested_sum += 2;
            }
        }
    }
    
    printf("test_nested_constructs completed: nested_sum = %d\n", nested_sum);
}

int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Reset global counters */
    global_counter = 0;
    global_sum = 0;
    
    /* Test each clause in separate functions */
    test_parallel_for();
    test_parallel();
    test_sections();
    test_taskgroup();
    test_nested_constructs();
    
    /* Final verification */
    printf("\nAll tests completed successfully!\n");
    printf("Final global_sum = %d\n", global_sum);
    printf("Final global_counter = %d\n", global_counter);
    
    return 0;
}
