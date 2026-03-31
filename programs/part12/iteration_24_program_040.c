/* test_omp_clauses.c - Test program for OpenMP clause coverage in GCC pretty-printer */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int result_parallel_for = 0;
volatile int result_parallel = 0;
volatile int result_sections = 0;
volatile int result_taskgroup = 0;

/* Test OMP_CLAUSE_FOR - Pattern A */
void test_parallel_for(void) {
    int i;
    int local_sum = 0;
    
    /* Use combined parallel for with schedule clause */
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

/* Test OMP_CLAUSE_PARALLEL - Pattern B */
void test_parallel(void) {
    int thread_id;
    int local_data = 0;
    
    /* Standalone parallel region with data-sharing clauses */
    #pragma omp parallel private(thread_id, local_data) shared(global_counter) \
        default(none)
    {
        thread_id = omp_get_thread_num();
        local_data = thread_id * 10;
        
        #pragma omp critical
        {
            global_counter += local_data;
        }
    }
    
    result_parallel = global_counter;
    printf("test_parallel completed: global_counter = %d\n", global_counter);
}

/* Test OMP_CLAUSE_SECTIONS - Pattern C */
void test_sections(void) {
    int section1_result = 0;
    int section2_result = 0;
    int section3_result = 0;
    
    /* Combined parallel sections construct */
    #pragma omp parallel sections private(section1_result, section2_result, section3_result) \
        shared(global_counter) firstprivate(result_sections)
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
    
    result_sections = section1_result + section2_result + section3_result;
    printf("test_sections completed: sum = %d, global_counter = %d\n", 
           result_sections, global_counter);
}

/* Test OMP_CLAUSE_TASKGROUP - Pattern D */
void test_taskgroup(void) {
    int task_sum = 0;
    
    /* Nested construct: parallel region with master that creates tasks */
    #pragma omp parallel master
    {
        /* Taskgroup clause inside parallel master region */
        #pragma omp taskgroup
        {
            for (int i = 0; i < 10; i++) {
                #pragma omp task firstprivate(i) shared(task_sum, global_counter) \
                    mergeable
                {
                    int task_result = i * i;
                    #pragma omp atomic
                    task_sum += task_result;
                    
                    #pragma omp atomic
                    global_counter++;
                }
            }
        }
        
        /* Additional task after taskgroup */
        #pragma omp task shared(task_sum)
        {
            task_sum += 1000;
        }
        
        #pragma omp taskwait
    }
    
    result_taskgroup = task_sum;
    printf("test_taskgroup completed: task_sum = %d, global_counter = %d\n", 
           task_sum, global_counter);
}

/* Additional test with nested clauses for better coverage */
void test_nested_constructs(void) {
    int i, j;
    int matrix_sum = 0;
    
    /* Nested parallel for loops */
    #pragma omp parallel for private(i, j) shared(matrix_sum) collapse(2) \
        schedule(dynamic)
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            #pragma omp atomic
            matrix_sum += i * j;
        }
    }
    
    printf("test_nested_constructs completed: matrix_sum = %d\n", matrix_sum);
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
    
    global_counter = 0;  /* Reset for next test */
    test_sections();
    
    global_counter = 0;  /* Reset for next test */
    test_taskgroup();
    
    /* Test nested constructs */
    test_nested_constructs();
    
    /* Verify all functions executed */
    printf("\nAll tests completed successfully!\n");
    printf("Final verification:\n");
    printf("  result_parallel_for = %d\n", result_parallel_for);
    printf("  result_parallel = %d\n", result_parallel);
    printf("  result_sections = %d\n", result_sections);
    printf("  result_taskgroup = %d\n", result_taskgroup);
    
    /* Simple validation */
    if (result_parallel_for == 4950 &&  /* Sum of 0-99 */
        result_sections == 11175 &&     /* Sum of 0-149 */
        result_taskgroup >= 285) {      /* Sum of squares 0-9 plus 1000 */
        printf("\nSUCCESS: All OpenMP constructs executed correctly!\n");
        return 0;
    } else {
        printf("\nWARNING: Some results may not match expected values\n");
        return 1;
    }
}
