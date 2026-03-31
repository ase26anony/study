/* test_omp_clauses.c
 * This program exercises specific OpenMP clauses to trigger their
 * pretty-printing in GCC's tree pretty-printer (tree-pretty-print.cc).
 * Compile with: gcc -O1 -fopenmp -fdump-tree-all test_omp_clauses.c -o test_omp_clauses
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

volatile int global_counter = 0;

/* Test OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int sum = 0;
    volatile int local_volatile = 0;
    
    /* Direct usage of 'for' clause in combined parallel for construct */
    #pragma omp parallel for schedule(static) private(i) shared(sum, local_volatile) \
        reduction(+:global_counter)
    for (i = 0; i < 100; i++) {
        sum += i;
        local_volatile = i;  /* Prevent optimization */
        global_counter++;
    }
    
    printf("test_parallel_for: sum = %d, global_counter = %d\n", sum, global_counter);
}

/* Test OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int local_sum = 0;
    volatile int barrier_var = 0;
    
    /* Standalone parallel region with various data-sharing clauses */
    #pragma omp parallel private(local_sum) shared(barrier_var) \
        firstprivate(global_counter) reduction(+:global_counter)
    {
        int thread_id = omp_get_thread_num();
        local_sum = thread_id * 10;
        barrier_var = thread_id;  /* Ensure construct isn't optimized away */
        
        #pragma omp critical
        {
            global_counter += local_sum;
        }
    }
    
    printf("test_parallel: global_counter = %d\n", global_counter);
}

/* Test OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section1_result = 0, section2_result = 0, section3_result = 0;
    volatile int dummy = 0;
    
    /* Combined parallel sections construct */
    #pragma omp parallel sections private(dummy) \
        shared(section1_result, section2_result, section3_result) \
        reduction(+:global_counter)
    {
        #pragma omp section
        {
            for (int i = 0; i < 50; i++) {
                section1_result += i;
                dummy = i;
            }
            global_counter += section1_result;
        }
        
        #pragma omp section
        {
            for (int i = 50; i < 100; i++) {
                section2_result += i;
                dummy = i;
            }
            global_counter += section2_result;
        }
        
        #pragma omp section
        {
            for (int i = 100; i < 150; i++) {
                section3_result += i;
                dummy = i;
            }
            global_counter += section3_result;
        }
    }
    
    printf("test_sections: results = %d, %d, %d, global_counter = %d\n",
           section1_result, section2_result, section3_result, global_counter);
}

/* Test OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_results[10] = {0};
    volatile int sync_var = 0;
    
    /* Create a parallel region with tasks inside a taskgroup */
    #pragma omp parallel shared(task_results, sync_var)
    {
        #pragma omp master
        {
            /* Taskgroup clause should be triggered here */
            #pragma omp taskgroup
            {
                for (int i = 0; i < 10; i++) {
                    #pragma omp task firstprivate(i) shared(task_results)
                    {
                        task_results[i] = i * i;
                        sync_var = i;  /* Prevent optimization */
                    }
                }
            }
            
            /* Additional task after taskgroup for contrast */
            #pragma omp task shared(task_results)
            {
                task_results[0] += 100;
            }
        }
    }
    
    /* Verify task execution */
    int total = 0;
    for (int i = 0; i < 10; i++) {
        total += task_results[i];
    }
    printf("test_taskgroup: total = %d\n", total);
}

/* Combined test with nested constructs */
void test_combined(void) {
    volatile int control = 0;
    
    /* Nested: parallel region containing for loop */
    #pragma omp parallel shared(control)
    {
        #pragma omp for schedule(dynamic) nowait
        for (int i = 0; i < 20; i++) {
            control += i;
        }
        
        #pragma omp single
        {
            /* Taskgroup inside single region */
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    control += 1000;
                }
            }
        }
    }
    
    printf("test_combined: control = %d\n", control);
}

int main(void) {
    printf("=== Testing OpenMP Clause Pretty Printing ===\n");
    
    /* Reset global counter */
    global_counter = 0;
    
    /* Execute all test functions */
    test_parallel_for();
    test_parallel();
    test_sections();
    test_taskgroup();
    test_combined();
    
    /* Final verification */
    printf("\n=== All tests completed ===\n");
    printf("Final global_counter = %d\n", global_counter);
    
    return 0;
}
