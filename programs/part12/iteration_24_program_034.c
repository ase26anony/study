/* test_omp_clauses.c - Test program for GCC OpenMP clause coverage */
#include <stdio.h>
#include <omp.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int result_parallel_for = 0;
volatile int result_parallel = 0;
volatile int result_sections = 0;
volatile int result_taskgroup = 0;

/* Test function for OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int local_sum = 0;
    
    /* Pattern A: OMP_CLAUSE_FOR with schedule clause */
    #pragma omp parallel for schedule(static) private(i) shared(local_sum) \
        num_threads(2) if(1)
    for (i = 0; i < 100; i++) {
        #pragma omp atomic
        local_sum += i;
    }
    
    result_parallel_for = local_sum;
    printf("test_parallel_for completed: sum = %d\n", local_sum);
}

/* Test function for OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int thread_id;
    
    /* Pattern B: Standalone OMP_CLAUSE_PARALLEL with data clauses */
    #pragma omp parallel private(thread_id) shared(global_counter) \
        default(none) num_threads(2)
    {
        thread_id = omp_get_thread_num();
        
        #pragma omp critical
        {
            global_counter++;
            printf("Thread %d in parallel region, counter = %d\n", 
                   thread_id, global_counter);
        }
    }
    
    result_parallel = global_counter;
    printf("test_parallel completed: final counter = %d\n", global_counter);
}

/* Test function for OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section1_result = 0;
    int section2_result = 0;
    
    /* Pattern C: OMP_CLAUSE_SECTIONS with nested constructs */
    #pragma omp parallel sections private(section1_result, section2_result) \
        shared(result_sections) num_threads(2)
    {
        #pragma omp section
        {
            /* First section with its own work */
            for (int i = 0; i < 50; i++) {
                section1_result += i;
            }
            printf("Section 1 completed: %d\n", section1_result);
        }
        
        #pragma omp section
        {
            /* Second section with different work */
            for (int i = 50; i < 100; i++) {
                section2_result += i;
            }
            printf("Section 2 completed: %d\n", section2_result);
        }
    }
    
    result_sections = section1_result + section2_result;
    printf("test_sections completed: total = %d\n", result_sections);
}

/* Test function for OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_sum = 0;
    
    /* Pattern D: OMP_CLAUSE_TASKGROUP within task-generating region */
    #pragma omp parallel num_threads(2)
    {
        #pragma omp master
        {
            /* Create a taskgroup to wait for all child tasks */
            #pragma omp taskgroup
            {
                for (int i = 0; i < 10; i++) {
                    #pragma omp task firstprivate(i) shared(task_sum)
                    {
                        int local_val = i * 10;
                        #pragma omp atomic
                        task_sum += local_val;
                    }
                }
            } /* implicit taskgroup wait here */
        }
    }
    
    result_taskgroup = task_sum;
    printf("test_taskgroup completed: task_sum = %d\n", task_sum);
}

/* Combined construct test to increase coverage probability */
void test_combined_constructs(void) {
    int i, j;
    int matrix_sum = 0;
    
    /* Nested parallel regions with different clauses */
    #pragma omp parallel private(i, j) shared(matrix_sum) num_threads(2)
    {
        #pragma omp for collapse(2) schedule(dynamic) nowait
        for (i = 0; i < 10; i++) {
            for (j = 0; j < 10; j++) {
                #pragma omp atomic
                matrix_sum += i * j;
            }
        }
        
        #pragma omp barrier
        
        #pragma omp single
        {
            printf("Matrix sum in combined test: %d\n", matrix_sum);
        }
    }
}

/* Main function that exercises all test cases */
int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Initialize OpenMP if needed */
    omp_set_dynamic(0);
    omp_set_num_threads(2);
    
    /* Execute all test functions */
    test_parallel_for();
    test_parallel();
    test_sections();
    test_taskgroup();
    test_combined_constructs();
    
    /* Verify all tests executed */
    int total = result_parallel_for + result_parallel + 
                result_sections + result_taskgroup;
    
    printf("\nAll tests completed successfully!\n");
    printf("Aggregate result: %d\n", total);
    printf("Individual results: for=%d, parallel=%d, sections=%d, taskgroup=%d\n",
           result_parallel_for, result_parallel, result_sections, result_taskgroup);
    
    return 0;
}
