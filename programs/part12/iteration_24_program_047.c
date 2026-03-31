/* test_omp_clauses.c - Test program for OpenMP clause coverage in GCC pretty-printer */

#include <stdio.h>
#include <omp.h>

volatile int global_counter = 0;

/* Test OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int sum = 0;
    volatile int local_volatile = 0;
    
    /* Combined parallel for directive */
    #pragma omp parallel for schedule(static) private(i) shared(sum) firstprivate(local_volatile)
    for (i = 0; i < 100; i++) {
        #pragma omp atomic
        sum += i;
        local_volatile = i; /* Use volatile to prevent optimization */
    }
    
    printf("test_parallel_for: sum = %d\n", sum);
    
    #pragma omp parallel
    {
        #pragma omp for schedule(dynamic) nowait
        for (i = 0; i < 50; i++) {
            #pragma omp atomic
            global_counter++;
        }
    }
}

/* Test OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int local_var = 0;
    
    /* Standalone parallel region with multiple clauses */
    #pragma omp parallel private(local_var) shared(global_counter) default(none)
    {
        int thread_id = omp_get_thread_num();
        local_var = thread_id;
        
        #pragma omp critical
        {
            global_counter += local_var;
        }
    }
    
    /* Nested parallel region */
    #pragma omp parallel num_threads(2) if(1)
    {
        #pragma omp single
        {
            printf("test_parallel: Inside parallel region\n");
        }
    }
}

/* Test OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section1_result = 0;
    int section2_result = 0;
    
    /* Combined parallel sections */
    #pragma omp parallel sections private(section1_result, section2_result) \
                shared(global_counter)
    {
        #pragma omp section
        {
            for (int i = 0; i < 100; i++) {
                section1_result += i;
            }
            #pragma omp atomic
            global_counter += section1_result;
        }
        
        #pragma omp section
        {
            for (int i = 0; i < 50; i++) {
                section2_result += i * 2;
            }
            #pragma omp atomic
            global_counter += section2_result;
        }
    }
    
    printf("test_sections: section results = %d, %d\n", section1_result, section2_result);
}

/* Test OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_results[10] = {0};
    
    /* Parallel region with master construct and taskgroup */
    #pragma omp parallel
    {
        #pragma omp master
        {
            #pragma omp taskgroup
            {
                for (int i = 0; i < 10; i++) {
                    #pragma omp task firstprivate(i) shared(task_results)
                    {
                        task_results[i] = i * i;
                    }
                }
            }
            
            /* Wait for all tasks in taskgroup */
            #pragma omp taskwait
        }
    }
    
    /* Alternative: single construct with taskgroup */
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
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += task_results[i];
    }
    printf("test_taskgroup: task results sum = %d\n", sum);
}

/* Test mixed/nested constructs */
void test_mixed_constructs(void) {
    /* Nested: parallel region containing for loop */
    #pragma omp parallel
    {
        #pragma omp for collapse(2) ordered
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                #pragma omp ordered
                {
                    #pragma omp atomic
                    global_counter++;
                }
            }
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
                        global_counter += 5;
                    }
                }
            }
            
            #pragma omp section
            {
                #pragma omp task
                {
                    #pragma omp atomic
                    global_counter += 10;
                }
            }
        }
    }
}

int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Call all test functions to ensure all constructs are processed */
    test_parallel_for();
    test_parallel();
    test_sections();
    test_taskgroup();
    test_mixed_constructs();
    
    printf("Final global_counter value: %d\n", global_counter);
    printf("All OpenMP constructs processed successfully.\n");
    
    return 0;
}
