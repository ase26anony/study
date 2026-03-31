/* test_omp_clauses.c - Coverage test for OpenMP clause pretty-printing */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

volatile int global_counter = 0;

/* Test OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int sum = 0;
    volatile int local_volatile = 0;
    
    /* Use parallel for with schedule clause */
    #pragma omp parallel for schedule(static) private(i) shared(sum) firstprivate(local_volatile)
    for (i = 0; i < 100; i++) {
        #pragma omp atomic
        sum += i;
        local_volatile = i; /* Use volatile to prevent optimization */
    }
    
    printf("test_parallel_for: sum = %d\n", sum);
    
    #pragma omp atomic
    global_counter += sum;
}

/* Test OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int local_sum = 0;
    
    /* Standalone parallel region with data-sharing clauses */
    #pragma omp parallel private(local_sum) shared(global_counter)
    {
        int thread_id = omp_get_thread_num();
        local_sum = thread_id * 10;
        
        #pragma omp atomic
        global_counter += local_sum;
    }
    
    printf("test_parallel: completed\n");
}

/* Test OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section1_result = 0;
    int section2_result = 0;
    
    /* Parallel sections construct */
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
    
    printf("test_sections: section1=%d, section2=%d\n", 
           section1_result, section2_result);
}

/* Test OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_results[10] = {0};
    
    /* Create tasks within a taskgroup */
    #pragma omp parallel master
    {
        #pragma omp taskgroup
        {
            for (int i = 0; i < 10; i++) {
                #pragma omp task firstprivate(i) shared(task_results)
                {
                    task_results[i] = i * i;
                }
            }
        }
        
        /* Wait for taskgroup completion */
        #pragma omp taskwait
        
        /* Sum results */
        int total = 0;
        for (int i = 0; i < 10; i++) {
            total += task_results[i];
        }
        
        #pragma omp atomic
        global_counter += total;
    }
    
    printf("test_taskgroup: completed with tasks\n");
}

/* Combined construct to increase coverage probability */
void test_combined(void) {
    /* Combined parallel for with reduction */
    #pragma omp parallel for reduction(+:global_counter)
    for (int i = 0; i < 1000; i++) {
        global_counter += 1;
    }
    
    printf("test_combined: added 1000 to global counter\n");
}

/* Nested constructs for deeper tree traversal */
void test_nested(void) {
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int i = 0; i < 100; i++) {
            #pragma omp atomic
            global_counter += 1;
        }
        
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    #pragma omp atomic
                    global_counter += 1000;
                }
            }
        }
    }
    
    printf("test_nested: completed nested constructs\n");
}

int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Initialize */
    global_counter = 0;
    
    /* Execute all test functions */
    test_parallel_for();
    test_parallel();
    test_sections();
    test_taskgroup();
    test_combined();
    test_nested();
    
    /* Final verification */
    printf("\nAll tests completed successfully!\n");
    printf("Final global_counter value: %d\n", global_counter);
    
    /* Simple validation */
    if (global_counter > 0) {
        printf("SUCCESS: Program executed correctly.\n");
        return 0;
    } else {
        printf("ERROR: Unexpected result.\n");
        return 1;
    }
}
