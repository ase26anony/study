/* test_omp_clauses.c
 * This program exercises specific OpenMP clauses to trigger
 * pretty-printer coverage for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP.
 */

#include <stdio.h>
#include <omp.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int dummy = 0;

/* Test OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int sum = 0;
    
    /* Pattern A: OMP_CLAUSE_FOR with schedule clause */
    #pragma omp parallel for private(i) shared(sum) schedule(static)
    for (i = 0; i < 100; i++) {
        #pragma omp atomic
        sum += i;
    }
    
    printf("test_parallel_for: sum = %d\n", sum);
    global_counter += sum;
}

/* Test OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int local_sum = 0;
    
    /* Pattern B: Standalone parallel region with data clauses */
    #pragma omp parallel private(dummy) shared(local_sum)
    {
        int thread_id = omp_get_thread_num();
        #pragma omp atomic
        local_sum += thread_id;
    }
    
    printf("test_parallel: local_sum = %d\n", local_sum);
    global_counter += local_sum;
}

/* Test OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section1_result = 0, section2_result = 0;
    
    /* Pattern C: Parallel sections construct */
    #pragma omp parallel sections private(dummy) shared(section1_result, section2_result)
    {
        #pragma omp section
        {
            for (int i = 0; i < 50; i++) {
                section1_result += i;
            }
        }
        
        #pragma omp section
        {
            for (int i = 50; i < 100; i++) {
                section2_result += i;
            }
        }
    }
    
    int total = section1_result + section2_result;
    printf("test_sections: total = %d\n", total);
    global_counter += total;
}

/* Test OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_sum = 0;
    
    /* Pattern D: Taskgroup within parallel region */
    #pragma omp parallel private(dummy) shared(task_sum)
    {
        #pragma omp master
        {
            #pragma omp taskgroup
            {
                #pragma omp task shared(task_sum)
                {
                    for (int i = 0; i < 25; i++) {
                        #pragma omp atomic
                        task_sum += i;
                    }
                }
                
                #pragma omp task shared(task_sum)
                {
                    for (int i = 25; i < 50; i++) {
                        #pragma omp atomic
                        task_sum += i;
                    }
                }
            }
        }
    }
    
    printf("test_taskgroup: task_sum = %d\n", task_sum);
    global_counter += task_sum;
}

/* Combined construct to increase coverage probability */
void test_combined(void) {
    int combined_sum = 0;
    
    /* Combined parallel for with multiple clauses */
    #pragma omp parallel for private(dummy) shared(combined_sum) schedule(dynamic)
    for (int i = 0; i < 200; i++) {
        #pragma omp atomic
        combined_sum += i % 10;
    }
    
    printf("test_combined: combined_sum = %d\n", combined_sum);
    global_counter += combined_sum;
}

int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Execute all test functions */
    test_parallel_for();
    test_parallel();
    test_sections();
    test_taskgroup();
    test_combined();
    
    printf("All tests completed. Global counter = %d\n", global_counter);
    printf("Expected value if all tests ran correctly: 12475\n");
    
    return 0;
}
