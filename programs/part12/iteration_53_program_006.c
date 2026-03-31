/* test_openmp_clauses.c
 * 
 * This program is designed to trigger the pretty-printing of specific
 * OpenMP clause nodes (OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP) in GCC's tree-pretty-print.cc.
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test_openmp_clauses.c
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function to test target parallel for and taskgroup clauses */
static void test_target_parallel_for_and_taskgroup(void) {
    int i;
    int local_sum = 0;
    
    /* This will generate OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR nodes */
    #pragma omp target teams distribute parallel for map(tofrom: local_sum) private(i) reduction(+:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    /* Nested taskgroup inside target parallel region */
    #pragma omp target parallel private(i)
    {
        #pragma omp taskgroup  /* This will generate OMP_CLAUSE_TASKGROUP node */
        {
            #pragma omp task
            {
                /* Simple task to ensure taskgroup is not optimized away */
                int temp = 0;
                for (i = 0; i < 10; i++) {
                    temp += i;
                }
            }
        }
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function to test target sections clause */
static void test_target_sections(void) {
    int section_a = 0, section_b = 0;
    int i;
    
    /* This will generate OMP_CLAUSE_SECTIONS node */
    #pragma omp target teams
    #pragma omp sections private(i) reduction(+:section_a, section_b)
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
    
    #pragma omp atomic
    global_sum += section_a + section_b;
}

/* Function to test combined parallel for clause */
static void test_target_parallel_for_combined(void) {
    int i;
    int combined_sum = 0;
    
    /* Combined parallel for in target region - generates both PARALLEL and FOR clauses */
    #pragma omp target parallel for private(i) reduction(+:combined_sum)
    for (i = 0; i < N; i++) {
        combined_sum += global_array[i] * 2;
    }
    
    #pragma omp atomic
    global_sum += combined_sum;
}

/* Main function with multiple OpenMP constructs */
int main(void) {
    int i;
    
    /* Initialize global array */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    /* Test 1: Target parallel for with nested taskgroup */
    test_target_parallel_for_and_taskgroup();
    
    /* Test 2: Target sections */
    test_target_sections();
    
    /* Test 3: Combined target parallel for */
    test_target_parallel_for_combined();
    
    /* Additional direct test in main() */
    int direct_sum = 0;
    
    /* Target teams distribute parallel for - generates FOR clause */
    #pragma omp target teams distribute parallel for map(tofrom: direct_sum) private(i) reduction(+:direct_sum)
    for (i = 0; i < N; i++) {
        direct_sum += global_array[i] * 3;
    }
    
    global_sum += direct_sum;
    
    printf("Final sum: %d\n", global_sum);
    
    /* Verify the result */
    int expected = 0;
    for (i = 0; i < N; i++) {
        expected += global_array[i] * 6;  /* 1 + 1 + 2 + 3 = 7? Let's recalc: */
        /* test1: adds array[i] once */
        /* test2: adds array[i] once (sections cover full array) */
        /* test3: adds array[i] * 2 */
        /* main: adds array[i] * 3 */
        /* Total multiplier: 1 + 1 + 2 + 3 = 7 */
    }
    expected = 0;
    for (i = 0; i < N; i++) {
        expected += global_array[i] * 7;
    }
    
    if (global_sum == expected) {
        printf("Result verification PASSED\n");
        return 0;
    } else {
        printf("Result verification FAILED: expected %d, got %d\n", expected, global_sum);
        return 1;
    }
}
