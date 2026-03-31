/* test_omp_clauses.c - Coverage test for OpenMP clause pretty-printing */
#include <stdio.h>
#include <omp.h>

volatile int global_counter = 0;

/* Test OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int sum = 0;
    volatile int local_volatile = 0;
    
    /* Direct usage of 'for' clause in combined construct */
    #pragma omp parallel for private(i) shared(sum) schedule(static) num_threads(2)
    for (i = 0; i < 100; i++) {
        sum += i;
        local_volatile = i;  /* Prevent optimization */
    }
    
    /* Another variation with collapse */
    #pragma omp parallel for collapse(2) private(i) reduction(+:sum)
    for (i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            sum += i * j;
        }
    }
    
    printf("test_parallel_for: sum = %d\n", sum);
    #pragma omp atomic
    global_counter++;
}

/* Test OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int local_sum = 0;
    volatile int barrier_var = 0;
    
    /* Standalone parallel region */
    #pragma omp parallel private(local_sum) shared(barrier_var) num_threads(2)
    {
        int tid = omp_get_thread_num();
        local_sum = tid * 100;
        
        /* Use barrier to ensure construct isn't optimized */
        #pragma omp barrier
        barrier_var = tid;
        
        #pragma omp critical
        {
            global_counter += local_sum;
        }
    }
    
    /* Parallel region with if clause */
    #pragma omp parallel if(1) default(none) shared(global_counter)
    {
        #pragma omp atomic
        global_counter++;
    }
    
    printf("test_parallel: barrier_var = %d\n", barrier_var);
}

/* Test OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section1_result = 0, section2_result = 0;
    volatile int dummy = 0;
    
    /* Combined parallel sections */
    #pragma omp parallel sections private(dummy) num_threads(2)
    {
        #pragma omp section
        {
            for (int i = 0; i < 50; i++) {
                section1_result += i;
                dummy = i;  /* Prevent optimization */
            }
            #pragma omp atomic
            global_counter += section1_result;
        }
        
        #pragma omp section
        {
            for (int i = 50; i < 100; i++) {
                section2_result += i;
                dummy = i;  /* Prevent optimization */
            }
            #pragma omp atomic
            global_counter += section2_result;
        }
    }
    
    /* Separate sections directive inside parallel */
    #pragma omp parallel
    {
        #pragma omp sections nowait
        {
            #pragma omp section
            { dummy = 1; }
            
            #pragma omp section
            { dummy = 2; }
        }
    }
    
    printf("test_sections: results = %d, %d\n", section1_result, section2_result);
}

/* Test OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_sum = 0;
    volatile int sync_var = 0;
    
    /* Taskgroup inside parallel master */
    #pragma omp parallel master num_threads(2)
    {
        #pragma omp taskgroup
        {
            #pragma omp task shared(task_sum) firstprivate(sync_var)
            {
                for (int i = 0; i < 25; i++) {
                    #pragma omp atomic
                    task_sum += i;
                    sync_var = i;
                }
            }
            
            #pragma omp task shared(task_sum) firstprivate(sync_var)
            {
                for (int i = 25; i < 50; i++) {
                    #pragma omp atomic
                    task_sum += i;
                    sync_var = i;
                }
            }
            
            /* Implicit taskwait at end of taskgroup */
        }
    }
    
    /* Taskgroup inside single */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    #pragma omp atomic
                    global_counter++;
                }
                
                #pragma omp task
                {
                    #pragma omp atomic
                    global_counter++;
                }
            }
        }
    }
    
    printf("test_taskgroup: task_sum = %d\n", task_sum);
}

/* Combined test with nested constructs */
void test_combined(void) {
    volatile int control = 1;
    
    /* Nested: parallel -> for -> taskgroup */
    #pragma omp parallel if(control) num_threads(2)
    {
        #pragma omp for schedule(dynamic)
        for (int i = 0; i < 10; i++) {
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    #pragma omp atomic
                    global_counter += i;
                }
            }
        }
    }
    
    printf("test_combined executed\n");
}

int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    test_parallel_for();
    test_parallel();
    test_sections();
    test_taskgroup();
    test_combined();
    
    printf("\nAll tests completed. Global counter = %d\n", global_counter);
    printf("Expected clauses triggered:\n");
    printf("  - OMP_CLAUSE_FOR (via parallel for, for)\n");
    printf("  - OMP_CLAUSE_PARALLEL (via parallel regions)\n");
    printf("  - OMP_CLAUSE_SECTIONS (via parallel sections, sections)\n");
    printf("  - OMP_CLAUSE_TASKGROUP (via taskgroup directives)\n");
    
    return 0;
}
