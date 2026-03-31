/* test_omp_clauses.c - Coverage for OpenMP clause name printing */
#include <stdio.h>
#include <omp.h>

volatile int global_counter = 0;

/* Test OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i, sum = 0;
    #pragma omp parallel for private(i) shared(sum) schedule(static, 4)
    for (i = 0; i < 100; i++) {
        #pragma omp atomic
        sum += i;
    }
    printf("test_parallel_for: sum = %d\n", sum);
    
    /* Another variant with collapse */
    #pragma omp parallel for collapse(2) private(i) firstprivate(sum)
    for (i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            #pragma omp atomic
            global_counter++;
        }
    }
}

/* Test OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int local_var = 0;
    
    #pragma omp parallel private(local_var) shared(global_counter)
    {
        local_var = omp_get_thread_num();
        #pragma omp critical
        {
            global_counter += local_var;
        }
    }
    printf("test_parallel: global_counter = %d\n", global_counter);
    
    /* Nested parallel region */
    #pragma omp parallel num_threads(2) if(1)
    {
        #pragma omp single
        {
            printf("Thread %d in nested parallel\n", omp_get_thread_num());
        }
    }
}

/* Test OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section1_result = 0, section2_result = 0;
    
    #pragma omp parallel sections private(section1_result, section2_result) \
        shared(global_counter)
    {
        #pragma omp section
        {
            section1_result = 1;
            for (int i = 0; i < 50; i++) {
                #pragma omp atomic
                global_counter++;
            }
        }
        
        #pragma omp section
        {
            section2_result = 2;
            for (int i = 0; i < 50; i++) {
                #pragma omp atomic
                global_counter--;
            }
        }
    }
    printf("test_sections: results = %d, %d\n", section1_result, section2_result);
}

/* Test OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_count = 0;
    
    #pragma omp parallel master
    {
        #pragma omp taskgroup
        {
            #pragma omp task shared(task_count)
            {
                #pragma omp atomic
                task_count++;
            }
            
            #pragma omp task shared(task_count)
            {
                #pragma omp atomic
                task_count++;
            }
            
            #pragma omp taskwait
        }
    }
    printf("test_taskgroup: task_count = %d\n", task_count);
    
    /* Alternative with single and taskgroup */
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
}

/* Combined constructs to increase coverage */
void test_combined(void) {
    /* Combined parallel for with reduction */
    int total = 0;
    #pragma omp parallel for reduction(+:total)
    for (int i = 0; i < 1000; i++) {
        total += i % 7;
    }
    printf("test_combined: reduction total = %d\n", total);
    
    /* Parallel sections with taskgroup inside */
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    #pragma omp atomic
                    global_counter += 10;
                }
            }
        }
        
        #pragma omp section
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    #pragma omp atomic
                    global_counter += 20;
                }
            }
        }
    }
}

int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    test_parallel();
    test_parallel_for();
    test_sections();
    test_taskgroup();
    test_combined();
    
    printf("Final global_counter = %d\n", global_counter);
    printf("All tests completed successfully!\n");
    
    return 0;
}
