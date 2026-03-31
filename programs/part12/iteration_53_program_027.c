/* test-openmp-clauses.c
 * 
 * This program is designed to generate OpenMP AST nodes that will trigger
 * the pretty-printer logic for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP in tree-pretty-print.cc.
 * 
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test-openmp-clauses.c
 * Additional dump flags: -fdump-tree-all, -fdump-tree-gimple
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
                map(tofrom: local_sum) private(i) reduction(+:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    /* Separate parallel clause in target region */
    #pragma omp target parallel private(i) shared(global_array) \
                reduction(+:global_sum)
    {
        #pragma omp for
        for (i = 0; i < N; i++) {
            global_sum += global_array[i];
        }
    }
    
    printf("Local sum from test_for_and_parallel: %d\n", local_sum);
}

/* Function 2: Tests 'sections' clause */
static void test_sections(void) {
    int section1_sum = 0, section2_sum = 0;
    int i;
    
    /* sections clause inside target teams construct */
    #pragma omp target teams
    #pragma omp sections private(i) reduction(+:section1_sum, section2_sum)
    {
        #pragma omp section
        for (i = 0; i < N/2; i++) {
            section1_sum += global_array[i];
        }
        
        #pragma omp section
        for (i = N/2; i < N; i++) {
            section2_sum += global_array[i];
        }
    }
    
    printf("Section sums: %d, %d\n", section1_sum, section2_sum);
}

/* Function 3: Tests 'taskgroup' clause nested inside parallel region */
static void test_taskgroup(void) {
    int task_results[4] = {0, 0, 0, 0};
    int i;
    
    /* taskgroup clause inside target parallel region */
    #pragma omp target parallel private(i) shared(task_results, global_array)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (i = 0; i < 4; i++) {
                    #pragma omp task firstprivate(i) shared(task_results)
                    {
                        int start = i * (N/4);
                        int end = (i + 1) * (N/4);
                        int j, partial = 0;
                        
                        for (j = start; j < end; j++) {
                            partial += global_array[j];
                        }
                        task_results[i] = partial;
                    }
                }
            }
        }
    }
    
    int total = 0;
    for (i = 0; i < 4; i++) {
        total += task_results[i];
    }
    printf("Taskgroup result: %d\n", total);
}

/* Function 4: Tests all clauses in nested contexts */
static void test_nested_combinations(void) {
    int i, j;
    int matrix_sum = 0;
    int small_array[10];
    
    /* Initialize small array */
    for (i = 0; i < 10; i++) {
        small_array[i] = i;
    }
    
    /* Nested: parallel with for inside, containing taskgroup */
    #pragma omp target parallel private(i, j) shared(small_array) \
                reduction(+:matrix_sum)
    {
        #pragma omp for
        for (i = 0; i < 10; i++) {
            #pragma omp taskgroup
            {
                #pragma omp task firstprivate(i) shared(small_array)
                {
                    for (j = 0; j < 10; j++) {
                        matrix_sum += small_array[i] * small_array[j];
                    }
                }
            }
        }
    }
    
    printf("Matrix sum from nested combinations: %d\n", matrix_sum);
}

int main(void) {
    int i;
    
    /* Initialize global array */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    printf("Testing OpenMP clauses for pretty-printer coverage...\n");
    
    /* Test 1: for and parallel clauses */
    test_for_and_parallel();
    
    /* Test 2: sections clause */
    test_sections();
    
    /* Test 3: taskgroup clause */
    test_taskgroup();
    
    /* Test 4: nested combinations */
    test_nested_combinations();
    
    printf("Global sum: %d\n", global_sum);
    printf("All tests completed.\n");
    
    return 0;
}
