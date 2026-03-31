/* test_omp_clauses.c
 * Generates OpenMP constructs with for, parallel, sections, and taskgroup clauses
 * to trigger pretty-printing of OMP_CLAUSE_* nodes in tree-pretty-print.cc
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function 1: Tests target parallel for with for clause */
static void test_for_clause(void) {
    int i;
    int local_sum = 0;
    
    /* This should generate OMP_CLAUSE_FOR */
    #pragma omp target teams distribute parallel for map(tofrom: local_sum) \
        reduction(+:local_sum) private(i)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function 2: Tests target parallel with parallel clause */
static void test_parallel_clause(void) {
    int i;
    int local_sum = 0;
    
    /* This should generate OMP_CLAUSE_PARALLEL */
    #pragma omp target parallel map(tofrom: local_sum) \
        reduction(+:local_sum) private(i)
    {
        #pragma omp for
        for (i = 0; i < N; i++) {
            local_sum += global_array[i] * 2;
        }
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function 3: Tests target sections with sections clause */
static void test_sections_clause(void) {
    int sum1 = 0, sum2 = 0;
    int i;
    
    /* This should generate OMP_CLAUSE_SECTIONS */
    #pragma omp target teams map(tofrom: sum1, sum2)
    #pragma omp sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < N/2; i++) {
                sum1 += global_array[i];
            }
        }
        
        #pragma omp section
        {
            for (i = N/2; i < N; i++) {
                sum2 += global_array[i];
            }
        }
    }
    
    #pragma omp atomic
    global_sum += sum1 + sum2;
}

/* Function 4: Tests taskgroup clause inside target region */
static void test_taskgroup_clause(void) {
    int task_sum = 0;
    int i;
    
    /* Outer target parallel region */
    #pragma omp target parallel map(tofrom: task_sum) private(i)
    {
        /* This should generate OMP_CLAUSE_TASKGROUP */
        #pragma omp taskgroup
        {
            #pragma omp task private(i) shared(task_sum)
            {
                int local_task_sum = 0;
                for (i = 0; i < N; i++) {
                    local_task_sum += global_array[i] / 2;
                }
                #pragma omp atomic
                task_sum += local_task_sum;
            }
            
            #pragma omp taskwait
        }
    }
    
    #pragma omp atomic
    global_sum += task_sum;
}

/* Function 5: Tests combined parallel and for clauses */
static void test_combined_clauses(void) {
    int combined_sum = 0;
    int i;
    
    /* Combined parallel and for clauses in single pragma */
    #pragma omp target parallel for map(tofrom: combined_sum) \
        reduction(+:combined_sum) private(i)
    for (i = 0; i < N; i++) {
        combined_sum += global_array[i] * 3;
    }
    
    #pragma omp atomic
    global_sum += combined_sum;
}

/* Main function orchestrates all tests */
int main(void) {
    int i;
    
    /* Initialize array with predictable values */
    for (i = 0; i < N; i++) {
        global_array[i] = (i % 100) + 1;  /* Values 1-100 repeating */
    }
    
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Execute all test functions to generate various OMP_CLAUSE_* nodes */
    test_for_clause();          /* Generates OMP_CLAUSE_FOR */
    test_parallel_clause();     /* Generates OMP_CLAUSE_PARALLEL */
    test_sections_clause();     /* Generates OMP_CLAUSE_SECTIONS */
    test_taskgroup_clause();    /* Generates OMP_CLAUSE_TASKGROUP */
    test_combined_clauses();    /* Generates both OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR */
    
    /* Calculate expected sum for verification */
    int expected_sum = 0;
    for (i = 0; i < N; i++) {
        expected_sum += global_array[i] * 7;  /* Sum from all operations */
    }
    
    printf("Global sum: %d\n", global_sum);
    printf("Expected sum: %d\n", expected_sum);
    printf("Difference: %d\n", global_sum - expected_sum);
    
    if (global_sum == expected_sum) {
        printf("SUCCESS: All OpenMP computations correct\n");
        return 0;
    } else {
        printf("ERROR: Computation mismatch\n");
        return 1;
    }
}
