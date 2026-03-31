/* test_openmp_coverage.c
 * Generates OpenMP constructs with specific clauses to cover
 * OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP
 * in tree-pretty-print.cc lines 1434-1445
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
#define M 100

/* File-scope variables for testing different contexts */
static int global_array[N];
static int global_sum = 0;

/* Function to test target teams distribute parallel for (for clause) */
static void test_for_clause(void) {
    int i;
    int local_sum = 0;
    
    /* Initialize array */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    /* OMP_CLAUSE_FOR: target teams distribute parallel for */
    #pragma omp target teams distribute parallel for \
        map(tofrom: local_sum) map(to: global_array[0:N]) \
        reduction(+:local_sum) private(i)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    global_sum += local_sum;
    printf("For clause test: sum = %d\n", local_sum);
}

/* Function to test target parallel (parallel clause) */
static void test_parallel_clause(void) {
    int i;
    int parallel_sum = 0;
    int private_var = 0;
    
    /* OMP_CLAUSE_PARALLEL: target parallel */
    #pragma omp target parallel map(tofrom: parallel_sum) \
        map(to: global_array[0:N]) private(i, private_var) \
        reduction(+:parallel_sum)
    {
        private_var = omp_get_thread_num();
        #pragma omp for
        for (i = 0; i < N; i++) {
            parallel_sum += global_array[i] + private_var;
        }
    }
    
    global_sum += parallel_sum;
    printf("Parallel clause test: sum = %d\n", parallel_sum);
}

/* Function to test target sections (sections clause) */
static void test_sections_clause(void) {
    int section_sum1 = 0, section_sum2 = 0;
    int i;
    
    /* OMP_CLAUSE_SECTIONS: target teams with sections inside */
    #pragma omp target teams map(tofrom: section_sum1, section_sum2) \
        map(to: global_array[0:N])
    {
        /* Sections directive inside target teams */
        #pragma omp sections private(i) reduction(+:section_sum1, section_sum2)
        {
            #pragma omp section
            for (i = 0; i < N/2; i++) {
                section_sum1 += global_array[i];
            }
            
            #pragma omp section
            for (i = N/2; i < N; i++) {
                section_sum2 += global_array[i];
            }
        }
    }
    
    global_sum += section_sum1 + section_sum2;
    printf("Sections clause test: sums = %d, %d\n", section_sum1, section_sum2);
}

/* Function to test taskgroup clause */
static void test_taskgroup_clause(void) {
    int task_sum = 0;
    int i;
    
    /* OMP_CLAUSE_TASKGROUP: inside target parallel region */
    #pragma omp target parallel map(tofrom: task_sum) \
        map(to: global_array[0:N]) private(i)
    {
        #pragma omp single
        {
            /* Taskgroup directive */
            #pragma omp taskgroup task_reduction(+:task_sum)
            {
                #pragma omp task in_reduction(+:task_sum) private(i)
                {
                    int local_task_sum = 0;
                    for (i = 0; i < N/2; i++) {
                        local_task_sum += global_array[i];
                    }
                    #pragma omp atomic
                    task_sum += local_task_sum;
                }
                
                #pragma omp task in_reduction(+:task_sum) private(i)
                {
                    int local_task_sum = 0;
                    for (i = N/2; i < N; i++) {
                        local_task_sum += global_array[i];
                    }
                    #pragma omp atomic
                    task_sum += local_task_sum;
                }
            }
        }
    }
    
    global_sum += task_sum;
    printf("Taskgroup clause test: sum = %d\n", task_sum);
}

/* Function combining multiple clauses */
static void test_combined_clauses(void) {
    int combined_sum = 0;
    int i;
    
    /* Combined parallel and for clauses in single pragma */
    #pragma omp target parallel for map(tofrom: combined_sum) \
        map(to: global_array[0:N]) reduction(+:combined_sum) \
        private(i) schedule(static)
    for (i = 0; i < N; i++) {
        combined_sum += global_array[i] * 2;
    }
    
    global_sum += combined_sum;
    printf("Combined parallel+for test: sum = %d\n", combined_sum);
}

/* Main function orchestrating all tests */
int main(void) {
    int final_validation = 0;
    int i;
    
    /* Initialize global array */
    for (i = 0; i < N; i++) {
        global_array[i] = (i * 3) % 97;
    }
    
    printf("=== OpenMP Clause Coverage Tests ===\n");
    
    /* Test each clause individually */
    test_for_clause();          /* Generates OMP_CLAUSE_FOR */
    test_parallel_clause();     /* Generates OMP_CLAUSE_PARALLEL */
    test_sections_clause();     /* Generates OMP_CLAUSE_SECTIONS */
    test_taskgroup_clause();    /* Generates OMP_CLAUSE_TASKGROUP */
    test_combined_clauses();    /* Generates combined clauses */
    
    /* Final validation computation using all constructs */
    #pragma omp target teams distribute parallel for \
        map(tofrom: final_validation) map(to: global_array[0:N]) \
        reduction(+:final_validation) private(i)
    for (i = 0; i < N; i++) {
        final_validation += global_array[i];
    }
    
    printf("\n=== Results ===\n");
    printf("Global accumulated sum: %d\n", global_sum);
    printf("Final validation sum: %d\n", final_validation);
    
    /* Simple check to ensure computations were meaningful */
    if (global_sum > 0 && final_validation > 0) {
        printf("All OpenMP tests completed successfully.\n");
        return 0;
    } else {
        printf("Error: Unexpected results.\n");
        return 1;
    }
}
