/* test_openmp_clauses.c
 * This program is designed to trigger the pretty-printer logic for
 * specific OpenMP clauses: for, parallel, sections, and taskgroup.
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test_openmp_clauses.c
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function to test 'for' and 'parallel' clauses in combination */
static void test_for_and_parallel(void) {
    int local_sum = 0;
    int i;

    /* Combined parallel and for clause in target region */
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
    int section_sum_a = 0, section_sum_b = 0;
    int i;

    /* Sections clause inside target teams construct */
    #pragma omp target teams map(tofrom: section_sum_a, section_sum_b)
    #pragma omp distribute parallel for private(i) reduction(+:section_sum_a)
    for (i = 0; i < N; i++) {
        section_sum_a += global_array[i];
    }

    #pragma omp target sections
    {
        #pragma omp section
        {
            for (i = 0; i < N/2; i++) {
                section_sum_b += global_array[i];
            }
        }
        #pragma omp section
        {
            for (i = N/2; i < N; i++) {
                section_sum_b += global_array[i];
            }
        }
    }

    #pragma omp atomic
    global_sum += section_sum_a + section_sum_b;
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
        }
    }

    #pragma omp atomic
    global_sum += task_sum;
}

/* Main function with mixed constructs */
int main(void) {
    int i;
    
    /* Initialize array */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }

    /* Test all clause combinations */
    test_for_and_parallel();
    test_sections();
    test_taskgroup();

    /* Additional nested constructs in main */
    int main_sum = 0;
    
    /* Parallel for in target region */
    #pragma omp target teams distribute parallel for map(tofrom: main_sum) private(i) reduction(+:main_sum)
    for (i = 0; i < N; i++) {
        main_sum += global_array[i];
    }
    
    /* Sections in target region */
    #pragma omp target sections
    {
        #pragma omp section
        {
            for (i = 0; i < N/2; i++) {
                main_sum += global_array[i];
            }
        }
        #pragma omp section
        {
            for (i = N/2; i < N; i++) {
                main_sum += global_array[i];
            }
        }
    }

    /* Taskgroup with nested tasks */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    for (i = 0; i < N/4; i++) {
                        #pragma omp atomic
                        main_sum += global_array[i];
                    }
                }
                #pragma omp task
                {
                    for (i = N/4; i < N/2; i++) {
                        #pragma omp atomic
                        main_sum += global_array[i];
                    }
                }
            }
        }
    }

    global_sum += main_sum;
    
    printf("Final sum: %d\n", global_sum);
    return 0;
}
