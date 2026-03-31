/* test_omp_clauses.c - Test program to trigger GCC pretty-printer for OpenMP clauses */
#include <stdio.h>
#include <omp.h>

volatile int global_counter = 0;

/* Test OMP_CLAUSE_FOR clause */
void test_parallel_for(void) {
    int i;
    int sum = 0;
    
    /* Direct usage of 'for' clause in combined construct */
    #pragma omp parallel for schedule(static) private(i) shared(sum) reduction(+:sum)
    for (i = 0; i < 100; i++) {
        sum += i;
    }
    
    /* Another variant with collapse */
    #pragma omp parallel for collapse(2) private(i) schedule(dynamic, 4)
    for (i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            #pragma omp atomic
            global_counter++;
        }
    }
    
    printf("test_parallel_for: sum = %d, global_counter = %d\n", sum, global_counter);
}

/* Test OMP_CLAUSE_PARALLEL clause */
void test_parallel(void) {
    int local_var = 0;
    
    /* Standalone parallel region */
    #pragma omp parallel private(local_var) shared(global_counter)
    {
        local_var = omp_get_thread_num();
        #pragma omp critical
        {
            global_counter += local_var;
        }
    }
    
    /* Parallel region with if clause */
    #pragma omp parallel if(global_counter > 0) num_threads(2)
    {
        #pragma omp single
        {
            printf("test_parallel: Thread %d in parallel region\n", omp_get_thread_num());
        }
    }
    
    printf("test_parallel: global_counter = %d\n", global_counter);
}

/* Test OMP_CLAUSE_SECTIONS clause */
void test_sections(void) {
    int section1_result = 0;
    int section2_result = 0;
    
    /* Combined parallel sections */
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
    
    /* Separate sections construct */
    #pragma omp parallel
    {
        #pragma omp sections nowait
        {
            #pragma omp section
            { global_counter += 1; }
            
            #pragma omp section
            { global_counter += 2; }
        }
    }
    
    printf("test_sections: global_counter = %d\n", global_counter);
}

/* Test OMP_CLAUSE_TASKGROUP clause */
void test_taskgroup(void) {
    int task_sum = 0;
    
    /* Taskgroup within parallel master */
    #pragma omp parallel master
    {
        #pragma omp taskgroup
        {
            for (int i = 0; i < 10; i++) {
                #pragma omp task firstprivate(i) shared(task_sum)
                {
                    #pragma omp atomic
                    task_sum += i;
                }
            }
        }
        
        /* Wait for all tasks in the taskgroup */
        #pragma omp taskwait
    }
    
    /* Taskgroup within single region */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task shared(global_counter)
                {
                    #pragma omp atomic
                    global_counter += 100;
                }
                
                #pragma omp task shared(global_counter)
                {
                    #pragma omp atomic
                    global_counter += 200;
                }
            }
        }
    }
    
    printf("test_taskgroup: task_sum = %d, global_counter = %d\n", task_sum, global_counter);
}

/* Combined test with nested constructs */
void test_combined(void) {
    /* Nested: parallel region containing for loop and taskgroup */
    #pragma omp parallel
    {
        #pragma omp for schedule(guided)
        for (int i = 0; i < 20; i++) {
            #pragma omp atomic
            global_counter++;
        }
        
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                { global_counter += 5; }
                
                #pragma omp task
                { global_counter += 10; }
            }
        }
    }
    
    printf("test_combined: global_counter = %d\n", global_counter);
}

int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Reset counter */
    global_counter = 0;
    test_parallel_for();
    
    global_counter = 0;
    test_parallel();
    
    global_counter = 0;
    test_sections();
    
    global_counter = 0;
    test_taskgroup();
    
    global_counter = 0;
    test_combined();
    
    printf("\nAll tests completed successfully!\n");
    printf("Final global_counter value: %d\n", global_counter);
    
    return 0;
}
