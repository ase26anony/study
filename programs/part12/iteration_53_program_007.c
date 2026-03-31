/* test_openmp_clauses.c
 * Generates OpenMP constructs with clauses: for, parallel, sections, taskgroup
 * to trigger pretty-printing of OMP_CLAUSE_* nodes in tree-pretty-print.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1000
#define M 100

/* Global variables for testing different scopes */
static int global_array[N];
static int global_sum = 0;

/* Function 1: Tests 'for' and 'parallel' clauses in combination */
static void test_for_parallel_clauses(void) {
    int i;
    int local_sum = 0;
    int local_array[M];
    
    /* Initialize local array */
    for (i = 0; i < M; i++) {
        local_array[i] = i + 1;
    }
    
    /* TARGET 1: Generate OMP_CLAUSE_FOR 
     * Using target teams distribute parallel for */
    #pragma omp target teams distribute parallel for \
        map(to: local_array[0:M]) map(tofrom: local_sum) \
        private(i) reduction(+:local_sum)
    for (i = 0; i < M; i++) {
        local_sum += local_array[i];
    }
    
    /* TARGET 2: Generate OMP_CLAUSE_PARALLEL 
     * Using target parallel */
    #pragma omp target parallel map(to: local_array[0:M]) \
        map(tofrom: local_sum) private(i) reduction(+:local_sum)
    {
        #pragma omp for
        for (i = 0; i < M; i++) {
            local_sum += local_array[i] * 2;
        }
    }
    
    /* TARGET 3: Generate both 'parallel' and 'for' in single pragma */
    #pragma omp target parallel for \
        map(to: local_array[0:M]) map(tofrom: local_sum) \
        private(i) reduction(+:local_sum)
    for (i = 0; i < M; i++) {
        local_sum += local_array[i] * 3;
    }
    
    printf("test_for_parallel_clauses: local_sum = %d\n", local_sum);
}

/* Function 2: Tests 'sections' clause */
static void test_sections_clause(void) {
    int section1_sum = 0, section2_sum = 0;
    int i;
    
    /* Initialize global array */
    #pragma omp parallel for private(i)
    for (i = 0; i < N; i++) {
        global_array[i] = (i % 10) + 1;
    }
    
    /* TARGET 4: Generate OMP_CLAUSE_SECTIONS 
     * Using target sections inside target teams */
    #pragma omp target teams map(to: global_array[0:N]) \
        map(tofrom: section1_sum, section2_sum)
    {
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
    }
    
    printf("test_sections_clause: section1_sum = %d, section2_sum = %d\n", 
           section1_sum, section2_sum);
}

/* Function 3: Tests 'taskgroup' clause */
static void test_taskgroup_clause(void) {
    int task_results[4] = {0};
    int i, j;
    
    /* TARGET 5: Generate OMP_CLAUSE_TASKGROUP 
     * Using taskgroup inside target parallel region */
    #pragma omp target parallel map(tofrom: task_results[0:4]) \
        private(i, j)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (j = 0; j < 4; j++) {
                    #pragma omp task firstprivate(j) shared(task_results)
                    {
                        int local_result = 0;
                        for (i = 0; i < 250; i++) {
                            local_result += (i + j) % 7;
                        }
                        task_results[j] = local_result;
                    }
                }
            }
        }
    }
    
    int total = 0;
    for (i = 0; i < 4; i++) {
        total += task_results[i];
    }
    printf("test_taskgroup_clause: total = %d\n", total);
}

/* Function 4: Tests nested combinations */
static void test_nested_combinations(void) {
    int matrix[10][10];
    int row_sums[10] = {0};
    int i, j;
    
    /* Initialize matrix */
    #pragma omp parallel for private(i, j) collapse(2)
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Complex nesting with multiple clauses */
    #pragma omp target teams map(to: matrix[0:10][0:10]) \
        map(tofrom: row_sums[0:10]) private(i, j)
    {
        #pragma omp distribute parallel for private(j) \
            reduction(+:row_sums[:10])
        for (i = 0; i < 10; i++) {
            for (j = 0; j < 10; j++) {
                row_sums[i] += matrix[i][j];
            }
        }
        
        /* Another sections inside teams */
        #pragma omp sections private(i)
        {
            #pragma omp section
            {
                #pragma omp parallel for reduction(+:global_sum)
                for (i = 0; i < 10; i++) {
                    global_sum += row_sums[i];
                }
            }
            
            #pragma omp section
            {
                int temp = 0;
                #pragma omp parallel for reduction(+:temp)
                for (i = 0; i < 10; i++) {
                    temp += row_sums[i] % 100;
                }
                printf("  Nested sections temp = %d\n", temp);
            }
        }
    }
    
    printf("test_nested_combinations: global_sum = %d\n", global_sum);
}

/* Main function orchestrates all tests */
int main(void) {
    printf("Starting OpenMP clause coverage tests...\n");
    
    /* Test 1: for and parallel clauses */
    test_for_parallel_clauses();
    
    /* Test 2: sections clause */
    test_sections_clause();
    
    /* Test 3: taskgroup clause */
    test_taskgroup_clause();
    
    /* Test 4: Nested combinations */
    test_nested_combinations();
    
    /* Final verification computation */
    int final_check = 0;
    #pragma omp target teams distribute parallel for \
        map(to: global_array[0:N]) map(tofrom: final_check) \
        reduction(+:final_check)
    for (int i = 0; i < N; i++) {
        final_check += global_array[i];
    }
    
    printf("Final check sum = %d\n", final_check);
    printf("All tests completed.\n");
    
    return 0;
}
