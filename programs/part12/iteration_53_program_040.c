/* test_openmp_clauses.c
 * 
 * This program is designed to trigger the OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP pretty-printing logic in
 * tree-pretty-print.cc (lines 1434-1445) when compiled with GCC's
 * -fdump-tree-* flags (e.g., -fdump-tree-omplower, -fdump-tree-original).
 *
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test_openmp_clauses.c
 * Additional dump flags: -fdump-tree-gimple, -fdump-tree-all
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
static void test_for_and_parallel(void)
{
    int i;
    int local_sum = 0;
    
    /* OMP_CLAUSE_FOR: target teams distribute parallel for */
    #pragma omp target teams distribute parallel for map(tofrom:local_sum) private(i)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    /* OMP_CLAUSE_PARALLEL: target parallel */
    #pragma omp target parallel map(tofrom:local_sum) private(i)
    {
        int temp = 0;
        #pragma omp for
        for (i = 0; i < N; i++) {
            temp += global_array[i];
        }
        #pragma omp atomic
        local_sum += temp;
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function 2: Demonstrates OMP_CLAUSE_SECTIONS
 * Uses target sections inside a target teams construct.
 */
static void test_sections(void)
{
    int section_a = 0, section_b = 0;
    
    /* OMP_CLAUSE_SECTIONS: target sections inside target teams */
    #pragma omp target teams
    #pragma omp sections private(section_a, section_b)
    {
        #pragma omp section
        {
            for (int i = 0; i < N/2; i++) {
                section_a += global_array[i];
            }
        }
        #pragma omp section
        {
            for (int i = N/2; i < N; i++) {
                section_b += global_array[i];
            }
        }
    }
    
    #pragma omp atomic
    global_sum += section_a + section_b;
}

/* Function 3: Demonstrates OMP_CLAUSE_TASKGROUP
 * Uses taskgroup inside a target parallel region.
 */
static void test_taskgroup(void)
{
    int task_sum = 0;
    
    /* OMP_CLAUSE_TASKGROUP: taskgroup inside target parallel */
    #pragma omp target parallel map(tofrom:task_sum)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    int temp = 0;
                    for (int i = 0; i < N; i += 2) {
                        temp += global_array[i];
                    }
                    #pragma omp atomic
                    task_sum += temp;
                }
                
                #pragma omp task
                {
                    int temp = 0;
                    for (int i = 1; i < N; i += 2) {
                        temp += global_array[i];
                    }
                    #pragma omp atomic
                    task_sum += temp;
                }
            } /* end taskgroup */
        } /* end single */
    } /* end target parallel */
    
    #pragma omp atomic
    global_sum += task_sum;
}

/* Function 4: Demonstrates clause combination
 * Combines parallel and for in a single pragma.
 */
static void test_combined_clauses(void)
{
    int combined_sum = 0;
    
    /* Combined parallel and for clauses: target parallel for */
    #pragma omp target parallel for map(tofrom:combined_sum) private(int i)
    for (int i = 0; i < N; i++) {
        combined_sum += global_array[i];
    }
    
    #pragma omp atomic
    global_sum += combined_sum;
}

int main(void)
{
    /* Initialize global array */
    for (int i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    /* Reset global sum */
    global_sum = 0;
    
    /* Call test functions to generate various OpenMP clauses */
    test_for_and_parallel();      /* for, parallel */
    test_sections();              /* sections */
    test_taskgroup();             /* taskgroup */
    test_combined_clauses();      /* parallel + for combined */
    
    /* Verify result (should be 4 * sum of array) */
    int expected = 0;
    for (int i = 0; i < N; i++) {
        expected += global_array[i];
    }
    expected *= 4;
    
    printf("Global sum: %d (expected: %d)\n", global_sum, expected);
    printf("Result: %s\n", global_sum == expected ? "PASS" : "FAIL");
    
    return 0;
}
