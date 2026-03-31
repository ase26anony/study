/* test-omp-clauses.c
 * This program is designed to trigger the pretty-printing logic for
 * specific OpenMP clauses in tree-pretty-print.cc:
 *   OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test.c
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function 1: Tests 'for' and 'parallel' clauses in combination */
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
            local_sum += global_array[i] * 2;
        }
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function 2: Tests 'sections' clause */
static void test_sections(void) {
    int section_a_sum = 0;
    int section_b_sum = 0;
    int i;
    
    /* Sections clause inside target teams construct */
    #pragma omp target teams
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
    
    #pragma omp atomic
    global_sum += section_a_sum + section_b_sum;
}

/* Function 3: Tests 'taskgroup' clause */
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

/* Function 4: Tests all clauses in nested contexts */
static void test_nested_constructs(void) {
    int nested_sum = 0;
    int i, j;
    
    /* Complex nesting with multiple clauses */
    #pragma omp target teams distribute parallel for \
                private(i, j) reduction(+:nested_sum) collapse(2)
    for (i = 0; i < 10; i++) {
        for (j = 0; j < N/10; j++) {
            nested_sum += global_array[i * (N/10) + j];
        }
    }
    
    /* Sections inside parallel region */
    #pragma omp target parallel
    {
        #pragma omp sections private(i) reduction(+:nested_sum)
        {
            #pragma omp section
            for (i = 0; i < N/3; i++) {
                nested_sum += global_array[i];
            }
            
            #pragma omp section
            {
                #pragma omp taskgroup
                {
                    #pragma omp task private(i) shared(nested_sum)
                    {
                        int temp = 0;
                        for (i = N/3; i < 2*N/3; i++) {
                            temp += global_array[i];
                        }
                        #pragma omp atomic
                        nested_sum += temp;
                    }
                }
            }
        }
    }
    
    #pragma omp atomic
    global_sum += nested_sum;
}

int main(void) {
    int i;
    
    /* Initialize global array */
    #pragma omp parallel for private(i)
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    /* Reset global sum */
    global_sum = 0;
    
    /* Call test functions to generate various OpenMP clauses */
    test_for_and_parallel();      /* Generates FOR and PARALLEL clauses */
    test_sections();              /* Generates SECTIONS clause */
    test_taskgroup();             /* Generates TASKGROUP clause */
    test_nested_constructs();     /* Generates all clauses in nested contexts */
    
    /* Verify computation */
    int expected_sum = 0;
    for (i = 0; i < N; i++) {
        expected_sum += global_array[i];
    }
    expected_sum = expected_sum * 3;  /* Each element counted multiple times */
    
    printf("Computed sum: %d\n", global_sum);
    printf("Expected sum: %d\n", expected_sum);
    printf("Verification: %s\n", 
           (global_sum == expected_sum) ? "PASS" : "FAIL");
    
    return (global_sum == expected_sum) ? 0 : 1;
}
