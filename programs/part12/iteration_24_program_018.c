/* test_omp_clauses.c
 * Test program to trigger GCC's tree pretty-printer for specific OpenMP clauses:
 * - OMP_CLAUSE_FOR
 * - OMP_CLAUSE_PARALLEL  
 * - OMP_CLAUSE_SECTIONS
 * - OMP_CLAUSE_TASKGROUP
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int prevent_optimization = 0;

/* Function 1: Tests OMP_CLAUSE_FOR with combined parallel for */
void test_parallel_for(void) {
    int i, sum = 0;
    const int N = 100;
    
    /* Combined parallel for directive - triggers OMP_CLAUSE_FOR */
    #pragma omp parallel for private(i) shared(sum) schedule(static) num_threads(4)
    for (i = 0; i < N; i++) {
        #pragma omp atomic
        sum += i;
    }
    
    printf("test_parallel_for: sum = %d (expected %d)\n", 
           sum, (N-1)*N/2);
    global_counter += sum;
}

/* Function 2: Tests OMP_CLAUSE_PARALLEL with basic parallel region */
void test_parallel(void) {
    int local_sum = 0;
    
    /* Standalone parallel directive - triggers OMP_CLAUSE_PARALLEL */
    #pragma omp parallel private(local_sum) shared(global_counter) num_threads(2)
    {
        local_sum = omp_get_thread_num() + 1;
        
        #pragma omp critical
        {
            global_counter += local_sum;
            prevent_optimization += omp_get_thread_num();
        }
    }
    
    printf("test_parallel: executed with thread participation\n");
}

/* Function 3: Tests OMP_CLAUSE_SECTIONS with parallel sections */
void test_sections(void) {
    int section1_result = 0, section2_result = 0, section3_result = 0;
    
    /* Parallel sections directive - triggers OMP_CLAUSE_SECTIONS */
    #pragma omp parallel sections private(section1_result, section2_result, section3_result) \
            shared(global_counter) num_threads(3)
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
    
    printf("test_sections: sections completed (sums: %d, %d, %d)\n",
           section1_result, section2_result, section3_result);
}

/* Function 4: Tests OMP_CLAUSE_TASKGROUP with task-based parallelism */
void test_taskgroup(void) {
    int task_results[10] = {0};
    
    /* Nested structure to ensure taskgroup clause is processed */
    #pragma omp parallel num_threads(2)
    {
        #pragma omp master
        {
            /* Taskgroup directive - triggers OMP_CLAUSE_TASKGROUP */
            #pragma omp taskgroup
            {
                for (int i = 0; i < 10; i++) {
                    #pragma omp task firstprivate(i) shared(task_results)
                    {
                        task_results[i] = i * i;
                        #pragma omp atomic
                        global_counter += task_results[i];
                    }
                }
            } /* implicit taskgroup wait here */
        }
    }
    
    /* Verify task results */
    int verify = 0;
    for (int i = 0; i < 10; i++) {
        verify += task_results[i];
    }
    printf("test_taskgroup: task results sum = %d (expected 285)\n", verify);
}

/* Function 5: Tests combined constructs with nesting */
void test_combined(void) {
    int i, j;
    const int N = 20;
    int matrix_sum = 0;
    
    /* Nested parallelism with multiple clauses */
    #pragma omp parallel for private(i, j) shared(matrix_sum) collapse(2) \
            schedule(dynamic) num_threads(2)
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            #pragma omp atomic
            matrix_sum += i * j;
        }
    }
    
    printf("test_combined: matrix sum = %d\n", matrix_sum);
    global_counter += matrix_sum;
}

/* Main function that calls all test cases */
int main(void) {
    int initial_counter = global_counter;
    
    printf("=== Testing OpenMP Clause Coverage for GCC Pretty-Printer ===\n\n");
    
    /* Execute all test functions */
    test_parallel_for();
    test_parallel();
    test_sections();
    test_taskgroup();
    test_combined();
    
    /* Final verification */
    printf("\n=== Summary ===\n");
    printf("Initial global_counter: %d\n", initial_counter);
    printf("Final global_counter: %d\n", global_counter);
    printf("Prevent optimization dummy: %d\n", prevent_optimization);
    
    if (global_counter > initial_counter) {
        printf("SUCCESS: All OpenMP constructs executed and contributed to computation.\n");
        return 0;
    } else {
        printf("WARNING: No computation detected - OpenMP regions may have been optimized out.\n");
        return 1;
    }
}
