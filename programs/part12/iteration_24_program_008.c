/* test_omp_clauses.c - Test program for OpenMP clause coverage in GCC tree pretty-printer */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_sum = 0;

/* Function 1: Test OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int local_sum = 0;
    
    /* Use combined parallel for with schedule clause */
    #pragma omp parallel for schedule(static) private(i) shared(local_sum)
    for (i = 0; i < 100; i++) {
        #pragma omp atomic
        local_sum += i;
    }
    
    #pragma omp parallel for schedule(dynamic, 4) reduction(+:global_sum)
    for (i = 0; i < 50; i++) {
        global_sum += i * 2;
    }
    
    printf("test_parallel_for completed: local_sum = %d, global_sum = %d\n", 
           local_sum, global_sum);
}

/* Function 2: Test OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int thread_id;
    
    /* Basic parallel region with private/shared clauses */
    #pragma omp parallel private(thread_id) shared(global_counter)
    {
        thread_id = omp_get_thread_num();
        
        #pragma omp critical
        {
            global_counter++;
            printf("Thread %d: global_counter = %d\n", thread_id, global_counter);
        }
    }
    
    /* Parallel region with reduction */
    int sum = 0;
    #pragma omp parallel reduction(+:sum)
    {
        sum += omp_get_thread_num() + 1;
    }
    
    printf("test_parallel completed: sum = %d\n", sum);
}

/* Function 3: Test OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section1_result = 0;
    int section2_result = 0;
    int section3_result = 0;
    
    /* Combined parallel sections */
    #pragma omp parallel sections private(global_counter) \
                shared(section1_result, section2_result, section3_result)
    {
        #pragma omp section
        {
            for (int i = 0; i < 30; i++) {
                section1_result += i;
            }
            printf("Section 1 completed: %d\n", section1_result);
        }
        
        #pragma omp section
        {
            for (int i = 0; i < 40; i++) {
                section2_result += i * 2;
            }
            printf("Section 2 completed: %d\n", section2_result);
        }
        
        #pragma omp section
        {
            for (int i = 0; i < 50; i++) {
                section3_result += i * 3;
            }
            printf("Section 3 completed: %d\n", section3_result);
        }
    }
    
    int total = section1_result + section2_result + section3_result;
    printf("test_sections completed: total = %d\n", total);
}

/* Function 4: Test OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int taskgroup_sum = 0;
    
    /* Create tasks within a taskgroup */
    #pragma omp parallel master
    {
        #pragma omp taskgroup
        {
            #pragma omp task shared(taskgroup_sum)
            {
                #pragma omp atomic
                taskgroup_sum += 100;
            }
            
            #pragma omp task shared(taskgroup_sum)
            {
                #pragma omp atomic
                taskgroup_sum += 200;
            }
            
            #pragma omp task shared(taskgroup_sum)
            {
                #pragma omp atomic
                taskgroup_sum += 300;
            }
            
            /* Wait for all tasks in the taskgroup */
            #pragma omp taskwait
        }
    }
    
    /* Alternative: taskgroup within single region */
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
    
    printf("test_taskgroup completed: taskgroup_sum = %d, global_counter = %d\n", 
           taskgroup_sum, global_counter);
}

/* Function 5: Test nested combinations */
void test_nested_constructs(void) {
    /* Nested parallel regions */
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < 10; i++) {
            #pragma omp parallel for
            for (int j = 0; j < 5; j++) {
                #pragma omp atomic
                global_sum += i * j;
            }
        }
    }
    
    /* Sections containing taskgroups */
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    global_counter++;
                }
            }
        }
        
        #pragma omp section
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    global_counter += 2;
                }
            }
        }
    }
    
    printf("test_nested_constructs completed\n");
}

/* Main function that calls all test functions */
int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Initialize OpenMP */
    omp_set_num_threads(4);
    
    /* Execute all test functions */
    test_parallel_for();
    test_parallel();
    test_sections();
    test_taskgroup();
    test_nested_constructs();
    
    /* Final verification */
    printf("\nAll tests completed successfully!\n");
    printf("Final values: global_counter = %d, global_sum = %d\n", 
           global_counter, global_sum);
    
    return 0;
}
