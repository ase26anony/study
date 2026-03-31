/* test_omp_clauses.c
 * Program to trigger GCC tree pretty-printer output for specific OpenMP clauses:
 * - OMP_CLAUSE_FOR
 * - OMP_CLAUSE_PARALLEL  
 * - OMP_CLAUSE_SECTIONS
 * - OMP_CLAUSE_TASKGROUP
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

volatile int global_counter = 0;

/* Test function for OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int sum = 0;
    volatile int local_sum = 0;  /* volatile to prevent optimization */
    
    /* Combined parallel for directive - triggers OMP_CLAUSE_FOR */
    #pragma omp parallel for private(i) shared(sum) schedule(static, 4) num_threads(2)
    for (i = 0; i < 100; i++) {
        #pragma omp atomic
        sum += i;
        local_sum = i;  /* Use volatile variable */
    }
    
    printf("test_parallel_for: sum = %d (expected 4950)\n", sum);
    global_counter += sum;
}

/* Test function for OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int local_var = 0;
    
    /* Standalone parallel region - triggers OMP_CLAUSE_PARALLEL */
    #pragma omp parallel private(local_var) shared(global_counter) default(none)
    {
        local_var = omp_get_thread_num();
        
        #pragma omp critical
        {
            global_counter += local_var + 1;
        }
    }
    
    printf("test_parallel: executed with %d threads\n", omp_get_max_threads());
}

/* Test function for OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section1_result = 0, section2_result = 0, section3_result = 0;
    
    /* Parallel sections directive - triggers OMP_CLAUSE_SECTIONS */
    #pragma omp parallel sections private(section1_result, section2_result, section3_result) \
            shared(global_counter) firstprivate(global_counter)
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
            section3_result = 999;
            #pragma omp atomic
            global_counter += section3_result;
        }
    }
    
    printf("test_sections: sections completed (results: %d, %d, %d)\n", 
           section1_result, section2_result, section3_result);
}

/* Test function for OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_sum = 0;
    
    /* Nested construct with taskgroup - triggers OMP_CLAUSE_TASKGROUP */
    #pragma omp parallel master
    {
        #pragma omp taskgroup
        {
            #pragma omp task shared(task_sum) 
            {
                int local = 42;
                #pragma omp atomic
                task_sum += local;
            }
            
            #pragma omp task shared(task_sum)
            {
                int local = 58;
                #pragma omp atomic  
                task_sum += local;
            }
            
            #pragma omp taskwait
        }
        
        /* Additional task after taskgroup */
        #pragma omp task shared(task_sum, global_counter)
        {
            #pragma omp atomic
            global_counter += task_sum;
        }
    }
    
    printf("test_taskgroup: task_sum = %d (expected 100)\n", task_sum);
}

/* Additional test with nested clauses */
void test_nested_constructs(void) {
    int i, j;
    int matrix_sum = 0;
    
    /* Nested parallel regions with different clauses */
    #pragma omp parallel private(i, j) shared(matrix_sum)
    {
        #pragma omp for collapse(2) schedule(dynamic)
        for (i = 0; i < 10; i++) {
            for (j = 0; j < 10; j++) {
                #pragma omp atomic
                matrix_sum += i * j;
            }
        }
        
        #pragma omp single
        {
            printf("test_nested_constructs: matrix_sum = %d\n", matrix_sum);
            #pragma omp atomic
            global_counter += matrix_sum;
        }
    }
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
    test_nested_constructs();
    
    printf("\nAll tests completed.\n");
    printf("Final global_counter = %d\n", global_counter);
    
    /* Simple validation */
    if (global_counter > 0) {
        printf("SUCCESS: All OpenMP constructs executed.\n");
        return 0;
    } else {
        printf("ERROR: No computation performed.\n");
        return 1;
    }
}
