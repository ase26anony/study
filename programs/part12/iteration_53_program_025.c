/* test-openmp-clauses.c
 * This program is designed to trigger the pretty-printer logic for
 * specific OpenMP clauses: for, parallel, sections, and taskgroup.
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test-openmp-clauses.c
 * Also try: gcc -O2 -fopenmp -fdump-tree-all test-openmp-clauses.c
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
    
    /* This pragma should generate OMP_CLAUSE_FOR and OMP_CLAUSE_PARALLEL nodes */
    #pragma omp target teams distribute parallel for reduction(+:local_sum) \
        map(tofrom:local_sum) private(i)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    /* Also test them separately in nested form */
    #pragma omp target parallel
    {
        #pragma omp for private(i) reduction(+:global_sum)
        for (i = 0; i < N/2; i++) {
            global_sum += global_array[i];
        }
    }
    
    printf("test_for_and_parallel: local_sum = %d, global_sum = %d\n", local_sum, global_sum);
}

/* Function 2: Tests 'sections' clause */
static void test_sections(void) {
    int section_a_sum = 0, section_b_sum = 0;
    int i;
    
    /* This pragma should generate OMP_CLAUSE_SECTIONS node */
    #pragma omp target teams distribute parallel for
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    /* Sections inside target teams */
    #pragma omp target teams
    {
        #pragma omp sections private(i) reduction(+:section_a_sum, section_b_sum)
        {
            #pragma omp section
            for (i = 0; i < N/2; i++) {
                section_a_sum += global_array[i];
            }
            
            #pragma omp section
            for (i = N/2; i < N; i++) {
                section_b_sum += global_array[i];
            }
        }
    }
    
    printf("test_sections: section_a_sum = %d, section_b_sum = %d\n", 
           section_a_sum, section_b_sum);
}

/* Function 3: Tests 'taskgroup' clause */
static void test_taskgroup(void) {
    int task_sum = 0;
    
    /* Taskgroup inside target parallel region */
    #pragma omp target parallel
    {
        /* This should generate OMP_CLAUSE_TASKGROUP node */
        #pragma omp taskgroup
        {
            #pragma omp task shared(task_sum)
            {
                int i;
                int local_task_sum = 0;
                for (i = 0; i < N; i++) {
                    local_task_sum += global_array[i];
                }
                #pragma omp atomic
                task_sum += local_task_sum;
            }
            
            #pragma omp task shared(task_sum)
            {
                int i;
                int local_task_sum = 0;
                for (i = 0; i < N; i++) {
                    local_task_sum += global_array[i] * 2;
                }
                #pragma omp atomic
                task_sum += local_task_sum;
            }
        }
    }
    
    printf("test_taskgroup: task_sum = %d\n", task_sum);
}

/* Function 4: Tests all clauses in a single complex construct */
static void test_combined(void) {
    int combined_sum = 0;
    int i;
    
    /* Complex construct with multiple clauses */
    #pragma omp target teams distribute parallel for private(i) reduction(+:combined_sum)
    for (i = 0; i < N; i++) {
        combined_sum += global_array[i];
    }
    
    /* Nested with sections */
    #pragma omp target parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            {
                #pragma omp taskgroup
                {
                    #pragma omp task
                    {
                        combined_sum += 1;
                    }
                }
            }
            #pragma omp section
            {
                combined_sum += 2;
            }
        }
    }
    
    printf("test_combined: combined_sum = %d\n", combined_sum);
}

int main(void) {
    int i;
    
    /* Initialize global array */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 50;
    }
    
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Call all test functions to generate various OpenMP constructs */
    test_for_and_parallel();      /* Generates for and parallel clauses */
    test_sections();              /* Generates sections clause */
    test_taskgroup();             /* Generates taskgroup clause */
    test_combined();              /* Generates all clauses in combination */
    
    /* Final verification sum */
    int final_sum = 0;
    #pragma omp parallel for reduction(+:final_sum) private(i)
    for (i = 0; i < N; i++) {
        final_sum += global_array[i];
    }
    
    printf("Final sum = %d\n", final_sum);
    printf("Test completed successfully.\n");
    
    return 0;
}
