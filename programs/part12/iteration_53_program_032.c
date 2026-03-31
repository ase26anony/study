/* test-omp-clauses.c
 * This program is designed to trigger the pretty-printing of specific
 * OpenMP clause nodes (OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP) in tree-pretty-print.cc.
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test.c
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function 1: Demonstrates 'for' and 'parallel' clauses in combination */
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

/* Function 2: Demonstrates 'sections' clause */
static void test_sections(void) {
    int section_a_sum = 0;
    int section_b_sum = 0;
    int i;
    
    /* Sections clause inside target teams construct */
    #pragma omp target teams
    #pragma omp distribute parallel for private(i) reduction(+:section_a_sum)
    for (i = 0; i < N; i++) {
        section_a_sum += global_array[i];
    }
    
    /* Direct sections clause in target region */
    #pragma omp target sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < N/2; i++) {
                section_b_sum += global_array[i];
            }
        }
        #pragma omp section
        {
            for (i = N/2; i < N; i++) {
                section_b_sum += global_array[i];
            }
        }
    }
    
    #pragma omp atomic
    global_sum += section_a_sum + section_b_sum;
}

/* Function 3: Demonstrates 'taskgroup' clause nested in parallel region */
static void test_taskgroup(void) {
    int task_sum = 0;
    int i;
    
    /* Taskgroup inside target parallel region */
    #pragma omp target parallel private(i)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task private(i) shared(task_sum)
                {
                    int local_task_sum = 0;
                    for (i = 0; i < N/4; i++) {
                        local_task_sum += global_array[i];
                    }
                    #pragma omp atomic
                    task_sum += local_task_sum;
                }
                
                #pragma omp task private(i) shared(task_sum)
                {
                    int local_task_sum = 0;
                    for (i = N/4; i < N/2; i++) {
                        local_task_sum += global_array[i];
                    }
                    #pragma omp atomic
                    task_sum += local_task_sum;
                }
            } /* end taskgroup */
        } /* end single */
    } /* end target parallel */
    
    #pragma omp atomic
    global_sum += task_sum;
}

/* Function 4: Complex combination of all target clauses */
static void test_combined_clauses(void) {
    int combined_sum = 0;
    int i;
    
    /* All four clauses in various combinations */
    
    /* 1. Target with parallel for */
    #pragma omp target parallel for private(i) reduction(+:combined_sum)
    for (i = 0; i < N; i++) {
        combined_sum += global_array[i];
    }
    
    /* 2. Target sections */
    #pragma omp target sections private(i) reduction(+:combined_sum)
    {
        #pragma omp section
        {
            for (i = 0; i < N/3; i++) {
                combined_sum += global_array[i];
            }
        }
        #pragma omp section
        {
            for (i = N/3; i < 2*N/3; i++) {
                combined_sum += global_array[i];
            }
        }
        #pragma omp section
        {
            for (i = 2*N/3; i < N; i++) {
                combined_sum += global_array[i];
            }
        }
    }
    
    /* 3. Taskgroup inside another parallel region */
    #pragma omp target parallel private(i)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task private(i)
                {
                    int tmp = 0;
                    for (i = 0; i < 10; i++) {
                        tmp += global_array[i];
                    }
                    #pragma omp atomic
                    combined_sum += tmp;
                }
            }
        }
    }
    
    #pragma omp atomic
    global_sum += combined_sum;
}

int main(void) {
    int i;
    
    /* Initialize array with predictable values */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    /* Execute all test functions to generate the OpenMP clauses */
    test_for_and_parallel();
    test_sections();
    test_taskgroup();
    test_combined_clauses();
    
    /* Verify the computation */
    int expected_sum = 0;
    for (i = 0; i < N; i++) {
        expected_sum += global_array[i];
    }
    expected_sum *= 4; /* Each test function adds the full sum once */
    
    printf("Computed sum: %d\n", global_sum);
    printf("Expected sum: %d\n", expected_sum);
    printf("Verification: %s\n", global_sum == expected_sum ? "PASS" : "FAIL");
    
    return global_sum == expected_sum ? 0 : 1;
}
