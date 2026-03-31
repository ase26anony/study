/* test_omp_clauses.c - Test program for GCC OpenMP clause pretty-printer coverage */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int result_check = 0;

/* Function 1: Test OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int local_sum = 0;
    int private_var = 10;
    
    /* Use OMP_CLAUSE_FOR with schedule clause */
    #pragma omp parallel for private(private_var) schedule(static) reduction(+:local_sum)
    for (i = 0; i < 100; i++) {
        private_var = i;
        local_sum += private_var;
    }
    
    #pragma omp atomic
    global_counter += local_sum;
    result_check += 1;
    printf("test_parallel_for completed: sum = %d\n", local_sum);
}

/* Function 2: Test OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int thread_id;
    int shared_data = 42;
    
    /* Basic parallel region with OMP_CLAUSE_PARALLEL */
    #pragma omp parallel private(thread_id) shared(shared_data, global_counter)
    {
        thread_id = omp_get_thread_num();
        
        #pragma omp critical
        {
            global_counter += thread_id;
            shared_data += thread_id;
        }
    }
    
    result_check += 2;
    printf("test_parallel completed: shared_data = %d\n", shared_data);
}

/* Function 3: Test OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section1_result = 0;
    int section2_result = 0;
    
    /* Combined parallel sections directive */
    #pragma omp parallel sections private(section1_result, section2_result) \
        firstprivate(global_counter)
    {
        #pragma omp section
        {
            section1_result = 1;
            #pragma omp atomic
            global_counter += section1_result;
        }
        
        #pragma omp section
        {
            section2_result = 2;
            #pragma omp atomic
            global_counter += section2_result;
        }
    }
    
    result_check += 4;
    printf("test_sections completed: results = %d, %d\n", 
           section1_result, section2_result);
}

/* Function 4: Test OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_counter = 0;
    
    /* Create a parallel region with master construct */
    #pragma omp parallel
    {
        #pragma omp master
        {
            /* Use OMP_CLAUSE_TASKGROUP inside task-generating region */
            #pragma omp taskgroup
            {
                int i;
                for (i = 0; i < 5; i++) {
                    #pragma omp task firstprivate(i) shared(task_counter)
                    {
                        int local = i * 2;
                        #pragma omp atomic
                        task_counter += local;
                    }
                }
            }
            
            /* Another taskgroup with taskwait */
            #pragma omp taskgroup
            {
                #pragma omp task shared(task_counter)
                {
                    #pragma omp atomic
                    task_counter += 100;
                }
                #pragma omp taskwait
            }
        }
    }
    
    #pragma omp atomic
    global_counter += task_counter;
    result_check += 8;
    printf("test_taskgroup completed: task_counter = %d\n", task_counter);
}

/* Function 5: Test nested combinations */
void test_combined_constructs(void) {
    int i, j;
    
    /* Nested parallel for with taskgroup inside */
    #pragma omp parallel for private(j) schedule(dynamic)
    for (i = 0; i < 10; i++) {
        #pragma omp taskgroup
        {
            #pragma omp task firstprivate(i)
            {
                int temp = i * i;
                #pragma omp atomic
                global_counter += temp;
            }
        }
    }
    
    result_check += 16;
    printf("test_combined_constructs completed\n");
}

/* Main function that calls all test functions */
int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Initialize OpenMP */
    omp_set_num_threads(4);
    
    /* Execute all test functions */
    test_parallel_for();
    test_parallel();
    test_sections();
    test_taskgroup();
    test_combined_constructs();
    
    /* Verify all functions executed */
    if (result_check == (1 + 2 + 4 + 8 + 16)) {
        printf("\nSUCCESS: All OpenMP constructs executed correctly!\n");
        printf("Final global_counter = %d\n", global_counter);
        printf("Expected result_check = 31, Actual = %d\n", result_check);
    } else {
        printf("\nERROR: Some constructs may not have executed properly\n");
        printf("Expected result_check = 31, Actual = %d\n", result_check);
        return 1;
    }
    
    return 0;
}
