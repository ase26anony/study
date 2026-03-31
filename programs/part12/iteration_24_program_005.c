/* test_omp_clauses.c - Test program for GCC OpenMP clause coverage */
#include <stdio.h>
#include <omp.h>

volatile int global_counter = 0;

/* Test OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i, sum = 0;
    int n = 100;
    
    /* Use combined parallel for with schedule clause */
    #pragma omp parallel for schedule(static) private(i) shared(n, sum) reduction(+:sum)
    for (i = 0; i < n; i++) {
        sum += i * 2;
    }
    
    printf("test_parallel_for: sum = %d\n", sum);
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
    
    /* Combined parallel sections construct */
    #pragma omp parallel sections private(section1_result, section2_result, section3_result) \
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
        
        #pragma omp section
        {
            for (int i = 100; i < 150; i++) {
                section3_result += i;
            }
            #pragma omp atomic
            global_counter += section3_result;
        }
    }
    
    printf("test_sections: results = %d, %d, %d\n", 
           section1_result, section2_result, section3_result);
}

/* Test OMP_CLAUSE_TASKGROUP */
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
                    int result = i * i;
                    #pragma omp atomic
                    task_sum += result;
                }
            }
        }
        
        /* Wait for all tasks in the taskgroup */
        #pragma omp taskwait
    }
    
    printf("test_taskgroup: task_sum = %d\n", task_sum);
    global_counter += task_sum;
}

/* Additional test with nested constructs */
void test_nested_constructs(void) {
    int nested_sum = 0;
    
    /* Nested parallel region with for loop */
    #pragma omp parallel
    {
        #pragma omp for collapse(2) schedule(dynamic)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                #pragma omp atomic
                nested_sum += i + j;
            }
        }
        
        /* Taskgroup inside parallel region */
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    #pragma omp atomic
                    nested_sum += 100;
                }
                
                #pragma omp task
                {
                    #pragma omp atomic
                    nested_sum += 200;
                }
            }
        }
    }
    
    printf("test_nested_constructs: nested_sum = %d\n", nested_sum);
    global_counter += nested_sum;
}

int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Test each clause type in separate functions */
    test_parallel_for();      /* Triggers OMP_CLAUSE_FOR */
    test_parallel();          /* Triggers OMP_CLAUSE_PARALLEL */
    test_sections();          /* Triggers OMP_CLAUSE_SECTIONS */
    test_taskgroup();         /* Triggers OMP_CLAUSE_TASKGROUP */
    test_nested_constructs(); /* Additional coverage with nested constructs */
    
    printf("\nFinal global_counter = %d\n", global_counter);
    printf("All OpenMP constructs executed successfully.\n");
    
    return 0;
}
