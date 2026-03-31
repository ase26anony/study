/* test_openmp_clauses.c
 * 
 * This program is designed to generate OpenMP AST nodes for the clauses:
 *   OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP
 * which will be visited by the pretty-printer in tree-pretty-print.cc
 * when compiled with appropriate dump flags (e.g., -fdump-tree-omplower).
 *
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test_openmp_clauses.c
 * Also try:     gcc -O2 -fopenmp -fdump-tree-all test_openmp_clauses.c
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
    
    /* Initialize array */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    /* 1. OMP_CLAUSE_FOR: target teams distribute parallel for */
    #pragma omp target teams distribute parallel for map(tofrom: local_sum) private(i) reduction(+:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    /* 2. OMP_CLAUSE_PARALLEL: target parallel */
    #pragma omp target parallel private(i) reduction(+:local_sum)
    {
        #pragma omp for
        for (i = 0; i < N; i++) {
            local_sum += global_array[i] * 2;
        }
    }
    
    /* 3. Combined parallel for clause in single pragma */
    int combined_sum = 0;
    #pragma omp target parallel for map(tofrom: combined_sum) private(i) reduction(+:combined_sum)
    for (i = 0; i < N; i++) {
        combined_sum += global_array[i] * 3;
    }
    
    printf("test_for_and_parallel: local_sum = %d, combined_sum = %d\n", local_sum, combined_sum);
}

/* Function 2: Demonstrates OMP_CLAUSE_SECTIONS
 * Uses target sections (sections clause) inside target teams.
 */
static void test_sections(void) {
    int section_a = 0, section_b = 0, section_c = 0;
    int i;
    
    /* OMP_CLAUSE_SECTIONS: target sections inside target teams */
    #pragma omp target teams
    #pragma omp sections private(i) reduction(+:section_a, section_b, section_c)
    {
        #pragma omp section
        for (i = 0; i < N/2; i++) {
            section_a += global_array[i];
        }
        
        #pragma omp section
        for (i = N/2; i < N; i++) {
            section_b += global_array[i];
        }
        
        #pragma omp section
        {
            for (i = 0; i < N; i += 10) {
                section_c += global_array[i];
            }
        }
    }
    
    printf("test_sections: a=%d, b=%d, c=%d\n", section_a, section_b, section_c);
}

/* Function 3: Demonstrates OMP_CLAUSE_TASKGROUP
 * Uses taskgroup clause inside a target parallel region.
 */
static void test_taskgroup(void) {
    int task_sum = 0;
    int i;
    
    /* OMP_CLAUSE_TASKGROUP: taskgroup inside target parallel */
    #pragma omp target parallel private(i)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task private(i) shared(task_sum)
                {
                    int local_task = 0;
                    for (i = 0; i < N; i += 4) {
                        local_task += global_array[i];
                    }
                    #pragma omp atomic
                    task_sum += local_task;
                }
                
                #pragma omp task private(i) shared(task_sum)
                {
                    int local_task = 0;
                    for (i = 1; i < N; i += 4) {
                        local_task += global_array[i];
                    }
                    #pragma omp atomic
                    task_sum += local_task;
                }
            } /* end taskgroup */
        } /* end single */
    } /* end target parallel */
    
    printf("test_taskgroup: task_sum = %d\n", task_sum);
}

/* Function 4: Mixed clauses in nested contexts
 * Combines multiple clauses across nested constructs.
 */
static void test_mixed_nested(void) {
    int outer_sum = 0, inner_sum = 0;
    int i, j;
    
    /* Outer: target parallel (parallel clause) */
    #pragma omp target parallel private(i, j) reduction(+:outer_sum)
    {
        /* Inner: target teams distribute parallel for (for clause) */
        #pragma omp target teams distribute parallel for private(j) reduction(+:inner_sum)
        for (i = 0; i < 10; i++) {
            for (j = 0; j < N; j++) {
                inner_sum += global_array[j] + i;
            }
        }
        outer_sum += inner_sum;
    }
    
    printf("test_mixed_nested: outer_sum = %d, inner_sum = %d\n", outer_sum, inner_sum);
}

int main(void) {
    /* Initialize global array */
    for (int i = 0; i < N; i++) {
        global_array[i] = (i * 3) % 97;
    }
    
    printf("=== OpenMP Clause Coverage Test ===\n");
    
    /* Call each test function to generate various OpenMP clauses */
    test_for_and_parallel();      /* for, parallel clauses */
    test_sections();              /* sections clause */
    test_taskgroup();             /* taskgroup clause */
    test_mixed_nested();          /* nested combinations */
    
    /* Final verification sum */
    int final_sum = 0;
    #pragma omp target parallel for reduction(+:final_sum)
    for (int i = 0; i < N; i++) {
        final_sum += global_array[i];
    }
    
    printf("Final sum = %d\n", final_sum);
    printf("=== End of Test ===\n");
    
    return 0;
}
