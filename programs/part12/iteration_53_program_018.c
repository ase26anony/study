/* test_openmp_clauses.c
 * Generates OpenMP constructs with specific clauses to cover
 * OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP
 * in tree-pretty-print.cc lines 1434-1445
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
#define M 100

/* File-scope variables for testing data environment */
static int global_array[N];
static int global_sum = 0;

/* Function 1: Tests target teams distribute parallel for (for clause) */
static void test_for_clause(void) {
    int i;
    int local_sum = 0;
    
    /* Initialize array */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    /* OMP_CLAUSE_FOR will be generated here */
    #pragma omp target teams distribute parallel for map(tofrom: local_sum) \
        reduction(+:local_sum) private(i)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    global_sum += local_sum;
    printf("For clause test sum: %d\n", local_sum);
}

/* Function 2: Tests target parallel (parallel clause) */
static void test_parallel_clause(void) {
    int i;
    int parallel_sum = 0;
    int private_var = 0;
    
    /* OMP_CLAUSE_PARALLEL will be generated here */
    #pragma omp target parallel map(tofrom: parallel_sum) \
        reduction(+:parallel_sum) private(i, private_var)
    {
        private_var = omp_get_thread_num();
        #pragma omp for
        for (i = 0; i < M; i++) {
            parallel_sum += i;
        }
    }
    
    global_sum += parallel_sum;
    printf("Parallel clause test sum: %d\n", parallel_sum);
}

/* Function 3: Tests target sections (sections clause) */
static void test_sections_clause(void) {
    int section_sum1 = 0, section_sum2 = 0;
    int i;
    
    /* OMP_CLAUSE_SECTIONS will be generated here */
    #pragma omp target teams map(tofrom: section_sum1, section_sum2)
    #pragma omp distribute
    for (i = 0; i < 1; i++) {  /* Dummy loop to attach distribute */
        #pragma omp sections reduction(+:section_sum1, section_sum2)
        {
            #pragma omp section
            {
                for (int j = 0; j < M; j++) {
                    section_sum1 += j * 2;
                }
            }
            #pragma omp section
            {
                for (int j = 0; j < M; j++) {
                    section_sum2 += j * 3;
                }
            }
        }
    }
    
    global_sum += section_sum1 + section_sum2;
    printf("Sections clause test sums: %d, %d\n", section_sum1, section_sum2);
}

/* Function 4: Tests taskgroup clause inside target region */
static void test_taskgroup_clause(void) {
    int task_sum = 0;
    
    /* OMP_CLAUSE_TASKGROUP will be generated here */
    #pragma omp target parallel map(tofrom: task_sum)
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:task_sum)
            {
                #pragma omp task in_reduction(+:task_sum)
                {
                    int local = 0;
                    for (int i = 0; i < 50; i++) {
                        local += i;
                    }
                    #pragma omp atomic
                    task_sum += local;
                }
                
                #pragma omp task in_reduction(+:task_sum)
                {
                    int local = 0;
                    for (int i = 50; i < 100; i++) {
                        local += i;
                    }
                    #pragma omp atomic
                    task_sum += local;
                }
            }
        }
    }
    
    global_sum += task_sum;
    printf("Taskgroup clause test sum: %d\n", task_sum);
}

/* Function 5: Tests combined parallel and for clauses */
static void test_combined_clauses(void) {
    int combined_sum = 0;
    int i;
    
    /* Both OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR will be generated here */
    #pragma omp target parallel for map(tofrom: combined_sum) \
        reduction(+:combined_sum) private(i)
    for (i = 0; i < N; i++) {
        combined_sum += global_array[i] * 2;
    }
    
    global_sum += combined_sum;
    printf("Combined parallel+for test sum: %d\n", combined_sum);
}

/* Main function orchestrates all tests */
int main(void) {
    printf("=== Testing OpenMP clause pretty-printing coverage ===\n");
    
    /* Initialize global array */
    for (int i = 0; i < N; i++) {
        global_array[i] = i % 50;
    }
    
    /* Run all test functions */
    test_for_clause();          /* Generates OMP_CLAUSE_FOR */
    test_parallel_clause();     /* Generates OMP_CLAUSE_PARALLEL */
    test_sections_clause();     /* Generates OMP_CLAUSE_SECTIONS */
    test_taskgroup_clause();    /* Generates OMP_CLAUSE_TASKGROUP */
    test_combined_clauses();    /* Generates OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR */
    
    printf("\nFinal global sum: %d\n", global_sum);
    printf("All OpenMP constructs executed successfully.\n");
    
    return 0;
}
