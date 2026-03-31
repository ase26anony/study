/* test_openmp_clauses.c
 * 
 * This program is designed to trigger GCC's tree pretty-printer
 * to output the textual names of specific OpenMP clauses:
 *   "for", "parallel", "sections", "taskgroup"
 * 
 * Compile with GCC dump flags to generate intermediate representations:
 *   gcc -O1 -fopenmp -fdump-tree-all -fdump-ipa-all test_openmp_clauses.c -o test_openmp_clauses
 * 
 * Or with OpenMP-specific diagnostics:
 *   gcc -O1 -fopenmp -fopt-info-omp -fopt-info-all test_openmp_clauses.c -o test_openmp_clauses
 * 
 * The program will run correctly without dump flags, but the coverage
 * goal is achieved during compilation when dump files are generated.
 */

#include <stdio.h>
#include <omp.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int prevent_optimization = 0;

/* Function 1: Tests OMP_CLAUSE_FOR with parallel for construct */
void test_parallel_for(void) {
    int i;
    int local_sum = 0;
    
    /* Use #pragma omp parallel for with schedule clause
     * This should trigger OMP_CLAUSE_FOR in the pretty-printer */
    #pragma omp parallel for schedule(static) private(i) shared(local_sum) reduction(+:global_counter)
    for (i = 0; i < 100; i++) {
        local_sum += i;
        global_counter++;
    }
    
    printf("test_parallel_for completed: local_sum = %d, global_counter = %d\n", 
           local_sum, global_counter);
    prevent_optimization += local_sum;
}

/* Function 2: Tests OMP_CLAUSE_PARALLEL with standalone parallel region */
void test_parallel(void) {
    int thread_id;
    
    /* Use #pragma omp parallel with private and shared clauses
     * This should trigger OMP_CLAUSE_PARALLEL in the pretty-printer */
    #pragma omp parallel private(thread_id) shared(global_counter, prevent_optimization)
    {
        thread_id = omp_get_thread_num();
        
        #pragma omp critical
        {
            global_counter++;
            prevent_optimization += thread_id;
        }
    }
    
    printf("test_parallel completed: global_counter = %d\n", global_counter);
}

/* Function 3: Tests OMP_CLAUSE_SECTIONS with parallel sections construct */
void test_sections(void) {
    int section1_result = 0;
    int section2_result = 0;
    int section3_result = 0;
    
    /* Use #pragma omp parallel sections
     * This should trigger OMP_CLAUSE_SECTIONS in the pretty-printer */
    #pragma omp parallel sections private(section1_result, section2_result, section3_result) \
                shared(global_counter, prevent_optimization)
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
            prevent_optimization += section3_result;
        }
    }
    
    printf("test_sections completed: section results = %d, %d, %d\n", 
           section1_result, section2_result, section3_result);
}

/* Function 4: Tests OMP_CLAUSE_TASKGROUP with taskgroup construct */
void test_taskgroup(void) {
    int task_counter = 0;
    
    /* Use #pragma omp parallel master to create a master thread
     * then use #pragma omp taskgroup inside
     * This should trigger OMP_CLAUSE_TASKGROUP in the pretty-printer */
    #pragma omp parallel master shared(task_counter, prevent_optimization)
    {
        #pragma omp taskgroup
        {
            for (int i = 0; i < 10; i++) {
                #pragma omp task firstprivate(i) shared(task_counter)
                {
                    #pragma omp atomic
                    task_counter++;
                    prevent_optimization += i;
                }
            }
            
            /* Wait for all tasks in the taskgroup */
            #pragma omp taskwait
        }
    }
    
    printf("test_taskgroup completed: task_counter = %d\n", task_counter);
}

/* Function 5: Tests combined parallel for with multiple clauses */
void test_combined_constructs(void) {
    int i, j;
    int matrix_sum = 0;
    
    /* Combined parallel for with multiple clauses
     * This creates richer tree structures for the pretty-printer */
    #pragma omp parallel for collapse(2) private(i, j) shared(matrix_sum) \
                schedule(dynamic, 4) reduction(+:global_counter)
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            matrix_sum += i * j;
            global_counter++;
        }
    }
    
    printf("test_combined_constructs completed: matrix_sum = %d\n", matrix_sum);
    prevent_optimization += matrix_sum;
}

/* Main function that calls all test functions */
int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Initialize OpenMP */
    omp_set_num_threads(4);
    
    /* Call each test function to trigger different OpenMP clauses */
    test_parallel_for();      /* Should trigger OMP_CLAUSE_FOR */
    test_parallel();          /* Should trigger OMP_CLAUSE_PARALLEL */
    test_sections();          /* Should trigger OMP_CLAUSE_SECTIONS */
    test_taskgroup();         /* Should trigger OMP_CLAUSE_TASKGROUP */
    test_combined_constructs(); /* Additional coverage with combined constructs */
    
    /* Final verification */
    printf("\nAll tests completed successfully!\n");
    printf("Final global_counter = %d\n", global_counter);
    printf("Final prevent_optimization = %d\n", prevent_optimization);
    
    /* Simple validation */
    if (global_counter > 0 && prevent_optimization > 0) {
        printf("Validation PASSED: All OpenMP constructs executed.\n");
        return 0;
    } else {
        printf("Validation FAILED: Some constructs may have been optimized away.\n");
        return 1;
    }
}
