/* test_omp_clauses.c
 * This program exercises specific OpenMP clauses to trigger
 * pretty-printer coverage for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP.
 */

#include <stdio.h>
#include <omp.h>

volatile int global_counter = 0;

/* Test function for OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int sum = 0;
    
    /* Direct use of 'for' clause in combined parallel for construct */
    #pragma omp parallel for schedule(static) private(i) reduction(+:sum)
    for (i = 0; i < 100; i++) {
        sum += i;
    }
    
    /* Additional nested for clause */
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (i = 0; i < 50; i++) {
            #pragma omp atomic
            global_counter++;
        }
    }
    
    printf("test_parallel_for: sum = %d, global_counter = %d\n", sum, global_counter);
}

/* Test function for OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int local_var = 0;
    
    /* Standalone parallel region */
    #pragma omp parallel private(local_var) shared(global_counter)
    {
        local_var = omp_get_thread_num();
        #pragma omp atomic
        global_counter += local_var;
    }
    
    /* Parallel region with if clause */
    #pragma omp parallel if(global_counter > 0) num_threads(2)
    {
        #pragma omp single
        {
            printf("test_parallel: Thread %d in second parallel region\n", 
                   omp_get_thread_num());
        }
    }
    
    printf("test_parallel completed\n");
}

/* Test function for OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section1_result = 0, section2_result = 0;
    
    /* Combined parallel sections construct */
    #pragma omp parallel sections private(global_counter) \
            firstprivate(section1_result, section2_result)
    {
        #pragma omp section
        {
            for (int i = 0; i < 100; i++) {
                section1_result += i;
            }
            printf("Section 1: result = %d\n", section1_result);
        }
        
        #pragma omp section
        {
            for (int i = 100; i < 200; i++) {
                section2_result += i;
            }
            printf("Section 2: result = %d\n", section2_result);
        }
    }
    
    /* Additional sections construct */
    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            { global_counter += 1; }
            #pragma omp section
            { global_counter += 2; }
        }
    }
    
    printf("test_sections: final counter = %d\n", global_counter);
}

/* Test function for OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_sum = 0;
    
    /* Taskgroup within parallel master */
    #pragma omp parallel master
    {
        #pragma omp taskgroup
        {
            #pragma omp task shared(task_sum)
            {
                #pragma omp atomic
                task_sum += 10;
            }
            
            #pragma omp task shared(task_sum)
            {
                #pragma omp atomic
                task_sum += 20;
            }
            
            #pragma omp taskwait
        }
    }
    
    /* Taskgroup within single region */
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
    
    printf("test_taskgroup: task_sum = %d, global_counter = %d\n", 
           task_sum, global_counter);
}

/* Additional test with nested clauses */
void test_nested_constructs(void) {
    /* Parallel region containing multiple clause types */
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < 10; i++) {
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    #pragma omp atomic
                    global_counter++;
                }
            }
        }
        
        #pragma omp sections
        {
            #pragma omp section
            { /* empty */ }
            #pragma omp section
            { /* empty */ }
        }
    }
    
    printf("test_nested_constructs completed\n");
}

int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Reset global counter */
    global_counter = 0;
    
    /* Test each clause type in separate functions */
    test_parallel_for();
    
    global_counter = 0;
    test_parallel();
    
    global_counter = 0;
    test_sections();
    
    global_counter = 0;
    test_taskgroup();
    
    global_counter = 0;
    test_nested_constructs();
    
    printf("\nAll OpenMP clause tests completed successfully!\n");
    printf("Final global_counter value: %d\n", global_counter);
    
    return 0;
}
