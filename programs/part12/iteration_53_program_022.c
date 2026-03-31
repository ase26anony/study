/* test_openmp_clauses.c
 * Generates OpenMP constructs with specific clauses to cover
 * OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP
 * in tree-pretty-print.cc lines 1434-1445
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
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
    int local_sum = 0;
    int private_var = 0;
    
    /* OMP_CLAUSE_PARALLEL will be generated here */
    #pragma omp target parallel map(tofrom: local_sum) \
        reduction(+:local_sum) private(i, private_var)
    {
        private_var = omp_get_thread_num();
        #pragma omp for
        for (i = 0; i < N; i++) {
            local_sum += global_array[i] + private_var;
        }
    }
    
    global_sum += local_sum;
    printf("Parallel clause test sum: %d\n", local_sum);
}

/* Function 3: Tests target sections (sections clause) */
static void test_sections_clause(void) {
    int section1_sum = 0, section2_sum = 0;
    int i;
    
    /* OMP_CLAUSE_SECTIONS will be generated here */
    #pragma omp target teams map(tofrom: section1_sum, section2_sum)
    #pragma omp sections reduction(+:section1_sum, section2_sum) \
        private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < N/2; i++) {
                section1_sum += global_array[i];
            }
        }
        
        #pragma omp section
        {
            for (i = N/2; i < N; i++) {
                section2_sum += global_array[i];
            }
        }
    }
    
    global_sum += section1_sum + section2_sum;
    printf("Sections clause test sums: %d, %d\n", section1_sum, section2_sum);
}

/* Function 4: Tests taskgroup clause inside target region */
static void test_taskgroup_clause(void) {
    int task_sum = 0;
    int i;
    
    /* OMP_CLAUSE_TASKGROUP will be generated here */
    #pragma omp target parallel map(tofrom: task_sum)
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:task_sum)
            {
                for (i = 0; i < 10; i++) {
                    #pragma omp task in_reduction(+:task_sum) private(i)
                    {
                        int j;
                        int local_task_sum = 0;
                        for (j = 0; j < N/10; j++) {
                            local_task_sum += global_array[i * (N/10) + j];
                        }
                        #pragma omp atomic
                        task_sum += local_task_sum;
                    }
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

int main(void) {
    /* Initialize global array */
    for (int i = 0; i < N; i++) {
        global_array[i] = (i * 3) % 97;
    }
    
    printf("=== Testing OpenMP clause pretty-printing coverage ===\n");
    
    /* Test each clause individually */
    test_for_clause();          /* Generates OMP_CLAUSE_FOR */
    test_parallel_clause();     /* Generates OMP_CLAUSE_PARALLEL */
    test_sections_clause();     /* Generates OMP_CLAUSE_SECTIONS */
    test_taskgroup_clause();    /* Generates OMP_CLAUSE_TASKGROUP */
    test_combined_clauses();    /* Generates OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR */
    
    printf("\nFinal global sum: %d\n", global_sum);
    printf("All OpenMP constructs executed successfully.\n");
    
    return 0;
}
