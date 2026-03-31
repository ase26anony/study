/* test_omp_clauses.c - Test program for GCC OpenMP clause pretty-printer coverage */

#include <stdio.h>
#include <omp.h>

volatile int global_counter = 0;

/* Test OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int sum = 0;
    volatile int local_var = 10;
    
    /* Combined parallel for directive - should trigger OMP_CLAUSE_FOR */
    #pragma omp parallel for private(i) shared(sum) schedule(static) num_threads(2)
    for (i = 0; i < 100; i++) {
        #pragma omp atomic
        sum += i + local_var;
    }
    
    printf("test_parallel_for: sum = %d\n", sum);
    global_counter += sum;
}

/* Test OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int local_sum = 0;
    
    /* Standalone parallel region - should trigger OMP_CLAUSE_PARALLEL */
    #pragma omp parallel private(local_sum) shared(global_counter) default(none)
    {
        int thread_id = omp_get_thread_num();
        local_sum = thread_id * 100;
        
        #pragma omp critical
        {
            global_counter += local_sum;
        }
    }
    
    printf("test_parallel: global_counter = %d\n", global_counter);
}

/* Test OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section1_result = 0;
    int section2_result = 0;
    
    /* Parallel sections - should trigger OMP_CLAUSE_SECTIONS */
    #pragma omp parallel sections private(section1_result, section2_result) \
        shared(global_counter) num_threads(2)
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
    
    printf("test_sections: section1=%d, section2=%d\n", 
           section1_result, section2_result);
}

/* Test OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_sum = 0;
    
    /* Parallel region with master and taskgroup - should trigger OMP_CLAUSE_TASKGROUP */
    #pragma omp parallel master shared(task_sum) num_threads(2)
    {
        #pragma omp taskgroup
        {
            #pragma omp task shared(task_sum) firstprivate(global_counter)
            {
                int local = global_counter + 1;
                #pragma omp atomic
                task_sum += local;
            }
            
            #pragma omp task shared(task_sum) firstprivate(global_counter)
            {
                int local = global_counter + 2;
                #pragma omp atomic
                task_sum += local;
            }
            
            #pragma omp taskwait
        }
    }
    
    printf("test_taskgroup: task_sum = %d\n", task_sum);
    global_counter += task_sum;
}

/* Additional test with nested constructs */
void test_nested_constructs(void) {
    int i, j;
    int matrix_sum = 0;
    
    /* Nested parallelism with for clauses */
    #pragma omp parallel for private(i, j) collapse(2) schedule(dynamic) \
        shared(matrix_sum) if(1)
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            #pragma omp atomic
            matrix_sum += i * j;
        }
    }
    
    printf("test_nested_constructs: matrix_sum = %d\n", matrix_sum);
    
    /* Taskgroup inside parallel region */
    #pragma omp parallel shared(global_counter)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    #pragma omp atomic
                    global_counter += 100;
                }
                
                #pragma omp task
                {
                    #pragma omp atomic
                    global_counter += 200;
                }
            }
        }
    }
}

/* Test with reduction clause combined with for */
void test_reduction_for(void) {
    int sum = 0;
    
    /* Parallel for with reduction - should still trigger OMP_CLAUSE_FOR */
    #pragma omp parallel for reduction(+:sum) schedule(guided)
    for (int i = 0; i < 1000; i++) {
        sum += i % 7;
    }
    
    printf("test_reduction_for: reduction sum = %d\n", sum);
    global_counter += sum;
}

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
    test_reduction_for();
    
    /* Final result */
    printf("\nAll tests completed successfully!\n");
    printf("Final global_counter value: %d\n", global_counter);
    
    /* Verify computation */
    if (global_counter > 0) {
        printf("SUCCESS: All OpenMP constructs executed.\n");
        return 0;
    } else {
        printf("ERROR: No computation performed.\n");
        return 1;
    }
}
