/* test_openmp_clauses.c
 * Generates OpenMP constructs with clauses that create OMP_CLAUSE_FOR,
 * OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP nodes.
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test_openmp_clauses.c
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function to test target parallel for clause */
static void test_target_parallel_for(void)
{
    int i;
    int local_sum = 0;
    
    /* This should generate OMP_CLAUSE_FOR and OMP_CLAUSE_PARALLEL nodes */
    #pragma omp target teams distribute parallel for \
                map(tofrom: local_sum) private(i) reduction(+:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function to test target parallel clause */
static void test_target_parallel(void)
{
    int i;
    int local_result = 0;
    
    /* This should generate OMP_CLAUSE_PARALLEL node */
    #pragma omp target parallel private(i) reduction(+:local_result)
    {
        #pragma omp for
        for (i = 0; i < N/2; i++) {
            local_result += global_array[i] * 2;
        }
    }
    
    #pragma omp atomic
    global_sum += local_result;
}

/* Function to test target sections clause */
static void test_target_sections(void)
{
    int section1_sum = 0, section2_sum = 0;
    
    /* This should generate OMP_CLAUSE_SECTIONS node */
    #pragma omp target teams
    {
        #pragma omp sections reduction(+:section1_sum, section2_sum)
        {
            #pragma omp section
            {
                for (int i = 0; i < N/2; i++) {
                    section1_sum += global_array[i];
                }
            }
            
            #pragma omp section
            {
                for (int j = N/2; j < N; j++) {
                    section2_sum += global_array[j];
                }
            }
        }
    }
    
    #pragma omp atomic
    global_sum += section1_sum + section2_sum;
}

/* Function to test taskgroup clause */
static void test_taskgroup(void)
{
    int task_sum = 0;
    
    /* Outer target parallel region */
    #pragma omp target parallel
    {
        /* This should generate OMP_CLAUSE_TASKGROUP node */
        #pragma omp taskgroup task_reduction(+:task_sum)
        {
            #pragma omp task in_reduction(+:task_sum)
            {
                int temp = 0;
                for (int i = 0; i < N/4; i++) {
                    temp += global_array[i];
                }
                #pragma omp atomic
                task_sum += temp;
            }
            
            #pragma omp task in_reduction(+:task_sum)
            {
                int temp = 0;
                for (int i = N/4; i < N/2; i++) {
                    temp += global_array[i];
                }
                #pragma omp atomic
                task_sum += temp;
            }
        }
    }
    
    #pragma omp atomic
    global_sum += task_sum;
}

/* Function combining multiple clauses */
static void test_combined_clauses(void)
{
    int combined_sum = 0;
    
    /* Combined parallel and for clauses in target region */
    #pragma omp target parallel for private(i) reduction(+:combined_sum) \
                map(tofrom: combined_sum)
    for (int i = 0; i < N; i++) {
        combined_sum += global_array[i] * 3;
    }
    
    #pragma omp atomic
    global_sum += combined_sum;
}

int main(void)
{
    /* Initialize array with predictable values */
    for (int i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    /* Reset global sum */
    global_sum = 0;
    
    /* Test each clause type */
    test_target_parallel_for();      /* Generates FOR and PARALLEL clauses */
    test_target_parallel();          /* Generates PARALLEL clause */
    test_target_sections();          /* Generates SECTIONS clause */
    test_taskgroup();                /* Generates TASKGROUP clause */
    test_combined_clauses();         /* Generates combined PARALLEL and FOR clauses */
    
    /* Verify computation */
    int expected_sum = 0;
    for (int i = 0; i < N; i++) {
        expected_sum += global_array[i];                     /* from test_target_parallel_for */
        expected_sum += (i < N/2) ? global_array[i] * 2 : 0; /* from test_target_parallel */
        expected_sum += global_array[i];                     /* from test_target_sections */
        expected_sum += (i < N/2) ? global_array[i] : 0;     /* from test_taskgroup */
        expected_sum += global_array[i] * 3;                 /* from test_combined_clauses */
    }
    
    printf("Computed sum: %d\n", global_sum);
    printf("Expected sum: %d\n", expected_sum);
    printf("Verification: %s\n", (global_sum == expected_sum) ? "PASS" : "FAIL");
    
    return (global_sum == expected_sum) ? 0 : 1;
}
