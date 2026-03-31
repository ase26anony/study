/* test_omp_clauses.c - Test program for OpenMP clause coverage in GCC tree pretty-printer */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_sum = 0;

/* Test function for OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int local_sum = 0;
    
    /* Pattern A: OMP_CLAUSE_FOR with scheduling clause */
    #pragma omp parallel for schedule(static) private(i) shared(local_sum) reduction(+:global_sum)
    for (i = 0; i < 100; i++) {
        local_sum += i;
        global_sum += i;
    }
    
    printf("test_parallel_for: local_sum = %d, global_sum = %d\n", local_sum, global_sum);
}

/* Test function for OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int thread_id;
    
    /* Pattern B: Standalone OMP_CLAUSE_PARALLEL with data-sharing clauses */
    #pragma omp parallel private(thread_id) shared(global_counter)
    {
        thread_id = omp_get_thread_num();
        #pragma omp atomic
        global_counter++;
        
        printf("Thread %d: global_counter = %d\n", thread_id, global_counter);
    }
    
    printf("test_parallel: final global_counter = %d\n", global_counter);
}

/* Test function for OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section1_result = 0;
    int section2_result = 0;
    
    /* Pattern C: OMP_CLAUSE_SECTIONS with nested parallelism */
    #pragma omp parallel sections private(section1_result, section2_result) \
                shared(global_sum) firstprivate(global_counter)
    {
        #pragma omp section
        {
            for (int i = 0; i < 50; i++) {
                section1_result += i;
            }
            #pragma omp atomic
            global_sum += section1_result;
            printf("Section 1: result = %d\n", section1_result);
        }
        
        #pragma omp section
        {
            for (int i = 50; i < 100; i++) {
                section2_result += i;
            }
            #pragma omp atomic
            global_sum += section2_result;
            printf("Section 2: result = %d\n", section2_result);
        }
    }
    
    printf("test_sections: total = %d\n", section1_result + section2_result);
}

/* Test function for OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_sum = 0;
    
    /* Pattern D: OMP_CLAUSE_TASKGROUP within parallel region */
    #pragma omp parallel master
    {
        #pragma omp taskgroup
        {
            #pragma omp task shared(task_sum) firstprivate(global_counter)
            {
                int local_task_sum = 0;
                for (int i = 0; i < 25; i++) {
                    local_task_sum += i;
                }
                #pragma omp atomic
                task_sum += local_task_sum;
            }
            
            #pragma omp task shared(task_sum) firstprivate(global_counter)
            {
                int local_task_sum = 0;
                for (int i = 25; i < 50; i++) {
                    local_task_sum += i;
                }
                #pragma omp atomic
                task_sum += local_task_sum;
            }
        }
        
        /* Additional task after taskgroup */
        #pragma omp task shared(task_sum)
        {
            #pragma omp atomic
            task_sum += 100;
        }
    }
    
    printf("test_taskgroup: task_sum = %d\n", task_sum);
}

/* Combined construct to increase coverage probability */
void test_combined_constructs(void) {
    int i, j;
    
    /* Nested parallelism with multiple clauses */
    #pragma omp parallel for collapse(2) private(i, j) schedule(dynamic)
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            #pragma omp atomic
            global_sum += i * j;
        }
    }
    
    printf("test_combined_constructs: global_sum = %d\n", global_sum);
}

int main(void) {
    printf("=== Starting OpenMP clause coverage test ===\n");
    
    /* Reset counters */
    global_counter = 0;
    global_sum = 0;
    
    /* Test each clause in separate functions */
    test_parallel_for();
    test_parallel();
    test_sections();
    test_taskgroup();
    test_combined_constructs();
    
    /* Final verification */
    printf("\n=== Test Summary ===\n");
    printf("Final global_counter = %d\n", global_counter);
    printf("Final global_sum = %d\n", global_sum);
    
    if (global_sum > 0 && global_counter > 0) {
        printf("All OpenMP constructs executed successfully!\n");
        return 0;
    } else {
        printf("Warning: Some constructs may have been optimized away\n");
        return 1;
    }
}
