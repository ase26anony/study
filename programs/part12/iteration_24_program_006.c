/* test_omp_clauses.c
 * This program exercises specific OpenMP clauses to trigger
 * pretty-printer coverage for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP.
 * Compile with: gcc -O1 -fopenmp -fdump-tree-all test_omp_clauses.c -o test_omp_clauses
 */

#include <stdio.h>
#include <omp.h>

volatile int global_counter = 0;

/* Test OMP_CLAUSE_FOR clause */
void test_parallel_for(void) {
    int i;
    int sum = 0;
    volatile int local_volatile = 42;  /* Prevent optimization */
    
    /* Pattern A: OMP_CLAUSE_FOR - using parallel for with schedule clause */
    #pragma omp parallel for private(i) shared(sum, local_volatile) schedule(static)
    for (i = 0; i < 100; i++) {
        #pragma omp atomic
        sum += i + local_volatile;
    }
    
    printf("test_parallel_for: sum = %d\n", sum);
    #pragma omp atomic
    global_counter += sum;
}

/* Test OMP_CLAUSE_PARALLEL clause */
void test_parallel(void) {
    int local_sum = 0;
    
    /* Pattern B: OMP_CLAUSE_PARALLEL - standalone parallel region */
    #pragma omp parallel private(local_sum) shared(global_counter)
    {
        local_sum = omp_get_thread_num();
        #pragma omp atomic
        global_counter += local_sum;
    }
    
    printf("test_parallel: executed with %d threads\n", omp_get_max_threads());
}

/* Test OMP_CLAUSE_SECTIONS clause */
void test_sections(void) {
    int section1_result = 0;
    int section2_result = 0;
    
    /* Pattern C: OMP_CLAUSE_SECTIONS - parallel sections construct */
    #pragma omp parallel sections private(section1_result, section2_result) \
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
    }
    
    printf("test_sections: section results = %d, %d\n", 
           section1_result, section2_result);
}

/* Test OMP_CLAUSE_TASKGROUP clause */
void test_taskgroup(void) {
    int task_sum = 0;
    
    /* Pattern D: OMP_CLAUSE_TASKGROUP - taskgroup inside parallel region */
    #pragma omp parallel master
    {
        #pragma omp taskgroup
        {
            for (int i = 0; i < 10; i++) {
                #pragma omp task firstprivate(i) shared(task_sum)
                {
                    int local = i * i;
                    #pragma omp atomic
                    task_sum += local;
                }
            }
        }
        
        /* Ensure all tasks complete */
        #pragma omp taskwait
        
        #pragma omp atomic
        global_counter += task_sum;
    }
    
    printf("test_taskgroup: task_sum = %d\n", task_sum);
}

/* Combined construct to increase coverage probability */
void test_combined(void) {
    int i, j;
    volatile int seed = 123;
    
    /* Combined parallel for with multiple clauses */
    #pragma omp parallel for private(i, j) firstprivate(seed) \
                schedule(dynamic, 4) collapse(2)
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            #pragma omp atomic
            global_counter += i * j + seed;
        }
    }
    
    printf("test_combined: executed combined parallel for\n");
}

int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Set threads for reproducibility */
    omp_set_num_threads(4);
    
    /* Execute all test functions */
    test_parallel_for();
    test_parallel();
    test_sections();
    test_taskgroup();
    test_combined();
    
    printf("\nAll tests completed successfully!\n");
    printf("Final global_counter value: %d\n", global_counter);
    
    /* Verify computation was performed */
    if (global_counter > 0) {
        printf("SUCCESS: All OpenMP constructs executed.\n");
        return 0;
    } else {
        printf("ERROR: No computation performed.\n");
        return 1;
    }
}
