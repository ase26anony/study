/* test_openmp_clauses.c
 * Designed to trigger OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP pretty-printing
 * in tree-pretty-print.cc lines 1434-1445
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function to test target parallel for with for clause */
static void test_target_parallel_for(void)
{
    int i;
    int local_sum = 0;
    
    /* This should generate OMP_CLAUSE_FOR and OMP_CLAUSE_PARALLEL */
    #pragma omp target teams distribute parallel for \
                map(tofrom: local_sum) private(i) reduction(+:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function to test target parallel with parallel clause */
static void test_target_parallel(void)
{
    int i;
    int local_sum = 0;
    
    /* This should generate OMP_CLAUSE_PARALLEL */
    #pragma omp target parallel private(i) reduction(+:local_sum) \
                map(tofrom: local_sum)
    {
        #pragma omp for
        for (i = 0; i < N; i++) {
            local_sum += global_array[i];
        }
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function to test target sections with sections clause */
static void test_target_sections(void)
{
    int sum1 = 0, sum2 = 0;
    
    /* This should generate OMP_CLAUSE_SECTIONS */
    #pragma omp target teams map(tofrom: sum1, sum2)
    {
        #pragma omp sections reduction(+:sum1, sum2)
        {
            #pragma omp section
            {
                for (int i = 0; i < N/2; i++) {
                    sum1 += global_array[i];
                }
            }
            
            #pragma omp section
            {
                for (int i = N/2; i < N; i++) {
                    sum2 += global_array[i];
                }
            }
        }
    }
    
    #pragma omp atomic
    global_sum += sum1 + sum2;
}

/* Function to test taskgroup clause */
static void test_taskgroup(void)
{
    int task_result = 0;
    
    /* This should generate OMP_CLAUSE_TASKGROUP */
    #pragma omp target parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:task_result)
            {
                #pragma omp task in_reduction(+:task_result)
                {
                    for (int i = 0; i < N/4; i++) {
                        task_result += global_array[i];
                    }
                }
                
                #pragma omp task in_reduction(+:task_result)
                {
                    for (int i = N/4; i < N/2; i++) {
                        task_result += global_array[i];
                    }
                }
            }
        }
    }
    
    #pragma omp atomic
    global_sum += task_result;
}

/* Function combining multiple clauses */
static void test_combined_clauses(void)
{
    int combined_sum = 0;
    
    /* Combined parallel and for clauses in one pragma */
    #pragma omp target parallel for private(combined_sum) \
                reduction(+:combined_sum) map(tofrom: combined_sum)
    for (int i = 0; i < N; i++) {
        combined_sum += global_array[i];
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
    
    /* Test each clause individually */
    test_target_parallel_for();      /* Triggers OMP_CLAUSE_FOR */
    test_target_parallel();          /* Triggers OMP_CLAUSE_PARALLEL */
    test_target_sections();          /* Triggers OMP_CLAUSE_SECTIONS */
    test_taskgroup();                /* Triggers OMP_CLAUSE_TASKGROUP */
    
    /* Test combined clauses */
    test_combined_clauses();         /* Triggers both PARALLEL and FOR */
    
    /* Calculate expected sum for verification */
    int expected_sum = 0;
    for (int i = 0; i < N; i++) {
        expected_sum += global_array[i];
    }
    expected_sum *= 5;  /* We run 5 different computations */
    
    printf("Computed sum: %d\n", global_sum);
    printf("Expected sum: %d\n", expected_sum);
    printf("Verification: %s\n", 
           global_sum == expected_sum ? "PASS" : "FAIL");
    
    return global_sum == expected_sum ? 0 : 1;
}
