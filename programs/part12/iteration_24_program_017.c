/* test_omp_clauses.c - Test program for GCC OpenMP clause pretty-printer coverage */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_sum = 0;

/* Function 1: Tests OMP_CLAUSE_FOR with combined parallel for */
void test_parallel_for(void) {
    int i;
    int local_sum = 0;
    
    /* Combined parallel for directive - triggers OMP_CLAUSE_FOR */
    #pragma omp parallel for schedule(static) private(i) shared(local_sum) reduction(+:global_sum)
    for (i = 0; i < 100; i++) {
        local_sum += i;
        global_sum += i;
    }
    
    printf("test_parallel_for: local_sum = %d, global_sum = %d\n", local_sum, global_sum);
}

/* Function 2: Tests OMP_CLAUSE_PARALLEL with standalone parallel region */
void test_parallel(void) {
    int thread_id;
    
    /* Standalone parallel directive - triggers OMP_CLAUSE_PARALLEL */
    #pragma omp parallel private(thread_id) shared(global_counter)
    {
        thread_id = omp_get_thread_num();
        #pragma omp atomic
        global_counter++;
        
        /* Additional work to prevent optimization */
        int temp = thread_id * thread_id;
        if (temp > 0) {
            /* Do nothing, just prevent dead code elimination */
        }
    }
    
    printf("test_parallel: global_counter = %d\n", global_counter);
}

/* Function 3: Tests OMP_CLAUSE_SECTIONS with parallel sections */
void test_sections(void) {
    int section1_result = 0;
    int section2_result = 0;
    int section3_result = 0;
    
    /* Parallel sections directive - triggers OMP_CLAUSE_SECTIONS */
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
    
    printf("test_sections: section results = %d, %d, %d, global_sum = %d\n",
           section1_result, section2_result, section3_result, global_sum);
}

/* Function 4: Tests OMP_CLAUSE_TASKGROUP with task constructs */
void test_taskgroup(void) {
    int task_results[10] = {0};
    
    /* Parallel region with master directive */
    #pragma omp parallel
    {
        #pragma omp master
        {
            /* Taskgroup directive - triggers OMP_CLAUSE_TASKGROUP */
            #pragma omp taskgroup
            {
                for (int i = 0; i < 10; i++) {
                    #pragma omp task firstprivate(i) shared(task_results)
                    {
                        /* Simulate some work */
                        int result = i * i * i;
                        task_results[i] = result;
                        
                        #pragma omp atomic
                        global_counter += result % 7;
                    }
                }
            }
            
            /* Wait for all tasks in the taskgroup */
            #pragma omp taskwait
        }
    }
    
    /* Verify task results */
    int task_sum = 0;
    for (int i = 0; i < 10; i++) {
        task_sum += task_results[i];
    }
    printf("test_taskgroup: task_sum = %d, global_counter = %d\n", task_sum, global_counter);
}

/* Function 5: Tests nested constructs for additional coverage */
void test_nested_constructs(void) {
    int i, j;
    
    /* Nested parallel regions with for loops */
    #pragma omp parallel private(i) shared(global_sum)
    {
        #pragma omp for schedule(dynamic)
        for (i = 0; i < 10; i++) {
            int inner_sum = 0;
            
            /* Nested parallel region inside the loop */
            #pragma omp parallel for private(j) reduction(+:inner_sum)
            for (j = 0; j < 10; j++) {
                inner_sum += i * j;
            }
            
            #pragma omp atomic
            global_sum += inner_sum;
        }
    }
    
    printf("test_nested_constructs: global_sum = %d\n", global_sum);
}

/* Main function that calls all test functions */
int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Reset global variables */
    global_counter = 0;
    global_sum = 0;
    
    /* Execute all test functions */
    test_parallel_for();
    
    global_counter = 0;  /* Reset for next test */
    test_parallel();
    
    test_sections();
    
    global_counter = 0;  /* Reset for next test */
    test_taskgroup();
    
    test_nested_constructs();
    
    /* Final verification */
    printf("\nAll tests completed successfully!\n");
    printf("Final global_sum = %d, final global_counter = %d\n", global_sum, global_counter);
    
    return 0;
}
