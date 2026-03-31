/* test_openmp_clauses.c
 * 
 * This program is designed to generate OpenMP AST nodes for the clauses:
 *   OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP
 * to trigger the pretty-printer logic in tree-pretty-print.cc lines 1434-1445.
 *
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test_openmp_clauses.c -o test_openmp_clauses
 * Additional dump flags: -fdump-tree-all, -fdump-tree-gimple
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000

/* File-scope variables for testing data environment */
static int global_array[N];
static int global_sum = 0;

/* Function 1: Tests target parallel for with 'for' and 'parallel' clauses */
static void test_target_parallel_for(void) {
    int i;
    int local_sum = 0;
    
    /* This generates OMP_CLAUSE_FOR and OMP_CLAUSE_PARALLEL nodes */
    #pragma omp target teams distribute parallel for map(tofrom: local_sum) private(i) reduction(+:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    #pragma omp atomic
    global_sum += local_sum;
    
    printf("test_target_parallel_for: local_sum = %d\n", local_sum);
}

/* Function 2: Tests target parallel (parallel clause only) */
static void test_target_parallel(void) {
    int i;
    int temp = 0;
    
    /* This generates OMP_CLAUSE_PARALLEL node */
    #pragma omp target parallel private(i) reduction(+:temp)
    {
        #pragma omp for
        for (i = 0; i < N; i++) {
            temp += global_array[i] % 10;
        }
    }
    
    #pragma omp atomic
    global_sum += temp;
    
    printf("test_target_parallel: temp = %d\n", temp);
}

/* Function 3: Tests target sections (sections clause) */
static void test_target_sections(void) {
    int section1_sum = 0, section2_sum = 0;
    
    /* This generates OMP_CLAUSE_SECTIONS node */
    #pragma omp target teams
    #pragma omp sections private(global_array) reduction(+:section1_sum, section2_sum)
    {
        #pragma omp section
        {
            for (int i = 0; i < N/2; i++) {
                section1_sum += global_array[i];
            }
        }
        
        #pragma omp section
        {
            for (int i = N/2; i < N; i++) {
                section2_sum += global_array[i];
            }
        }
    }
    
    #pragma omp atomic
    global_sum += section1_sum + section2_sum;
    
    printf("test_target_sections: section1_sum = %d, section2_sum = %d\n", 
           section1_sum, section2_sum);
}

/* Function 4: Tests taskgroup clause inside target region */
static void test_target_with_taskgroup(void) {
    int task_sum = 0;
    
    /* Outer target parallel region */
    #pragma omp target parallel
    {
        /* This generates OMP_CLAUSE_TASKGROUP node */
        #pragma omp taskgroup task_reduction(+:task_sum)
        {
            #pragma omp task in_reduction(+:task_sum)
            {
                int local = 0;
                for (int i = 0; i < N/4; i++) {
                    local += global_array[i];
                }
                #pragma omp atomic
                task_sum += local;
            }
            
            #pragma omp task in_reduction(+:task_sum)
            {
                int local = 0;
                for (int i = N/4; i < N/2; i++) {
                    local += global_array[i];
                }
                #pragma omp atomic
                task_sum += local;
            }
        }
    }
    
    #pragma omp atomic
    global_sum += task_sum;
    
    printf("test_target_with_taskgroup: task_sum = %d\n", task_sum);
}

/* Function 5: Combined parallel for in a single pragma */
static void test_combined_parallel_for(void) {
    int combined_sum = 0;
    
    /* This generates both OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR nodes */
    #pragma omp target parallel for private(global_array) reduction(+:combined_sum)
    for (int i = 0; i < N; i++) {
        combined_sum += global_array[i] * 2;
    }
    
    #pragma omp atomic
    global_sum += combined_sum;
    
    printf("test_combined_parallel_for: combined_sum = %d\n", combined_sum);
}

/* Main function orchestrates all tests */
int main(void) {
    /* Initialize array with predictable values */
    for (int i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    printf("Initial global_sum = %d\n", global_sum);
    
    /* Execute all test functions to generate various OpenMP clauses */
    test_target_parallel_for();      /* Generates FOR and PARALLEL clauses */
    test_target_parallel();          /* Generates PARALLEL clause */
    test_target_sections();          /* Generates SECTIONS clause */
    test_target_with_taskgroup();    /* Generates TASKGROUP clause */
    test_combined_parallel_for();    /* Generates combined PARALLEL and FOR clauses */
    
    printf("Final global_sum = %d\n", global_sum);
    
    /* Verification */
    int expected_sum = 0;
    for (int i = 0; i < N; i++) {
        expected_sum += global_array[i];                    /* from test_target_parallel_for */
        expected_sum += global_array[i] % 10;               /* from test_target_parallel */
        expected_sum += global_array[i];                    /* from test_target_sections (both sections) */
        expected_sum += global_array[i % (N/2)];            /* from test_target_with_taskgroup (first half) */
        expected_sum += global_array[i] * 2;                /* from test_combined_parallel_for */
    }
    
    printf("Expected sum = %d\n", expected_sum);
    printf("Verification %s\n", (global_sum == expected_sum) ? "PASSED" : "FAILED");
    
    return 0;
}
