/* test_omp_clauses.c
 * Program to trigger GCC tree pretty-printer output for specific OpenMP clauses:
 * - OMP_CLAUSE_FOR
 * - OMP_CLAUSE_PARALLEL  
 * - OMP_CLAUSE_SECTIONS
 * - OMP_CLAUSE_TASKGROUP
 */

#include <stdio.h>
#include <omp.h>

volatile int global_counter = 0;
volatile int dummy = 0;

/* Test function for OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int sum = 0;
    
    /* Direct usage of 'for' clause in combined parallel for directive */
    #pragma omp parallel for schedule(static) private(i) shared(sum) reduction(+:sum)
    for (i = 0; i < 100; i++) {
        sum += i;
        dummy = i;  /* Prevent optimization */
    }
    
    /* Another variation with collapse */
    #pragma omp parallel for collapse(2) private(i) shared(global_counter)
    for (i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            #pragma omp atomic
            global_counter++;
        }
    }
    
    printf("test_parallel_for completed: sum = %d, counter = %d\n", sum, global_counter);
}

/* Test function for OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int local_var = 0;
    
    /* Standalone parallel region */
    #pragma omp parallel private(local_var) shared(global_counter)
    {
        local_var = omp_get_thread_num();
        
        #pragma omp critical
        {
            global_counter += local_var;
            dummy = local_var;
        }
    }
    
    /* Parallel region with if clause */
    #pragma omp parallel if(global_counter > 0) num_threads(2) \
                default(shared) private(local_var)
    {
        local_var = omp_get_thread_num() * 10;
        #pragma omp atomic
        global_counter += local_var;
    }
    
    printf("test_parallel completed: counter = %d\n", global_counter);
}

/* Test function for OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section1_result = 0, section2_result = 0, section3_result = 0;
    
    /* Combined parallel sections directive */
    #pragma omp parallel sections private(dummy) \
                shared(section1_result, section2_result, section3_result)
    {
        #pragma omp section
        {
            for (int i = 0; i < 50; i++) {
                section1_result += i;
            }
            dummy = section1_result;
        }
        
        #pragma omp section  
        {
            for (int i = 50; i < 100; i++) {
                section2_result += i;
            }
            dummy = section2_result;
        }
        
        #pragma omp section
        {
            for (int i = 100; i < 150; i++) {
                section3_result += i;
            }
            dummy = section3_result;
        }
    }
    
    /* Standalone sections (implicitly parallel) */
    #pragma omp sections firstprivate(section1_result) lastprivate(section2_result)
    {
        #pragma omp section
        {
            section1_result = 1;
        }
        #pragma omp section
        {
            section2_result = 2;
        }
    }
    
    printf("test_sections completed: results = %d, %d, %d\n", 
           section1_result, section2_result, section3_result);
}

/* Test function for OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_sum = 0;
    
    /* Create a parallel region that generates tasks */
    #pragma omp parallel shared(task_sum)
    {
        #pragma omp master
        {
            /* Taskgroup enclosing multiple tasks */
            #pragma omp taskgroup
            {
                #pragma omp task shared(task_sum)
                {
                    #pragma omp atomic
                    task_sum += 10;
                    dummy = 10;
                }
                
                #pragma omp task shared(task_sum)
                {
                    #pragma omp atomic  
                    task_sum += 20;
                    dummy = 20;
                }
                
                #pragma omp task shared(task_sum)
                {
                    #pragma omp atomic
                    task_sum += 30;
                    dummy = 30;
                }
            }
        }
    }
    
    /* Another taskgroup pattern using single */
    #pragma omp parallel shared(task_sum)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (int i = 0; i < 5; i++) {
                    #pragma omp task firstprivate(i) shared(task_sum)
                    {
                        #pragma omp atomic
                        task_sum += i;
                        dummy = i;
                    }
                }
            }
        }
    }
    
    printf("test_taskgroup completed: task_sum = %d\n", task_sum);
}

/* Main function that calls all test functions */
int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Reset counter */
    global_counter = 0;
    
    /* Execute all test functions */
    test_parallel_for();
    test_parallel();
    test_sections();
    test_taskgroup();
    
    printf("\nAll OpenMP clause tests completed successfully!\n");
    printf("Final global_counter = %d\n", global_counter);
    
    return 0;
}
