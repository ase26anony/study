/* test-omp-clauses.c
 * This program is designed to trigger the OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP pretty-printing logic
 * in tree-pretty-print.cc (lines 1434-1445).
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test.c
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function to test target parallel for and parallel clauses */
static void test_target_parallel_for(void) {
    int i;
    int local_sum = 0;
    
    /* This generates OMP_CLAUSE_FOR and OMP_CLAUSE_PARALLEL */
    #pragma omp target teams distribute parallel for map(tofrom: local_sum) \
                reduction(+:local_sum) private(i)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    /* Combined parallel and for in target region */
    #pragma omp target parallel for reduction(+:global_sum) private(i)
    for (i = 0; i < N; i++) {
        global_sum += global_array[i];
    }
    
    printf("Target parallel for sum: %d\n", local_sum);
}

/* Function to test target sections clause */
static void test_target_sections(void) {
    int section_a = 0, section_b = 0;
    int i;
    
    /* This generates OMP_CLAUSE_SECTIONS */
    #pragma omp target teams distribute parallel for simd private(i)
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    #pragma omp target teams
    {
        #pragma omp sections reduction(+:section_a, section_b)
        {
            #pragma omp section
            {
                for (i = 0; i < N/2; i++) {
                    section_a += global_array[i];
                }
            }
            #pragma omp section
            {
                for (i = N/2; i < N; i++) {
                    section_b += global_array[i];
                }
            }
        }
    }
    
    printf("Target sections sums: %d, %d\n", section_a, section_b);
}

/* Function to test taskgroup clause */
static void test_taskgroup(void) {
    int task_sum = 0;
    
    /* taskgroup must be inside a parallel region */
    #pragma omp target parallel
    {
        #pragma omp single
        {
            /* This generates OMP_CLAUSE_TASKGROUP */
            #pragma omp taskgroup task_reduction(+:task_sum)
            {
                #pragma omp task in_reduction(+:task_sum)
                {
                    for (int i = 0; i < N; i++) {
                        task_sum += global_array[i] % 10;
                    }
                }
                
                #pragma omp task in_reduction(+:task_sum)
                {
                    for (int i = 0; i < N; i++) {
                        task_sum += (global_array[i] / 10) % 10;
                    }
                }
            }
        }
    }
    
    printf("Taskgroup sum: %d\n", task_sum);
}

/* Function with nested clauses */
static void test_nested_clauses(void) {
    int nested_sum = 0;
    
    /* Nested: parallel inside target with for */
    #pragma omp target
    {
        #pragma omp parallel for reduction(+:nested_sum) private(int i)
        for (int i = 0; i < N; i++) {
            nested_sum += global_array[i] * 2;
        }
        
        /* taskgroup inside parallel region */
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp taskgroup
                {
                    #pragma omp task
                    {
                        nested_sum += 1;
                    }
                }
            }
        }
    }
    
    printf("Nested clauses sum: %d\n", nested_sum);
}

int main(void) {
    /* Initialize global array */
    for (int i = 0; i < N; i++) {
        global_array[i] = i + 1;
    }
    
    printf("Testing OpenMP clause pretty-printing coverage...\n");
    
    /* Call all test functions to generate various OMP clauses */
    test_target_parallel_for();
    test_target_sections();
    test_taskgroup();
    test_nested_clauses();
    
    /* Final verification */
    int final_check = 0;
    #pragma omp target parallel for reduction(+:final_check)
    for (int i = 0; i < N; i++) {
        final_check += global_array[i];
    }
    
    printf("Final check sum: %d\n", final_check);
    printf("Global sum: %d\n", global_sum);
    
    return 0;
}
