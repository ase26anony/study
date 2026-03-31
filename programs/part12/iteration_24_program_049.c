/* test_omp_clauses.c
 * This program exercises specific OpenMP clauses to trigger
 * pretty-printer coverage for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

volatile int global_counter = 0;  /* Prevent optimization */

/* Test OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int sum = 0;
    volatile int local_volatile = 0;
    
    /* Use parallel for with schedule clause */
    #pragma omp parallel for schedule(static) private(i) shared(sum, local_volatile)
    for (i = 0; i < 100; i++) {
        #pragma omp atomic
        sum += i;
        local_volatile = i;  /* Prevent loop optimization */
    }
    
    printf("test_parallel_for: sum = %d\n", sum);
    #pragma omp atomic
    global_counter += sum;
}

/* Test OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int local_sum = 0;
    
    /* Basic parallel region with data-sharing clauses */
    #pragma omp parallel private(local_sum) shared(global_counter)
    {
        local_sum = omp_get_thread_num();
        #pragma omp atomic
        global_counter += local_sum;
    }
    
    printf("test_parallel: executed with %d threads\n", 
           omp_get_max_threads());
}

/* Test OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section1_result = 0;
    int section2_result = 0;
    
    /* Combined parallel sections directive */
    #pragma omp parallel sections private(section1_result, section2_result) \
                shared(global_counter)
    {
        #pragma omp section
        {
            section1_result = 1;
            #pragma omp atomic
            global_counter += section1_result;
        }
        
        #pragma omp section
        {
            section2_result = 2;
            #pragma omp atomic
            global_counter += section2_result;
        }
    }
    
    printf("test_sections: results = %d, %d\n", 
           section1_result, section2_result);
}

/* Test OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_sum = 0;
    
    /* Create tasks within a taskgroup */
    #pragma omp parallel master
    {
        #pragma omp taskgroup
        {
            #pragma omp task shared(task_sum) firstprivate(global_counter)
            {
                #pragma omp atomic
                task_sum += 1;
            }
            
            #pragma omp task shared(task_sum)
            {
                #pragma omp atomic
                task_sum += 2;
            }
            
            /* Wait for all tasks in the group */
            #pragma omp taskwait
        }
        
        #pragma omp atomic
        global_counter += task_sum;
    }
    
    printf("test_taskgroup: task_sum = %d\n", task_sum);
}

/* Additional test with nested taskgroup */
void test_nested_taskgroup(void) {
    int nested_sum = 0;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task shared(nested_sum)
                {
                    #pragma omp atomic
                    nested_sum += 10;
                }
                
                /* Nested taskgroup */
                #pragma omp taskgroup
                {
                    #pragma omp task shared(nested_sum)
                    {
                        #pragma omp atomic
                        nested_sum += 20;
                    }
                }
            }
        }
    }
    
    #pragma omp atomic
    global_counter += nested_sum;
    printf("test_nested_taskgroup: nested_sum = %d\n", nested_sum);
}

/* Test combined parallel for with reduction */
void test_combined_constructs(void) {
    int reduction_sum = 0;
    
    /* Combined construct that should trigger both PARALLEL and FOR clauses */
    #pragma omp parallel for reduction(+:reduction_sum) schedule(dynamic)
    for (int i = 0; i < 50; i++) {
        reduction_sum += i * 2;
    }
    
    #pragma omp atomic
    global_counter += reduction_sum;
    printf("test_combined_constructs: reduction_sum = %d\n", reduction_sum);
}

int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    printf("Initial global_counter = %d\n", global_counter);
    
    /* Execute all test functions */
    test_parallel();
    test_parallel_for();
    test_sections();
    test_taskgroup();
    test_nested_taskgroup();
    test_combined_constructs();
    
    printf("\nAll tests completed.\n");
    printf("Final global_counter = %d\n", global_counter);
    
    /* Simple validation */
    if (global_counter > 0) {
        printf("SUCCESS: OpenMP constructs executed correctly.\n");
        return 0;
    } else {
        printf("ERROR: No OpenMP work performed.\n");
        return 1;
    }
}
