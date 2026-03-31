/* test_openmp_clauses.c
 * This program is designed to trigger the pretty-printing logic for
 * specific OpenMP clauses in tree-pretty-print.cc:
 *   OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test_openmp_clauses.c
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
    
    /* Combined parallel and for clauses in target region */
    #pragma omp target teams distribute parallel for map(tofrom: local_sum) private(i) reduction(+:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    /* Separate parallel clause in target region */
    #pragma omp target parallel private(i) reduction(+:local_sum)
    {
        #pragma omp for
        for (i = 0; i < N/2; i++) {
            local_sum += global_array[i];
        }
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function to test 'sections' clause */
static void test_sections(void) {
    int section_a_sum = 0;
    int section_b_sum = 0;
    int i;
    
    /* Sections clause inside target teams construct */
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
    
    #pragma omp atomic
    global_sum += section_a_sum + section_b_sum;
}

/* Function to test 'taskgroup' clause */
static void test_taskgroup(void) {
    int task_sum = 0;
    int i;
    
    /* Taskgroup inside target parallel region */
    #pragma omp target parallel private(i)
    {
        #pragma omp taskgroup
        {
            #pragma omp task private(i) reduction(+:task_sum)
            {
                for (i = 0; i < N/4; i++) {
                    task_sum += global_array[i];
                }
            }
            
            #pragma omp task private(i) reduction(+:task_sum)
            {
                for (i = N/4; i < N/2; i++) {
                    task_sum += global_array[i];
                }
            }
            
            #pragma omp taskwait
        }
    }
    
    #pragma omp atomic
    global_sum += task_sum;
}

/* Main function with additional clause combinations */
int main(void) {
    int i;
    
    /* Initialize global array */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    /* Test 1: Direct combination of parallel and for clauses */
    #pragma omp target parallel for private(i) reduction(+:global_sum)
    for (i = 0; i < N; i++) {
        global_sum += global_array[i];
    }
    
    /* Test 2: Target with distribute parallel for (for clause) */
    int loop_sum = 0;
    #pragma omp target teams distribute parallel for private(i) reduction(+:loop_sum)
    for (i = 0; i < N; i++) {
        loop_sum += global_array[i] * 2;
    }
    global_sum += loop_sum;
    
    /* Call test functions */
    test_for_and_parallel();
    test_sections();
    test_taskgroup();
    
    /* Final verification computation */
    int final_check = 0;
    #pragma omp target parallel for private(i) reduction(+:final_check)
    for (i = 0; i < N; i++) {
        final_check += global_array[i];
    }
    
    printf("Global sum: %d\n", global_sum);
    printf("Final check: %d\n", final_check);
    
    return 0;
}
