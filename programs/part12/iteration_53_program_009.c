/* test_openmp_clauses.c
 * This program is designed to trigger the pretty-printer logic for
 * specific OpenMP clauses (for, parallel, sections, taskgroup) in
 * tree-pretty-print.cc lines 1434-1445.
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test_openmp_clauses.c
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
    
    /* This will generate OMP_CLAUSE_FOR and OMP_CLAUSE_PARALLEL nodes */
    #pragma omp target teams distribute parallel for map(tofrom:local_sum) reduction(+:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    /* Another combination: parallel for in target region */
    #pragma omp target parallel for reduction(+:global_sum) private(i)
    for (i = 0; i < N; i++) {
        global_sum += global_array[i];
    }
    
    printf("Local sum from test_for_and_parallel: %d\n", local_sum);
}

/* Function 2: Tests 'sections' clause */
static void test_sections(void) {
    int section1_sum = 0, section2_sum = 0;
    int i;
    
    /* This will generate OMP_CLAUSE_SECTIONS node */
    #pragma omp target teams distribute parallel for simd reduction(+:section1_sum)
    for (i = 0; i < N/2; i++) {
        section1_sum += global_array[i];
    }
    
    /* Sections inside target teams */
    #pragma omp target teams
    {
        #pragma omp sections reduction(+:section2_sum)
        {
            #pragma omp section
            for (i = N/2; i < 3*N/4; i++) {
                section2_sum += global_array[i];
            }
            #pragma omp section
            for (i = 3*N/4; i < N; i++) {
                section2_sum += global_array[i];
            }
        }
    }
    
    printf("Section sums: %d, %d\n", section1_sum, section2_sum);
}

/* Function 3: Tests 'taskgroup' clause */
static void test_taskgroup(void) {
    int task_sum = 0;
    int i;
    
    /* Taskgroup inside target parallel region - will generate OMP_CLAUSE_TASKGROUP */
    #pragma omp target parallel
    {
        #pragma omp taskgroup
        {
            #pragma omp task reduction(+:task_sum)
            {
                for (i = 0; i < N; i += 2) {
                    task_sum += global_array[i];
                }
            }
            
            #pragma omp task reduction(+:task_sum)
            {
                for (i = 1; i < N; i += 2) {
                    task_sum += global_array[i];
                }
            }
        }
    }
    
    /* Another taskgroup example with taskloop */
    #pragma omp target parallel
    {
        #pragma omp taskgroup
        {
            #pragma omp taskloop reduction(+:task_sum) grainsize(64)
            for (i = 0; i < N; i++) {
                task_sum += global_array[i];
            }
        }
    }
    
    printf("Taskgroup sum: %d\n", task_sum);
}

/* Function 4: Complex nested example with multiple clauses */
static void test_nested_constructs(void) {
    int nested_sum = 0;
    int i, j;
    
    /* Nested: target -> parallel -> for */
    #pragma omp target parallel for collapse(2) reduction(+:nested_sum) private(i, j)
    for (i = 0; i < 10; i++) {
        for (j = 0; j < N/10; j++) {
            nested_sum += global_array[i*(N/10) + j];
        }
    }
    
    /* Target with sections containing taskgroup */
    #pragma omp target
    {
        #pragma omp sections
        {
            #pragma omp section
            {
                int partial = 0;
                #pragma omp taskgroup
                {
                    #pragma omp task reduction(+:partial)
                    for (i = 0; i < N/4; i++) partial += global_array[i];
                    #pragma omp task reduction(+:partial)
                    for (i = N/4; i < N/2; i++) partial += global_array[i];
                }
                nested_sum += partial;
            }
            
            #pragma omp section
            {
                int partial = 0;
                #pragma omp parallel for reduction(+:partial)
                for (i = N/2; i < N; i++) partial += global_array[i];
                nested_sum += partial;
            }
        }
    }
    
    printf("Nested constructs sum: %d\n", nested_sum);
}

int main(void) {
    int i;
    
    /* Initialize array with predictable values */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    printf("=== Testing OpenMP clause pretty-printing ===\n");
    
    /* Call all test functions to generate various OpenMP constructs */
    test_for_and_parallel();
    test_sections();
    test_taskgroup();
    test_nested_constructs();
    
    printf("Global sum: %d\n", global_sum);
    printf("=== Program completed successfully ===\n");
    
    return 0;
}
