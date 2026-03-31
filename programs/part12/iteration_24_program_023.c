/* test_omp_clauses.c
 * 
 * This program is designed to trigger GCC's tree pretty-printer
 * to output the textual names of specific OpenMP clauses:
 *   "for", "parallel", "sections", "taskgroup"
 * 
 * Compile with GCC dump flags to generate intermediate representations
 * that will cause the pretty-printer to traverse these OpenMP clause nodes:
 *   gcc -O1 -fopenmp -fdump-tree-all test_omp_clauses.c -o test_omp_clauses
 *   gcc -O2 -fopenmp -fdump-tree-omp test_omp_clauses.c -o test_omp_clauses
 */

#include <stdio.h>
#include <omp.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int prevent_optimization = 0;

/* Function 1: Tests OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int sum = 0;
    
    /* Pattern A: OMP_CLAUSE_FOR 
     * Using combined 'parallel for' directive with schedule clause
     */
    #pragma omp parallel for schedule(static) private(i) shared(sum) reduction(+:global_counter)
    for (i = 0; i < 100; i++) {
        sum += i;
        global_counter++;
    }
    
    printf("test_parallel_for: sum = %d, global_counter = %d\n", sum, global_counter);
    prevent_optimization += sum;
}

/* Function 2: Tests OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int local_var = 0;
    
    /* Pattern B: OMP_CLAUSE_PARALLEL 
     * Standalone parallel region with data-sharing clauses
     */
    #pragma omp parallel private(local_var) shared(global_counter) firstprivate(prevent_optimization)
    {
        local_var = omp_get_thread_num();
        #pragma omp atomic
        global_counter += local_var;
    }
    
    printf("test_parallel: global_counter = %d\n", global_counter);
}

/* Function 3: Tests OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section1_result = 0;
    int section2_result = 0;
    
    /* Pattern C: OMP_CLAUSE_SECTIONS 
     * Combined parallel sections directive
     */
    #pragma omp parallel sections private(section1_result, section2_result) \
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
    }
    
    printf("test_sections: section1 = %d, section2 = %d, total = %d\n", 
           section1_result, section2_result, section1_result + section2_result);
    prevent_optimization += section1_result + section2_result;
}

/* Function 4: Tests OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_sum = 0;
    
    /* Pattern D: OMP_CLAUSE_TASKGROUP 
     * Nested taskgroup inside parallel region with tasks
     */
    #pragma omp parallel master
    {
        #pragma omp taskgroup
        {
            #pragma omp task shared(task_sum) 
            {
                int local_sum = 0;
                for (int i = 0; i < 25; i++) {
                    local_sum += i;
                }
                #pragma omp atomic
                task_sum += local_sum;
            }
            
            #pragma omp task shared(task_sum)
            {
                int local_sum = 0;
                for (int i = 25; i < 50; i++) {
                    local_sum += i;
                }
                #pragma omp atomic
                task_sum += local_sum;
            }
        } /* end taskgroup */
        
        /* Additional task after taskgroup to ensure taskgroup is not optimized away */
        #pragma omp task shared(global_counter)
        {
            #pragma omp atomic
            global_counter += task_sum;
        }
    }
    
    printf("test_taskgroup: task_sum = %d\n", task_sum);
    prevent_optimization += task_sum;
}

/* Function 5: Tests nested combination of clauses */
void test_combined(void) {
    /* Combined test with multiple clause types in nested structure */
    #pragma omp parallel
    {
        #pragma omp for schedule(dynamic)
        for (int i = 0; i < 10; i++) {
            #pragma omp atomic
            prevent_optimization += i;
        }
        
        #pragma omp sections
        {
            #pragma omp section
            {
                #pragma omp taskgroup
                {
                    #pragma omp task
                    {
                        #pragma omp atomic
                        global_counter++;
                    }
                }
            }
            
            #pragma omp section
            {
                #pragma omp atomic
                global_counter += 2;
            }
        }
    }
    
    printf("test_combined: prevent_optimization = %d, global_counter = %d\n", 
           prevent_optimization, global_counter);
}

int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Reset counters */
    global_counter = 0;
    prevent_optimization = 0;
    
    /* Execute all test functions */
    test_parallel_for();
    test_parallel();
    test_sections();
    test_taskgroup();
    test_combined();
    
    /* Final verification */
    printf("\nAll tests completed.\n");
    printf("Final global_counter = %d\n", global_counter);
    printf("Final prevent_optimization = %d\n", prevent_optimization);
    
    if (global_counter > 0 && prevent_optimization > 0) {
        printf("SUCCESS: All code paths executed.\n");
        return 0;
    } else {
        printf("WARNING: Some computations may have been optimized out.\n");
        return 1;
    }
}
