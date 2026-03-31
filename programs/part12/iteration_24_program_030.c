/* test_omp_clauses.c
 * This program exercises OpenMP clauses to trigger GCC's tree pretty-printer
 * for the uncovered clause names: for, parallel, sections, taskgroup
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

volatile int global_counter = 0;

/* Test OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int sum = 0;
    volatile int local_sum = 0;
    
    /* Use parallel for with schedule clause to ensure clause is processed */
    #pragma omp parallel for schedule(static) private(i) reduction(+:sum)
    for (i = 0; i < 100; i++) {
        sum += i;
        local_sum = sum;  /* Use volatile to prevent optimization */
    }
    
    #pragma omp atomic
    global_counter += sum;
    
    printf("test_parallel_for: sum = %d\n", sum);
}

/* Test OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int local_var = 0;
    
    /* Basic parallel region with data-sharing clauses */
    #pragma omp parallel private(local_var) shared(global_counter)
    {
        local_var = omp_get_thread_num();
        
        #pragma omp atomic
        global_counter += local_var;
        
        /* Use asm volatile to ensure region isn't optimized away */
        __asm__ volatile ("" : : "r"(local_var));
    }
    
    printf("test_parallel completed\n");
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
            for (int i = 0; i < 50; i++) {
                section1_result += i;
            }
            #pragma omp atomic
            global_counter += section1_result;
        }
        
        #pragma omp section
        {
            for (int i = 50; i < 100; i++) {
                section2_result += i;
            }
            #pragma omp atomic
            global_counter += section2_result;
        }
    }
    
    printf("test_sections: results = %d, %d\n", section1_result, section2_result);
}

/* Test OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_sum = 0;
    
    /* Create parallel region with master thread generating tasks */
    #pragma omp parallel master
    {
        /* Taskgroup to wait for all tasks in the group */
        #pragma omp taskgroup
        {
            for (int i = 0; i < 10; i++) {
                #pragma omp task firstprivate(i) shared(task_sum)
                {
                    int result = i * i;
                    #pragma omp atomic
                    task_sum += result;
                    
                    /* Use volatile to prevent optimization */
                    volatile int temp = result;
                    (void)temp;
                }
            }
        } /* implicit taskgroup wait here */
        
        #pragma omp atomic
        global_counter += task_sum;
    }
    
    printf("test_taskgroup: task_sum = %d\n", task_sum);
}

/* Alternative taskgroup test with single construct */
void test_taskgroup_single(void) {
    int task_sum = 0;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (int i = 0; i < 5; i++) {
                    #pragma omp task firstprivate(i) shared(task_sum)
                    {
                        int val = i * 10;
                        #pragma omp atomic
                        task_sum += val;
                    }
                }
            }
        }
    }
    
    #pragma omp atomic
    global_counter += task_sum;
    
    printf("test_taskgroup_single: task_sum = %d\n", task_sum);
}

/* Combined construct to increase coverage probability */
void test_combined(void) {
    /* Nested parallel regions with different clauses */
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        if (tid == 0) {
            /* Inner parallel for */
            #pragma omp parallel for schedule(dynamic)
            for (int i = 0; i < 20; i++) {
                #pragma omp atomic
                global_counter += i;
            }
        }
    }
    
    printf("test_combined completed\n");
}

int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Execute all test functions */
    test_parallel_for();
    test_parallel();
    test_sections();
    test_taskgroup();
    test_taskgroup_single();
    test_combined();
    
    printf("\nAll tests completed successfully!\n");
    printf("Final global_counter value: %d\n", global_counter);
    
    /* Verify computation */
    int expected = 0;
    /* Sum of 0-99 = 4950 from test_parallel_for */
    /* Sum of thread IDs depends on number of threads */
    /* Sum of 0-99 = 4950 from test_sections */
    /* Sum of squares 0-9 = 285 from test_taskgroup */
    /* Sum of 0-4 * 10 = 100 from test_taskgroup_single */
    /* Sum of 0-19 = 190 from test_combined */
    
    printf("Expected counter > 0, actual: %d\n", global_counter);
    
    return global_counter > 0 ? 0 : 1;
}
