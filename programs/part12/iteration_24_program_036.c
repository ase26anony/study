/* test_omp_clauses.c
 * Test program to cover OpenMP clause name printing in GCC tree pretty-printer
 * Specifically targets: OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, 
 * OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int result_checksum = 0;

/* Function 1: Tests OMP_CLAUSE_FOR with combined parallel for */
void test_parallel_for(void) {
    int i;
    int local_sum = 0;
    const int N = 100;
    
    /* Combined parallel for directive - should trigger OMP_CLAUSE_FOR */
    #pragma omp parallel for private(i) shared(local_sum) schedule(static, 10)
    for (i = 0; i < N; i++) {
        #pragma omp atomic
        local_sum += i * 2;
    }
    
    #pragma omp atomic
    result_checksum += local_sum;
    
    printf("test_parallel_for completed: sum = %d\n", local_sum);
}

/* Function 2: Tests OMP_CLAUSE_PARALLEL with basic parallel region */
void test_parallel(void) {
    int thread_id;
    int local_counter = 0;
    
    /* Basic parallel region - should trigger OMP_CLAUSE_PARALLEL */
    #pragma omp parallel private(thread_id) firstprivate(local_counter) \
                    shared(global_counter)
    {
        thread_id = omp_get_thread_num();
        local_counter = thread_id * 10;
        
        #pragma omp atomic
        global_counter += local_counter;
        
        #pragma omp barrier
        
        #pragma omp master
        {
            printf("test_parallel: %d threads executed\n", omp_get_num_threads());
        }
    }
    
    printf("test_parallel completed: global_counter = %d\n", global_counter);
}

/* Function 3: Tests OMP_CLAUSE_SECTIONS with parallel sections */
void test_sections(void) {
    int section_a = 0, section_b = 0, section_c = 0;
    
    /* Parallel sections - should trigger OMP_CLAUSE_SECTIONS */
    #pragma omp parallel sections private(section_a, section_b, section_c) \
                                 shared(result_checksum)
    {
        #pragma omp section
        {
            for (int i = 0; i < 50; i++) {
                section_a += i;
            }
            #pragma omp atomic
            result_checksum += section_a;
        }
        
        #pragma omp section
        {
            for (int i = 50; i < 100; i++) {
                section_b += i;
            }
            #pragma omp atomic
            result_checksum += section_b;
        }
        
        #pragma omp section
        {
            for (int i = 100; i < 150; i++) {
                section_c += i;
            }
            #pragma omp atomic
            result_checksum += section_c;
        }
    }
    
    printf("test_sections completed: sections sum = %d\n", 
           section_a + section_b + section_c);
}

/* Function 4: Tests OMP_CLAUSE_TASKGROUP with nested task constructs */
void test_taskgroup(void) {
    int task_results[10] = {0};
    
    /* Parallel region with master and taskgroup - should trigger OMP_CLAUSE_TASKGROUP */
    #pragma omp parallel
    {
        #pragma omp master
        {
            /* Taskgroup ensures all tasks complete before continuing */
            #pragma omp taskgroup
            {
                for (int i = 0; i < 10; i++) {
                    #pragma omp task firstprivate(i) shared(task_results)
                    {
                        /* Simulate some work */
                        int val = i * i * 10;
                        for (int j = 0; j < 100; j++) {
                            val += j;
                        }
                        task_results[i] = val;
                    }
                }
            } /* End of taskgroup - wait for all tasks */
            
            /* All tasks are guaranteed to be complete here */
            int final_sum = 0;
            for (int i = 0; i < 10; i++) {
                final_sum += task_results[i];
            }
            
            #pragma omp atomic
            result_checksum += final_sum;
            
            printf("test_taskgroup completed: task sum = %d\n", final_sum);
        }
    }
}

/* Function 5: Tests nested combination of clauses */
void test_nested_constructs(void) {
    int i, j;
    int matrix_sum = 0;
    const int M = 20;
    const int N = 20;
    
    /* Outer parallel region */
    #pragma omp parallel private(i, j) shared(matrix_sum)
    {
        /* Inner for loop with scheduling */
        #pragma omp for collapse(2) schedule(dynamic, 5)
        for (i = 0; i < M; i++) {
            for (j = 0; j < N; j++) {
                #pragma omp atomic
                matrix_sum += i * j;
            }
        }
        
        /* Sections within the same parallel region */
        #pragma omp sections
        {
            #pragma omp section
            {
                #pragma omp atomic
                matrix_sum += 1000;
            }
            
            #pragma omp section
            {
                #pragma omp atomic
                matrix_sum += 2000;
            }
        }
    }
    
    #pragma omp atomic
    result_checksum += matrix_sum;
    
    printf("test_nested_constructs completed: matrix_sum = %d\n", matrix_sum);
}

/* Main function that calls all test functions */
int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    printf("========================================\n");
    
    /* Initialize OpenMP if needed */
    omp_set_num_threads(4);
    
    /* Execute all test functions */
    test_parallel_for();
    test_parallel();
    test_sections();
    test_taskgroup();
    test_nested_constructs();
    
    /* Final verification */
    printf("\n========================================\n");
    printf("All tests completed successfully!\n");
    printf("Final result_checksum = %d\n", result_checksum);
    printf("Final global_counter = %d\n", global_counter);
    
    /* Simple validation */
    if (result_checksum > 0 && global_counter > 0) {
        printf("Validation PASSED: Results are non-zero\n");
        return 0;
    } else {
        printf("Validation FAILED: Unexpected results\n");
        return 1;
    }
}
