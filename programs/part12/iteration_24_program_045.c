/* test_omp_clauses.c
 * This program exercises specific OpenMP clauses to trigger
 * pretty-printer coverage for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP.
 * Compile with: gcc -O1 -fopenmp -fdump-tree-all test_omp_clauses.c -o test_omp_clauses
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int prevent_optimization = 0;

/* Test function for OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int sum = 0;
    int private_var = 10;
    
    /* Combined parallel for directive - triggers OMP_CLAUSE_FOR */
    #pragma omp parallel for private(private_var) schedule(static) reduction(+:sum)
    for (i = 0; i < 100; i++) {
        private_var = i;
        sum += private_var;
        /* Prevent loop optimization */
        prevent_optimization += i % 2;
    }
    
    /* Also test standalone for clause in worksharing context */
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (i = 0; i < 50; i++) {
            #pragma omp atomic
            global_counter++;
        }
    }
    
    printf("test_parallel_for: sum = %d, global_counter = %d\n", sum, global_counter);
}

/* Test function for OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int local_sum = 0;
    
    /* Basic parallel region - triggers OMP_CLAUSE_PARALLEL */
    #pragma omp parallel private(local_sum) shared(global_counter)
    {
        local_sum = omp_get_thread_num();
        
        #pragma omp atomic
        global_counter += local_sum;
        
        /* Nested parallel region for additional coverage */
        #pragma omp parallel num_threads(1)
        {
            #pragma omp atomic
            prevent_optimization++;
        }
    }
    
    /* Parallel region with if clause */
    #pragma omp parallel if(global_counter > 0) default(shared)
    {
        #pragma omp single
        {
            printf("test_parallel: Thread %d executing single\n", omp_get_thread_num());
        }
    }
    
    printf("test_parallel: global_counter = %d\n", global_counter);
}

/* Test function for OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section1_result = 0;
    int section2_result = 0;
    
    /* Combined parallel sections - triggers OMP_CLAUSE_SECTIONS */
    #pragma omp parallel sections private(prevent_optimization) \
                firstprivate(global_counter) lastprivate(section1_result)
    {
        #pragma omp section
        {
            section1_result = 1;
            for (int i = 0; i < 10; i++) {
                prevent_optimization += i;
            }
        }
        
        #pragma omp section
        {
            section2_result = 2;
            #pragma omp atomic
            global_counter += 5;
        }
        
        #pragma omp section
        {
            /* Empty section still creates the clause */
            int dummy = section1_result + section2_result;
            prevent_optimization += dummy;
        }
    }
    
    /* Standalone sections directive */
    #pragma omp parallel
    {
        #pragma omp sections reduction(+:prevent_optimization)
        {
            #pragma omp section
            { prevent_optimization += 1; }
            
            #pragma omp section
            { prevent_optimization += 2; }
        }
    }
    
    printf("test_sections: results = %d, %d, global_counter = %d\n", 
           section1_result, section2_result, global_counter);
}

/* Test function for OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int taskgroup_sum = 0;
    
    /* Create a parallel region that generates tasks */
    #pragma omp parallel num_threads(2)
    {
        #pragma omp master
        {
            /* Taskgroup clause - triggers OMP_CLAUSE_TASKGROUP */
            #pragma omp taskgroup
            {
                for (int i = 0; i < 5; i++) {
                    #pragma omp task shared(taskgroup_sum) firstprivate(i)
                    {
                        #pragma omp atomic
                        taskgroup_sum += i;
                        
                        /* Nested task within taskgroup */
                        #pragma omp task
                        {
                            #pragma omp atomic
                            prevent_optimization++;
                        }
                    }
                }
            }
            
            /* Another taskgroup with depend clause */
            int dep_var = 0;
            #pragma omp taskgroup
            {
                #pragma omp task depend(out: dep_var)
                { dep_var = 1; }
                
                #pragma omp task depend(in: dep_var)
                {
                    #pragma omp atomic
                    global_counter += dep_var;
                }
            }
        }
    }
    
    /* Alternative: taskgroup inside single region */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                { prevent_optimization *= 2; }
                
                #pragma omp task
                { prevent_optimization /= 2; }
            }
        }
    }
    
    printf("test_taskgroup: taskgroup_sum = %d, global_counter = %d\n", 
           taskgroup_sum, global_counter);
}

/* Additional test with nested clauses for maximum coverage */
void test_nested_constructs(void) {
    /* Nested parallel with for inside */
    #pragma omp parallel
    {
        #pragma omp for collapse(2)
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                #pragma omp atomic
                prevent_optimization += i * j;
            }
        }
        
        /* Sections inside parallel */
        #pragma omp sections
        {
            #pragma omp section
            { global_counter++; }
            
            #pragma omp section
            { 
                /* Taskgroup inside section inside parallel */
                #pragma omp taskgroup
                {
                    #pragma omp task
                    { prevent_optimization--; }
                }
            }
        }
    }
    
    printf("test_nested_constructs: prevent_optimization = %d\n", prevent_optimization);
}

int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Reset counters */
    global_counter = 0;
    prevent_optimization = 0;
    
    /* Execute all test functions */
    test_parallel_for();
    
    global_counter = 0;  /* Reset for clarity in output */
    test_parallel();
    
    test_sections();
    
    test_taskgroup();
    
    test_nested_constructs();
    
    /* Final verification */
    printf("\nAll tests completed successfully!\n");
    printf("Final values: global_counter = %d, prevent_optimization = %d\n", 
           global_counter, prevent_optimization);
    
    return 0;
}
