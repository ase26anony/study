/* test-omp-pretty-print.c
 * 
 * This program is designed to trigger the pretty-printing logic for
 * specific OpenMP clause types in GCC's tree-pretty-print.cc:
 *   OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, 
 *   OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP
 *
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test.c
 * Additional flags: -fdump-tree-all -fdump-tree-gimple
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function 1: Tests 'for' and 'parallel' clauses in combination */
static void test_for_and_parallel(void) {
    int i;
    int local_sum = 0;
    
    /* Combined parallel and for clauses in target region */
    #pragma omp target teams distribute parallel for \
                map(tofrom: local_sum) map(to: global_array) \
                reduction(+:local_sum) private(i)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    /* Separate parallel clause in target region */
    #pragma omp target parallel map(tofrom: global_sum) \
                               reduction(+:global_sum) private(i)
    {
        #pragma omp for
        for (i = 0; i < N; i++) {
            global_sum += global_array[i];
        }
    }
    
    printf("test_for_and_parallel: local_sum = %d, global_sum = %d\n", 
           local_sum, global_sum);
}

/* Function 2: Tests 'sections' clause */
static void test_sections(void) {
    int section1_sum = 0, section2_sum = 0;
    int i;
    
    /* Sections clause inside target teams construct */
    #pragma omp target teams map(to: global_array) \
                             map(tofrom: section1_sum, section2_sum)
    #pragma omp sections reduction(+:section1_sum, section2_sum) \
                         private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < N/2; i++) {
                section1_sum += global_array[i];
            }
        }
        
        #pragma omp section
        {
            for (i = N/2; i < N; i++) {
                section2_sum += global_array[i];
            }
        }
    }
    
    printf("test_sections: section1_sum = %d, section2_sum = %d\n", 
           section1_sum, section2_sum);
}

/* Function 3: Tests 'taskgroup' clause */
static void test_taskgroup(void) {
    int task_sum = 0;
    int i;
    
    /* Taskgroup inside target parallel region */
    #pragma omp target parallel map(to: global_array) \
                               map(tofrom: task_sum) private(i)
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:task_sum)
            {
                #pragma omp task in_reduction(+:task_sum) private(i)
                {
                    int local_task_sum = 0;
                    for (i = 0; i < N/2; i++) {
                        local_task_sum += global_array[i];
                    }
                    #pragma omp atomic
                    task_sum += local_task_sum;
                }
                
                #pragma omp task in_reduction(+:task_sum) private(i)
                {
                    int local_task_sum = 0;
                    for (i = N/2; i < N; i++) {
                        local_task_sum += global_array[i];
                    }
                    #pragma omp atomic
                    task_sum += local_task_sum;
                }
            }
        }
    }
    
    printf("test_taskgroup: task_sum = %d\n", task_sum);
}

/* Function 4: Tests all clauses in nested contexts */
static void test_nested_clauses(void) {
    int nested_sum = 0;
    int i;
    
    /* Nested: target -> parallel -> for */
    #pragma omp target map(to: global_array) map(tofrom: nested_sum)
    {
        #pragma omp parallel private(i) reduction(+:nested_sum)
        {
            #pragma omp for
            for (i = 0; i < N; i++) {
                nested_sum += global_array[i];
            }
        }
        
        /* Sections inside the same target region */
        #pragma omp sections reduction(+:nested_sum) private(i)
        {
            #pragma omp section
            {
                for (i = 0; i < N/4; i++) {
                    nested_sum += global_array[i];
                }
            }
            #pragma omp section
            {
                for (i = N/4; i < N/2; i++) {
                    nested_sum += global_array[i];
                }
            }
        }
    }
    
    printf("test_nested_clauses: nested_sum = %d\n", nested_sum);
}

int main(void) {
    int i;
    
    /* Initialize global array */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Call all test functions to generate various OpenMP constructs */
    test_for_and_parallel();      /* Generates OMP_CLAUSE_FOR and OMP_CLAUSE_PARALLEL */
    test_sections();              /* Generates OMP_CLAUSE_SECTIONS */
    test_taskgroup();             /* Generates OMP_CLAUSE_TASKGROUP */
    test_nested_clauses();        /* Generates multiple clauses in nested context */
    
    /* Final verification sum */
    int final_sum = 0;
    #pragma omp parallel for reduction(+:final_sum) private(i)
    for (i = 0; i < N; i++) {
        final_sum += global_array[i];
    }
    
    printf("Final verification sum = %d\n", final_sum);
    printf("Test completed successfully.\n");
    
    return 0;
}
