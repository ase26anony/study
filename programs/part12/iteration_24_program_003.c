/* test_omp_clauses.c - Test program for GCC OpenMP clause coverage */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

volatile int global_counter = 0;
volatile int dummy = 0;

/* Function 1: Tests OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int sum = 0;
    int private_var = 10;
    
    /* Use parallel for with schedule clause */
    #pragma omp parallel for schedule(static) private(private_var) reduction(+:sum)
    for (i = 0; i < 100; i++) {
        private_var = i;
        sum += private_var;
        dummy = i;  /* Prevent optimization */
    }
    
    printf("test_parallel_for: sum = %d\n", sum);
    #pragma omp atomic
    global_counter += sum;
}

/* Function 2: Tests OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int local_sum = 0;
    
    /* Basic parallel region with data-sharing clauses */
    #pragma omp parallel private(local_sum) shared(global_counter)
    {
        int thread_id = omp_get_thread_num();
        local_sum = thread_id * 10;
        
        #pragma omp atomic
        global_counter += local_sum;
        
        dummy = thread_id;  /* Prevent optimization */
    }
    
    printf("test_parallel: executed with %d threads\n", 
           omp_get_max_threads());
}

/* Function 3: Tests OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section1_result = 0;
    int section2_result = 0;
    int section3_result = 0;
    
    /* Parallel sections with different work in each section */
    #pragma omp parallel sections private(dummy) \
            firstprivate(section1_result, section2_result, section3_result)
    {
        #pragma omp section
        {
            for (int i = 0; i < 50; i++) {
                section1_result += i;
            }
            #pragma omp atomic
            global_counter += section1_result;
            printf("Section 1 completed: %d\n", section1_result);
        }
        
        #pragma omp section
        {
            for (int i = 50; i < 100; i++) {
                section2_result += i;
            }
            #pragma omp atomic
            global_counter += section2_result;
            printf("Section 2 completed: %d\n", section2_result);
        }
        
        #pragma omp section
        {
            for (int i = 100; i < 150; i++) {
                section3_result += i;
            }
            #pragma omp atomic
            global_counter += section3_result;
            printf("Section 3 completed: %d\n", section3_result);
        }
    }
}

/* Function 4: Tests OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_sum = 0;
    
    /* Create tasks within a taskgroup */
    #pragma omp parallel master
    {
        #pragma omp taskgroup
        {
            for (int i = 0; i < 10; i++) {
                #pragma omp task firstprivate(i) shared(task_sum)
                {
                    int task_result = i * i;
                    #pragma omp atomic
                    task_sum += task_result;
                    dummy = i;  /* Prevent optimization */
                }
            }
        }
        
        /* Wait for all tasks in taskgroup */
        #pragma omp taskwait
        
        printf("Taskgroup completed: task_sum = %d\n", task_sum);
        #pragma omp atomic
        global_counter += task_sum;
    }
}

/* Function 5: Combined constructs for additional coverage */
void test_combined(void) {
    int i, j;
    int matrix_sum = 0;
    
    /* Nested parallelism with for clauses */
    #pragma omp parallel for collapse(2) private(j) reduction(+:matrix_sum)
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            matrix_sum += i * j;
            dummy = i + j;  /* Prevent optimization */
        }
    }
    
    printf("test_combined (collapsed loops): matrix_sum = %d\n", matrix_sum);
    #pragma omp atomic
    global_counter += matrix_sum;
}

int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    printf("Initial global_counter = %d\n", global_counter);
    
    /* Set number of threads for reproducibility */
    omp_set_num_threads(4);
    
    /* Execute all test functions */
    test_parallel_for();
    test_parallel();
    test_sections();
    test_taskgroup();
    test_combined();
    
    printf("\nAll tests completed successfully!\n");
    printf("Final global_counter = %d\n", global_counter);
    
    /* Verify computation */
    if (global_counter > 0) {
        printf("SUCCESS: OpenMP constructs executed and contributed to result.\n");
        return 0;
    } else {
        printf("ERROR: No computation performed.\n");
        return 1;
    }
}
