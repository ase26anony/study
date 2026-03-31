/* test_omp_clauses.c - Coverage for OMP_CLAUSE_FOR, PARALLEL, SECTIONS, TASKGROUP */

#include <stdio.h>
#include <omp.h>

volatile int global_counter = 0;

/* Test OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int sum = 0;
    volatile int local_sum = 0;
    
    /* Combined parallel for directive */
    #pragma omp parallel for schedule(static) private(i) shared(sum) reduction(+:local_sum)
    for (i = 0; i < 100; i++) {
        local_sum += i;
    }
    
    /* Separate for directive within parallel region */
    #pragma omp parallel private(i) shared(sum)
    {
        #pragma omp for schedule(dynamic) nowait
        for (i = 0; i < 50; i++) {
            #pragma omp atomic
            sum += i;
        }
    }
    
    printf("test_parallel_for: local_sum = %d, sum = %d\n", local_sum, sum);
    #pragma omp atomic
    global_counter += 1;
}

/* Test OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int thread_id;
    
    /* Basic parallel region */
    #pragma omp parallel private(thread_id) num_threads(2)
    {
        thread_id = omp_get_thread_num();
        #pragma omp critical
        {
            printf("test_parallel: Thread %d executing\n", thread_id);
        }
        
        /* Nested parallel region */
        #pragma omp parallel num_threads(1) copyin(thread_id)
        {
            #pragma omp single
            {
                printf("test_parallel: Nested region, outer thread %d\n", thread_id);
            }
        }
    }
    
    /* Parallel region with if clause */
    volatile int condition = 1;
    #pragma omp parallel if(condition) default(none) shared(condition)
    {
        #pragma omp single
        {
            printf("test_parallel: Conditional parallel region executed\n");
        }
    }
    
    #pragma omp atomic
    global_counter += 1;
}

/* Test OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section1_result = 0;
    int section2_result = 0;
    volatile int total = 0;
    
    /* Combined parallel sections */
    #pragma omp parallel sections private(section1_result, section2_result) shared(total) \
            num_threads(3)
    {
        #pragma omp section
        {
            section1_result = 100;
            for (int i = 0; i < 10; i++) {
                section1_result += i;
            }
            #pragma omp atomic
            total += section1_result;
            printf("test_sections: Section 1 completed, result = %d\n", section1_result);
        }
        
        #pragma omp section
        {
            section2_result = 200;
            for (int i = 0; i < 5; i++) {
                section2_result *= (i + 1);
            }
            #pragma omp atomic
            total += section2_result;
            printf("test_sections: Section 2 completed, result = %d\n", section2_result);
        }
        
        #pragma omp section
        {
            printf("test_sections: Section 3 (extra section)\n");
        }
    }
    
    /* Separate sections directive */
    #pragma omp parallel
    {
        #pragma omp sections nowait
        {
            #pragma omp section
            {
                #pragma omp atomic
                total += 1;
            }
            
            #pragma omp section
            {
                #pragma omp atomic
                total += 2;
            }
        }
    }
    
    printf("test_sections: Total = %d\n", total);
    #pragma omp atomic
    global_counter += 1;
}

/* Test OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_result = 0;
    volatile int sync_var = 0;
    
    /* Taskgroup within parallel master */
    #pragma omp parallel master num_threads(4)
    {
        #pragma omp taskgroup
        {
            #pragma omp task shared(task_result) firstprivate(sync_var)
            {
                int local = 42;
                for (int i = 0; i < 100; i++) {
                    local += i;
                }
                #pragma omp atomic
                task_result += local;
            }
            
            #pragma omp task shared(task_result)
            {
                #pragma omp atomic
                task_result += 100;
            }
            
            #pragma omp taskwait
        }
        
        printf("test_taskgroup: Master thread, task_result = %d\n", task_result);
    }
    
    /* Taskgroup within single region */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (int i = 0; i < 5; i++) {
                    #pragma omp task firstprivate(i) shared(sync_var)
                    {
                        #pragma omp atomic
                        sync_var += i;
                    }
                }
            }
            printf("test_taskgroup: Single region completed, sync_var = %d\n", sync_var);
        }
    }
    
    /* Nested taskgroups */
    #pragma omp parallel
    {
        #pragma omp master
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    #pragma omp taskgroup
                    {
                        #pragma omp task
                        {
                            #pragma omp atomic
                            global_counter += 1;
                        }
                    }
                }
            }
        }
    }
    
    #pragma omp atomic
    global_counter += 1;
}

/* Main function that calls all test functions */
int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    test_parallel_for();
    test_parallel();
    test_sections();
    test_taskgroup();
    
    printf("\nAll tests completed. Global counter = %d\n", global_counter);
    printf("Expected global counter value: 4 (one increment per test function)\n");
    
    if (global_counter == 4) {
        printf("SUCCESS: All OpenMP constructs executed correctly.\n");
        return 0;
    } else {
        printf("WARNING: Global counter mismatch. Some constructs may have been optimized out.\n");
        return 1;
    }
}
