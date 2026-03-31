/* test_omp_clauses.c - Test program for OpenMP clause pretty-printer coverage */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_sum = 0;

/* Test OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int local_sum = 0;
    
    /* Use combined parallel for with schedule clause */
    #pragma omp parallel for private(i) shared(local_sum) schedule(static, 4)
    for (i = 0; i < 100; i++) {
        #pragma omp atomic
        local_sum += i;
    }
    
    #pragma omp atomic
    global_sum += local_sum;
    
    printf("test_parallel_for completed: local_sum = %d\n", local_sum);
}

/* Test OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int thread_id;
    int local_counter = 0;
    
    /* Standalone parallel region with data-sharing clauses */
    #pragma omp parallel private(thread_id) firstprivate(local_counter) shared(global_counter)
    {
        thread_id = omp_get_thread_num();
        local_counter = thread_id * 10;
        
        #pragma omp atomic
        global_counter += local_counter;
        
        #pragma omp critical
        {
            printf("Thread %d: local_counter = %d\n", thread_id, local_counter);
        }
    }
    
    printf("test_parallel completed: global_counter = %d\n", global_counter);
}

/* Test OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section1_result = 0;
    int section2_result = 0;
    int section3_result = 0;
    
    /* Combined parallel sections construct */
    #pragma omp parallel sections private(section1_result, section2_result, section3_result) \
                shared(global_sum)
    {
        #pragma omp section
        {
            for (int i = 0; i < 50; i++) {
                section1_result += i;
            }
            #pragma omp atomic
            global_sum += section1_result;
            printf("Section 1: result = %d\n", section1_result);
        }
        
        #pragma omp section
        {
            for (int i = 50; i < 100; i++) {
                section2_result += i;
            }
            #pragma omp atomic
            global_sum += section2_result;
            printf("Section 2: result = %d\n", section2_result);
        }
        
        #pragma omp section
        {
            for (int i = 100; i < 150; i++) {
                section3_result += i;
            }
            #pragma omp atomic
            global_sum += section3_result;
            printf("Section 3: result = %d\n", section3_result);
        }
    }
    
    printf("test_sections completed\n");
}

/* Test OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_sum = 0;
    
    /* Nested structure: parallel -> single -> taskgroup -> tasks */
    #pragma omp parallel shared(task_sum)
    {
        #pragma omp single
        {
            printf("Master thread creating tasks with taskgroup...\n");
            
            /* Taskgroup clause */
            #pragma omp taskgroup
            {
                for (int i = 0; i < 10; i++) {
                    #pragma omp task firstprivate(i) shared(task_sum)
                    {
                        int task_result = i * i;
                        #pragma omp atomic
                        task_sum += task_result;
                        printf("Task %d: computed %d\n", i, task_result);
                    }
                }
                
                /* Wait for all tasks in the group */
                #pragma omp taskwait
            }
        }
    }
    
    #pragma omp atomic
    global_sum += task_sum;
    
    printf("test_taskgroup completed: task_sum = %d\n", task_sum);
}

/* Combined test with nested clauses */
void test_combined(void) {
    int i, j;
    int matrix_sum = 0;
    
    /* Nested parallelism: outer parallel, inner for */
    #pragma omp parallel private(i, j) shared(matrix_sum)
    {
        #pragma omp for collapse(2) schedule(dynamic)
        for (i = 0; i < 10; i++) {
            for (j = 0; j < 10; j++) {
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
    global_sum += matrix_sum;
    
    printf("test_combined completed: matrix_sum = %d\n", matrix_sum);
}

int main(void) {
    printf("=== OpenMP Clause Coverage Test Program ===\n");
    
    /* Reset globals */
    global_counter = 0;
    global_sum = 0;
    
    /* Execute all test functions */
    test_parallel();
    test_parallel_for();
    test_sections();
    test_taskgroup();
    test_combined();
    
    /* Final verification */
    printf("\n=== Final Results ===\n");
    printf("global_counter = %d\n", global_counter);
    printf("global_sum = %d\n", global_sum);
    
    if (global_sum > 0) {
        printf("All tests completed successfully!\n");
        return 0;
    } else {
        printf("Error: No computation performed!\n");
        return 1;
    }
}
