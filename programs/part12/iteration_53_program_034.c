/* test-openmp-clauses.c
 * 
 * This program is designed to trigger the pretty-printing logic for
 * specific OpenMP clauses in tree-pretty-print.cc:
 *   OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, 
 *   OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP
 *
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test-openmp-clauses.c
 * Additional dump flags: -fdump-tree-all, -fdump-tree-gimple
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
    int section1_sum = 0, section2_sum = 0;
    int i;
    
    /* Sections clause inside target teams construct */
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
    
    #pragma omp atomic
    global_sum += section1_sum + section2_sum;
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
            #pragma omp taskgroup task_reduction(+:task_sum)
            {
                #pragma omp task in_reduction(+:task_sum) private(i)
                for (i = 0; i < N/4; i++) {
                    task_sum += global_array[i];
                }
                
                #pragma omp task in_reduction(+:task_sum) private(i)
                for (i = N/4; i < N/2; i++) {
                    task_sum += global_array[i];
                }
                
                #pragma omp taskwait
            }
        }
    }
    
    #pragma omp atomic
    global_sum += task_sum;
}

/* Function 4: Complex nesting with multiple clauses */
static void test_nested_clauses(void) {
    int nested_sum = 0;
    int i, j;
    
    /* Nested constructs with for clause at inner level */
    #pragma omp target teams distribute parallel for \
                private(i, j) reduction(+:nested_sum) collapse(2)
    for (i = 0; i < 10; i++) {
        for (j = 0; j < N/10; j++) {
            nested_sum += global_array[i * (N/10) + j];
        }
    }
    
    /* Sections with internal parallel for */
    #pragma omp target teams
    #pragma omp sections private(i) reduction(+:nested_sum)
    {
        #pragma omp section
        {
            #pragma omp parallel for private(i) reduction(+:nested_sum)
            for (i = 0; i < N/3; i++) {
                nested_sum += global_array[i];
            }
        }
        
        #pragma omp section
        {
            #pragma omp parallel for private(i) reduction(+:nested_sum)
            for (i = N/3; i < 2*N/3; i++) {
                nested_sum += global_array[i];
            }
        }
    }
    
    #pragma omp atomic
    global_sum += nested_sum;
}

int main(void) {
    int i;
    
    /* Initialize array with predictable values */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    /* Reset global sum */
    global_sum = 0;
    
    /* Execute all test functions to generate various OpenMP clauses */
    test_for_and_parallel();      /* Generates FOR and PARALLEL clauses */
    test_sections();              /* Generates SECTIONS clause */
    test_taskgroup();             /* Generates TASKGROUP clause */
    test_nested_clauses();        /* Generates multiple clauses in nested contexts */
    
    /* Verify computation (optional, for runtime validation) */
    int expected_sum = 0;
    for (i = 0; i < N; i++) {
        expected_sum += global_array[i];
    }
    expected_sum *= 4;  /* Each test function adds the array sum once */
    
    printf("Computed sum: %d\n", global_sum);
    printf("Expected sum: %d\n", expected_sum);
    printf("Verification: %s\n", (global_sum == expected_sum) ? "PASS" : "FAIL");
    
    return 0;
}
