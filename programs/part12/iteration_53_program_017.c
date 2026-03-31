/* test-openmp-clauses.c
 * 
 * This program is designed to trigger the pretty-printer logic for
 * specific OpenMP clauses (for, parallel, sections, taskgroup) in
 * tree-pretty-print.cc lines 1434-1445.
 * 
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test-openmp-clauses.c -o test-openmp-clauses
 * 
 * The -fdump-tree-* flags will generate dump files containing the AST
 * where the pretty-printer will be invoked for the OMP_CLAUSE nodes.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function to test 'for' and 'parallel' clauses in combination */
static void test_for_and_parallel(void)
{
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
        for (i = 0; i < N; i++) {
            local_sum += global_array[i];
        }
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function to test 'sections' clause */
static void test_sections(void)
{
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
static void test_taskgroup(void)
{
    int task_sum = 0;
    int i;
    
    /* Taskgroup inside target parallel region */
    #pragma omp target parallel private(i)
    {
        #pragma omp taskgroup task_reduction(+:task_sum)
        {
            #pragma omp task in_reduction(+:task_sum) private(i)
            {
                int local_task_sum = 0;
                for (i = 0; i < N/4; i++) {
                    local_task_sum += global_array[i];
                }
                #pragma omp atomic
                task_sum += local_task_sum;
            }
            
            #pragma omp task in_reduction(+:task_sum) private(i)
            {
                int local_task_sum = 0;
                for (i = N/4; i < N/2; i++) {
                    local_task_sum += global_array[i];
                }
                #pragma omp atomic
                task_sum += local_task_sum;
            }
        }
    }
    
    #pragma omp atomic
    global_sum += task_sum;
}

/* Function with nested constructs for clause combination */
static void test_nested_combinations(void)
{
    int i;
    int nested_sum = 0;
    
    /* Nested: taskgroup inside sections inside target parallel */
    #pragma omp target parallel private(i) reduction(+:nested_sum)
    {
        #pragma omp sections
        {
            #pragma omp section
            {
                #pragma omp taskgroup task_reduction(+:nested_sum)
                {
                    #pragma omp task in_reduction(+:nested_sum)
                    {
                        for (i = 0; i < N/8; i++) {
                            nested_sum += global_array[i];
                        }
                    }
                }
            }
            
            #pragma omp section
            {
                #pragma omp for
                for (i = N/8; i < N/4; i++) {
                    nested_sum += global_array[i];
                }
            }
        }
    }
    
    #pragma omp atomic
    global_sum += nested_sum;
}

int main(void)
{
    int i;
    
    /* Initialize global array */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    /* Reset global sum */
    global_sum = 0;
    
    /* Test all clause types in different functions */
    test_for_and_parallel();      /* Triggers OMP_CLAUSE_FOR and OMP_CLAUSE_PARALLEL */
    test_sections();              /* Triggers OMP_CLAUSE_SECTIONS */
    test_taskgroup();             /* Triggers OMP_CLAUSE_TASKGROUP */
    test_nested_combinations();   /* Triggers multiple clauses in nested context */
    
    /* Additional direct tests in main() */
    int main_sum = 0;
    
    /* Direct target parallel for with for clause */
    #pragma omp target teams distribute parallel for \
                map(tofrom: main_sum) private(i) reduction(+:main_sum)
    for (i = 0; i < N; i++) {
        main_sum += global_array[i];
    }
    
    /* Direct target sections with sections clause */
    #pragma omp target teams
    {
        #pragma omp sections private(i) reduction(+:main_sum)
        {
            #pragma omp section
            for (i = 0; i < N/2; i++) {
                main_sum += global_array[i];
            }
            #pragma omp section
            for (i = N/2; i < N; i++) {
                main_sum += global_array[i];
            }
        }
    }
    
    /* Direct taskgroup in target region */
    int taskgroup_sum = 0;
    #pragma omp target parallel private(i)
    {
        #pragma omp taskgroup task_reduction(+:taskgroup_sum)
        {
            #pragma omp task in_reduction(+:taskgroup_sum) private(i)
            {
                for (i = 0; i < N/4; i++) {
                    taskgroup_sum += global_array[i];
                }
            }
        }
    }
    
    main_sum += taskgroup_sum;
    global_sum += main_sum;
    
    /* Print result to ensure code is not optimized away */
    printf("Final sum: %d\n", global_sum);
    
    /* Expected value: Each element is i%100, sum of 0..999 mod 100
     * = 99 * (0+1+...+99) + (0+1+...+99) = 100 * (99*100/2) = 495000
     * Multiplied by number of accumulations in the test functions
     */
    
    return 0;
}
