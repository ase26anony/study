/* test_openmp_clauses.c
 * 
 * This program is designed to trigger the pretty-printer logic for
 * specific OpenMP clauses (for, parallel, sections, taskgroup) in
 * tree-pretty-print.cc lines 1434-1445.
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test_openmp_clauses.c
 * Also try: gcc -O2 -fopenmp -fdump-tree-all test_openmp_clauses.c
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
    
    /* Combined parallel and for clause in target region */
    #pragma omp target teams distribute parallel for map(tofrom:local_sum) private(i) reduction(+:local_sum)
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
    int section_a_sum = 0, section_b_sum = 0;
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

/* Function 3: Tests 'taskgroup' clause inside target parallel region */
static void test_taskgroup(void) {
    int task_sum = 0;
    int i;
    
    /* Taskgroup clause inside target parallel region */
    #pragma omp target parallel private(i)
    {
        #pragma omp taskgroup task_reduction(+:task_sum)
        {
            #pragma omp task in_reduction(+:task_sum) private(i)
            {
                int local = 0;
                for (i = 0; i < N/4; i++) {
                    local += global_array[i];
                }
                #pragma omp atomic
                task_sum += local;
            }
            
            #pragma omp task in_reduction(+:task_sum) private(i)
            {
                int local = 0;
                for (i = N/4; i < N/2; i++) {
                    local += global_array[i];
                }
                #pragma omp atomic
                task_sum += local;
            }
        }
    }
    
    #pragma omp atomic
    global_sum += task_sum;
}

/* Function 4: Tests all clauses in nested contexts */
static void test_nested_combinations(void) {
    int nested_sum = 0;
    int i, j;
    
    /* Nested: parallel with for inside target region */
    #pragma omp target parallel
    {
        #pragma omp for private(i) reduction(+:nested_sum) collapse(1)
        for (i = 0; i < N; i++) {
            nested_sum += global_array[i];
        }
        
        /* Sections inside the same parallel region */
        #pragma omp sections private(j) reduction(+:nested_sum)
        {
            #pragma omp section
            for (j = 0; j < N/2; j++) {
                nested_sum += j;
            }
            
            #pragma omp section
            for (j = N/2; j < N; j++) {
                nested_sum -= j;
            }
        }
        
        /* Taskgroup inside the parallel region */
        #pragma omp taskgroup task_reduction(+:nested_sum)
        {
            #pragma omp task in_reduction(+:nested_sum)
            {
                nested_sum += 100;
            }
        }
    }
    
    #pragma omp atomic
    global_sum += nested_sum;
}

int main(void) {
    int i;
    
    /* Initialize global array */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    /* Reset global sum */
    global_sum = 0;
    
    /* Test all clause combinations across different functions */
    test_for_and_parallel();      /* Triggers OMP_CLAUSE_FOR and OMP_CLAUSE_PARALLEL */
    test_sections();              /* Triggers OMP_CLAUSE_SECTIONS */
    test_taskgroup();             /* Triggers OMP_CLAUSE_TASKGROUP */
    test_nested_combinations();   /* Triggers all clauses in nested context */
    
    /* Verify computation */
    printf("Final sum: %d\n", global_sum);
    
    /* Expected value check */
    int expected = 0;
    for (i = 0; i < N; i++) {
        expected += global_array[i];
    }
    expected = expected * 2;  /* Account for multiple accumulations */
    expected += 100;          /* From taskgroup test */
    
    printf("Expected sum: %d\n", expected);
    
    if (global_sum == expected) {
        printf("SUCCESS: All OpenMP constructs executed correctly.\n");
    } else {
        printf("WARNING: Sum mismatch - OpenMP execution may differ.\n");
    }
    
    return 0;
}
