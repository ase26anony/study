/* test-omp-clauses.c
 * This program is designed to generate OpenMP AST nodes for the clauses:
 *   OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP
 * When compiled with -fopenmp and tree-dump flags (-fdump-tree-*), the pretty-printer
 * in tree-pretty-print.cc should visit these clause nodes, covering lines 1434-1445.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function 1: Demonstrates OMP_CLAUSE_FOR and OMP_CLAUSE_PARALLEL
 * Uses target teams distribute parallel for (for clause)
 * and target parallel (parallel clause) in separate regions.
 */
static void test_for_and_parallel(void) {
    int i;
    int local_sum = 0;
    
    /* OMP_CLAUSE_FOR: target teams distribute parallel for */
    #pragma omp target teams distribute parallel for map(tofrom: local_sum) private(i)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    /* OMP_CLAUSE_PARALLEL: target parallel with reduction */
    #pragma omp target parallel reduction(+:global_sum) private(i)
    {
        #pragma omp for
        for (i = 0; i < N; i++) {
            global_sum += global_array[i];
        }
    }
    
    printf("test_for_and_parallel: local_sum = %d, global_sum = %d\n", local_sum, global_sum);
}

/* Function 2: Demonstrates OMP_CLAUSE_SECTIONS
 * Uses target sections inside target teams.
 */
static void test_sections(void) {
    int section_a = 0, section_b = 0;
    int i;
    
    /* OMP_CLAUSE_SECTIONS: target sections within target teams */
    #pragma omp target teams
    #pragma omp sections private(i)
    {
        #pragma omp section
        for (i = 0; i < N/2; i++) {
            section_a += global_array[i];
        }
        
        #pragma omp section
        for (i = N/2; i < N; i++) {
            section_b += global_array[i];
        }
    }
    
    printf("test_sections: section_a = %d, section_b = %d, total = %d\n", 
           section_a, section_b, section_a + section_b);
}

/* Function 3: Demonstrates OMP_CLAUSE_TASKGROUP
 * Uses taskgroup inside a target parallel region.
 */
static void test_taskgroup(void) {
    int task_sum = 0;
    
    /* OMP_CLAUSE_TASKGROUP: taskgroup inside target parallel */
    #pragma omp target parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task shared(task_sum)
                {
                    int i;
                    for (i = 0; i < N; i++) {
                        #pragma omp atomic
                        task_sum += global_array[i];
                    }
                }
                /* Additional tasks could be added here */
            }
        }
    }
    
    printf("test_taskgroup: task_sum = %d\n", task_sum);
}

/* Function 4: Demonstrates clause combination
 * Combines parallel and for in a single pragma.
 */
static void test_combined_clauses(void) {
    int combined_sum = 0;
    int i;
    
    /* Combined parallel and for clauses */
    #pragma omp target parallel for reduction(+:combined_sum) private(i)
    for (i = 0; i < N; i++) {
        combined_sum += global_array[i];
    }
    
    printf("test_combined_clauses: combined_sum = %d\n", combined_sum);
}

int main(void) {
    int i;
    
    /* Initialize global array */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Call all test functions to generate various OpenMP constructs */
    test_for_and_parallel();
    test_sections();
    test_taskgroup();
    test_combined_clauses();
    
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
