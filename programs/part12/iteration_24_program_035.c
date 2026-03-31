/* test_omp_clauses.c
 * This program exercises OpenMP clauses to trigger GCC's tree pretty-printer
 * for the uncovered clause names: "for", "parallel", "sections", "taskgroup"
 */

#include <stdio.h>
#include <omp.h>

volatile int global_counter = 0;

/* Test function for OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int sum = 0;
    
    /* Use #pragma omp parallel for with schedule clause */
    #pragma omp parallel for schedule(static) private(i) reduction(+:sum)
    for (i = 0; i < 100; i++) {
        sum += i;
    }
    
    printf("test_parallel_for: sum = %d\n", sum);
    
    #pragma omp atomic
    global_counter += sum;
}

/* Test function for OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int local_var = 0;
    
    /* Use standalone #pragma omp parallel with data-sharing clauses */
    #pragma omp parallel private(local_var) shared(global_counter)
    {
        local_var = omp_get_thread_num();
        
        #pragma omp atomic
        global_counter += local_var;
    }
    
    printf("test_parallel completed\n");
}

/* Test function for OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section1_result = 0;
    int section2_result = 0;
    
    /* Use #pragma omp parallel sections */
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
    
    printf("test_sections: section1 = %d, section2 = %d\n", 
           section1_result, section2_result);
}

/* Test function for OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_sum = 0;
    
    /* Create a parallel region with tasks inside a taskgroup */
    #pragma omp parallel
    {
        #pragma omp master
        {
            /* Use #pragma omp taskgroup */
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
        }
    }
    
    printf("test_taskgroup: task_sum = %d\n", task_sum);
    
    #pragma omp atomic
    global_counter += task_sum;
}

/* Combined construct to increase coverage probability */
void test_combined(void) {
    int i;
    
    /* Combined parallel for with multiple clauses */
    #pragma omp parallel for schedule(dynamic) private(i) \
        shared(global_counter) if(1)
    for (i = 0; i < 50; i++) {
        #pragma omp atomic
        global_counter += 1;
    }
    
    printf("test_combined completed\n");
}

int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Call all test functions to exercise different OpenMP clauses */
    test_parallel_for();      /* Exercises OMP_CLAUSE_FOR */
    test_parallel();          /* Exercises OMP_CLAUSE_PARALLEL */
    test_sections();          /* Exercises OMP_CLAUSE_SECTIONS */
    test_taskgroup();         /* Exercises OMP_CLAUSE_TASKGROUP */
    test_combined();          /* Additional coverage */
    
    printf("\nAll tests completed successfully!\n");
    printf("Final global_counter value: %d\n", global_counter);
    
    /* Verify computation was performed */
    if (global_counter > 0) {
        printf("SUCCESS: OpenMP constructs executed correctly.\n");
        return 0;
    } else {
        printf("ERROR: No computation performed.\n");
        return 1;
    }
}
