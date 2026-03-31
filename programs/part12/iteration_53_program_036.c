/* test_openmp_clauses.c
 * 
 * This program is designed to generate OpenMP AST nodes that will trigger
 * the pretty-printer logic for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP in tree-pretty-print.cc.
 * 
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test_openmp_clauses.c
 * Additional flags: -fdump-tree-all, -fdump-tree-gimple
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
    
    /* This generates OMP_CLAUSE_FOR and OMP_CLAUSE_PARALLEL nodes */
    #pragma omp target teams distribute parallel for map(tofrom: local_sum) reduction(+:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    /* Also test combined parallel for */
    #pragma omp target parallel for reduction(+:local_sum) private(i)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i] * 2;
    }
    
    global_sum += local_sum;
}

/* Function 2: Tests 'sections' clause */
static void test_sections(void) {
    int section_a_sum = 0;
    int section_b_sum = 0;
    int i;
    
    /* This generates OMP_CLAUSE_SECTIONS node */
    #pragma omp target teams distribute parallel for simd reduction(+:section_a_sum)
    for (i = 0; i < N/2; i++) {
        section_a_sum += global_array[i];
    }
    
    /* Sections inside target teams */
    #pragma omp target teams
    {
        #pragma omp sections reduction(+:section_b_sum)
        {
            #pragma omp section
            for (i = N/2; i < 3*N/4; i++) {
                section_b_sum += global_array[i];
            }
            #pragma omp section
            for (i = 3*N/4; i < N; i++) {
                section_b_sum += global_array[i] * 2;
            }
        }
    }
    
    global_sum += section_a_sum + section_b_sum;
}

/* Function 3: Tests 'taskgroup' clause */
static void test_taskgroup(void) {
    int task_sum = 0;
    int i;
    
    /* Taskgroup inside target parallel region */
    #pragma omp target parallel private(i)
    {
        #pragma omp taskgroup
        {
            #pragma omp task reduction(+:task_sum)
            {
                for (i = 0; i < N/4; i++) {
                    task_sum += global_array[i];
                }
            }
            
            #pragma omp task reduction(+:task_sum)
            {
                for (i = N/4; i < N/2; i++) {
                    task_sum += global_array[i] * 3;
                }
            }
            
            #pragma omp taskwait
        }
        
        /* Additional parallel work */
        #pragma omp for reduction(+:task_sum)
        for (i = N/2; i < N; i++) {
            task_sum += global_array[i];
        }
    }
    
    global_sum += task_sum;
}

/* Function 4: Complex nesting with multiple clauses */
static void test_nested_constructs(void) {
    int nested_sum = 0;
    int i, j;
    
    /* Nested: target -> teams -> distribute -> parallel for */
    #pragma omp target teams distribute parallel for collapse(2) reduction(+:nested_sum) private(i, j)
    for (i = 0; i < 10; i++) {
        for (j = 0; j < N/10; j++) {
            nested_sum += global_array[i * (N/10) + j];
        }
    }
    
    /* Mixed: target parallel sections */
    #pragma omp target parallel
    {
        #pragma omp sections reduction(+:nested_sum)
        {
            #pragma omp section
            {
                #pragma omp taskgroup
                {
                    #pragma omp task
                    { nested_sum += 1; }
                    #pragma omp task
                    { nested_sum += 2; }
                }
            }
            #pragma omp section
            {
                for (i = 0; i < 10; i++) {
                    nested_sum += i;
                }
            }
        }
    }
    
    global_sum += nested_sum;
}

int main(void) {
    int i;
    
    /* Initialize global array */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    /* Call test functions to generate various OpenMP clauses */
    test_for_and_parallel();      /* Generates FOR and PARALLEL clauses */
    test_sections();              /* Generates SECTIONS clause */
    test_taskgroup();             /* Generates TASKGROUP clause */
    test_nested_constructs();     /* Generates multiple clauses in nested context */
    
    /* Final verification output */
    printf("Final sum: %d\n", global_sum);
    printf("Expected sum should be positive and non-zero\n");
    
    return 0;
}
