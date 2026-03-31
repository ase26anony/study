/* test-omp-clauses.c
 * This program is designed to trigger the pretty-printer logic for
 * specific OpenMP clauses: for, parallel, sections, and taskgroup.
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test-omp-clauses.c
 * The generated AST nodes should be visited by the pretty-printer when
 * dumping the intermediate representations.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function to test 'for' and 'parallel' clauses in combination */
static void test_for_and_parallel(void) {
    int i;
    int local_sum = 0;

    /* This pragma should generate OMP_CLAUSE_FOR and OMP_CLAUSE_PARALLEL nodes */
    #pragma omp target teams distribute parallel for map(tofrom: local_sum) private(i) reduction(+:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }

    /* Another variant: parallel for inside target */
    #pragma omp target parallel for private(i) reduction(+:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i] * 2;
    }

    global_sum += local_sum;
}

/* Function to test 'sections' clause */
static void test_sections(void) {
    int section_a = 0, section_b = 0;
    int i;

    /* This pragma should generate OMP_CLAUSE_SECTIONS node */
    #pragma omp target teams sections private(i) reduction(+:section_a, section_b)
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

    global_sum += section_a + section_b;
}

/* Function to test 'taskgroup' clause */
static void test_taskgroup(void) {
    int task_sum = 0;
    int i;

    /* taskgroup must appear inside a parallel region */
    #pragma omp target parallel private(i)
    {
        #pragma omp single
        {
            /* This pragma should generate OMP_CLAUSE_TASKGROUP node */
            #pragma omp taskgroup task_reduction(+:task_sum)
            {
                for (i = 0; i < N; i++) {
                    #pragma omp task in_reduction(+:task_sum) firstprivate(i)
                    {
                        task_sum += global_array[i];
                    }
                }
            }
        }
    }

    global_sum += task_sum;
}

/* Main function that orchestrates all tests */
int main(void) {
    int i;

    /* Initialize global array */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }

    /* Test 1: for and parallel clauses */
    test_for_and_parallel();

    /* Test 2: sections clause */
    test_sections();

    /* Test 3: taskgroup clause */
    test_taskgroup();

    /* Additional combined construct in main to ensure coverage */
    int final_check = 0;
    #pragma omp target parallel for private(i) reduction(+:final_check)
    for (i = 0; i < N; i++) {
        final_check += global_array[i] * 3;
    }
    global_sum += final_check;

    printf("Final sum: %d\n", global_sum);
    return 0;
}
