/* test_omp_clauses.c - Test program for OpenMP clause coverage */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int result_parallel_for = 0;
volatile int result_parallel = 0;
volatile int result_sections = 0;
volatile int result_taskgroup = 0;

/* Function 1: Test OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int local_sum = 0;
    
    /* Pattern A: OMP_CLAUSE_FOR with parallel clause combined */
    #pragma omp parallel for schedule(static) private(i) shared(local_sum) \
        reduction(+:global_counter)
    for (i = 0; i < 100; i++) {
        local_sum += i;
        global_counter++;
    }
    
    result_parallel_for = local_sum;
    printf("test_parallel_for completed: sum = %d, global_counter = %d\n", 
           local_sum, global_counter);
}

/* Function 2: Test OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int thread_id;
    int local_var = 0;
    
    /* Pattern B: Standalone parallel region with data clauses */
    #pragma omp parallel private(thread_id) firstprivate(local_var) \
        shared(global_counter)
    {
        thread_id = omp_get_thread_num();
        local_var = thread_id * 10;
        
        #pragma omp critical
        {
            global_counter += local_var;
        }
    }
    
    result_parallel = global_counter;
    printf("test_parallel completed: global_counter = %d\n", global_counter);
}

/* Function 3: Test OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section1_result = 0;
    int section2_result = 0;
    int section3_result = 0;
    
    /* Pattern C: Parallel sections construct */
    #pragma omp parallel sections private(global_counter) \
        shared(section1_result, section2_result, section3_result)
    {
        #pragma omp section
        {
            for (int i = 0; i < 50; i++) {
                section1_result += i;
            }
            printf("Section 1 completed: %d\n", section1_result);
        }
        
        #pragma omp section
        {
            for (int i = 50; i < 100; i++) {
                section2_result += i;
            }
            printf("Section 2 completed: %d\n", section2_result);
        }
        
        #pragma omp section
        {
            for (int i = 100; i < 150; i++) {
                section3_result += i;
            }
            printf("Section 3 completed: %d\n", section3_result);
        }
    }
    
    result_sections = section1_result + section2_result + section3_result;
    printf("test_sections completed: total = %d\n", result_sections);
}

/* Function 4: Test OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_sum = 0;
    
    /* Pattern D: Taskgroup within parallel region */
    #pragma omp parallel shared(task_sum, global_counter)
    {
        #pragma omp master
        {
            /* Pattern D: Explicit taskgroup clause */
            #pragma omp taskgroup
            {
                for (int i = 0; i < 10; i++) {
                    #pragma omp task firstprivate(i) shared(task_sum)
                    {
                        int task_result = i * i;
                        #pragma omp atomic
                        task_sum += task_result;
                        
                        #pragma omp atomic
                        global_counter++;
                    }
                }
            }
            
            /* Additional taskgroup with depend clause */
            #pragma omp taskgroup
            {
                #pragma omp task depend(inout: task_sum)
                {
                    task_sum *= 2;
                }
            }
        }
    }
    
    result_taskgroup = task_sum;
    printf("test_taskgroup completed: task_sum = %d, global_counter = %d\n", 
           task_sum, global_counter);
}

/* Function 5: Combined constructs for additional coverage */
void test_combined(void) {
    int i, j;
    int matrix_sum = 0;
    
    /* Nested parallel for with collapse */
    #pragma omp parallel for collapse(2) private(i, j) shared(matrix_sum) \
        schedule(dynamic)
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            #pragma omp atomic
            matrix_sum += i * j;
        }
    }
    
    printf("test_combined completed: matrix_sum = %d\n", matrix_sum);
}

/* Main function that calls all test functions */
int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Reset global counter */
    global_counter = 0;
    
    /* Execute all test functions */
    test_parallel_for();
    
    global_counter = 0;  /* Reset for next test */
    test_parallel();
    
    test_sections();
    test_taskgroup();
    test_combined();
    
    /* Verify all functions executed */
    int total_results = result_parallel_for + result_parallel + 
                       result_sections + result_taskgroup;
    
    printf("\n=== Test Summary ===\n");
    printf("parallel_for result: %d\n", result_parallel_for);
    printf("parallel result: %d\n", result_parallel);
    printf("sections result: %d\n", result_sections);
    printf("taskgroup result: %d\n", result_taskgroup);
    printf("Total computation: %d\n", total_results);
    printf("Final global_counter: %d\n", global_counter);
    
    if (total_results > 0 && global_counter > 0) {
        printf("SUCCESS: All OpenMP constructs executed.\n");
        return 0;
    } else {
        printf("FAILURE: Some constructs may have been optimized away.\n");
        return 1;
    }
}
