/* test_omp_clauses.c - Test program for OpenMP clause coverage in GCC pretty-printer */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

volatile int global_counter = 0;  /* volatile to prevent optimization */

/* Test OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int sum = 0;
    int array[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        array[i] = i + 1;
    }
    
    /* Use parallel for with schedule clause */
    #pragma omp parallel for schedule(static) private(i) shared(array, sum) reduction(+:sum)
    for (i = 0; i < 100; i++) {
        sum += array[i];
    }
    
    printf("test_parallel_for: sum = %d (expected: 5050)\n", sum);
    #pragma omp atomic
    global_counter += sum;
}

/* Test OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int local_sum = 0;
    
    /* Basic parallel region with private/shared clauses */
    #pragma omp parallel private(local_sum) shared(global_counter)
    {
        int thread_id = omp_get_thread_num();
        local_sum = thread_id * 10;
        
        #pragma omp critical
        {
            global_counter += local_sum;
        }
    }
    
    printf("test_parallel: executed parallel region\n");
}

/* Test OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section1_result = 0;
    int section2_result = 0;
    int section3_result = 0;
    
    /* Parallel sections with different work in each section */
    #pragma omp parallel sections private(section1_result, section2_result, section3_result) \
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
        
        #pragma omp section
        {
            for (int i = 100; i < 150; i++) {
                section3_result += i;
            }
            #pragma omp atomic
            global_counter += section3_result;
        }
    }
    
    printf("test_sections: sections completed (results: %d, %d, %d)\n", 
           section1_result, section2_result, section3_result);
}

/* Test OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_sum = 0;
    
    /* Create tasks within a taskgroup */
    #pragma omp parallel master
    {
        #pragma omp taskgroup
        {
            for (int i = 0; i < 10; i++) {
                #pragma omp task firstprivate(i) shared(task_sum)
                {
                    int result = i * i;
                    #pragma omp atomic
                    task_sum += result;
                }
            }
        }
        /* Taskgroup ensures all tasks complete before continuing */
        
        #pragma omp atomic
        global_counter += task_sum;
    }
    
    printf("test_taskgroup: task_sum = %d (expected: 285)\n", task_sum);
}

/* Combined construct to test nested clause printing */
void test_combined(void) {
    int combined_sum = 0;
    
    /* Combined parallel for construct */
    #pragma omp parallel for schedule(dynamic) reduction(+:combined_sum)
    for (int i = 0; i < 100; i++) {
        combined_sum += i * 2;
    }
    
    #pragma omp atomic
    global_counter += combined_sum;
    printf("test_combined: combined_sum = %d\n", combined_sum);
}

int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Reset global counter */
    global_counter = 0;
    
    /* Execute all test functions */
    test_parallel_for();
    test_parallel();
    test_sections();
    test_taskgroup();
    test_combined();
    
    printf("\nAll tests completed.\n");
    printf("Final global_counter value: %d\n", global_counter);
    
    /* Verify results */
    int expected = 5050 + 285 + 9900; /* from parallel_for, taskgroup, and combined */
    /* Note: parallel and sections contributions depend on thread count */
    
    printf("Expected minimum value: %d\n", expected);
    printf("Test %s\n", (global_counter >= expected) ? "PASSED" : "FAILED");
    
    return 0;
}
