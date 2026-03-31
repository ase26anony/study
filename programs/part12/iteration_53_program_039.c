/* test_openmp_clauses.c
 * Generates OpenMP constructs with specific clauses to trigger
 * OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, 
 * and OMP_CLAUSE_TASKGROUP pretty-printing logic.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function 1: Tests target parallel for with for clause */
static void test_target_parallel_for(void)
{
    int i;
    int local_sum = 0;
    
    /* This generates OMP_CLAUSE_FOR */
    #pragma omp target teams distribute parallel for \
                map(tofrom: local_sum) private(i) reduction(+:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    #pragma omp atomic
    global_sum += local_sum;
    
    printf("Target parallel for sum: %d\n", local_sum);
}

/* Function 2: Tests target parallel with parallel clause */
static void test_target_parallel(void)
{
    int i;
    int partial_sum = 0;
    
    /* This generates OMP_CLAUSE_PARALLEL */
    #pragma omp target parallel private(i) reduction(+:partial_sum) \
                map(tofrom: partial_sum)
    {
        #pragma omp for
        for (i = 0; i < N/2; i++) {
            partial_sum += global_array[i];
        }
    }
    
    #pragma omp atomic
    global_sum += partial_sum;
    
    printf("Target parallel sum: %d\n", partial_sum);
}

/* Function 3: Tests target sections with sections clause */
static void test_target_sections(void)
{
    int sum1 = 0, sum2 = 0;
    int i;
    
    /* This generates OMP_CLAUSE_SECTIONS */
    #pragma omp target teams map(tofrom: sum1, sum2)
    #pragma omp sections private(i)
    {
        #pragma omp section
        for (i = 0; i < N/2; i++) {
            sum1 += global_array[i];
        }
        
        #pragma omp section
        for (i = N/2; i < N; i++) {
            sum2 += global_array[i];
        }
    }
    
    #pragma omp atomic
    global_sum += sum1 + sum2;
    
    printf("Target sections sums: %d + %d = %d\n", sum1, sum2, sum1 + sum2);
}

/* Function 4: Tests taskgroup clause inside target region */
static void test_target_with_taskgroup(void)
{
    int task_result = 0;
    
    /* Combined parallel and for clause in single pragma */
    #pragma omp target parallel for map(tofrom: task_result) \
                private(global_sum)
    for (int i = 0; i < 10; i++) {
        /* This generates OMP_CLAUSE_TASKGROUP */
        #pragma omp taskgroup
        {
            #pragma omp task shared(task_result)
            {
                #pragma omp atomic
                task_result += i;
            }
        }
    }
    
    #pragma omp atomic
    global_sum += task_result;
    
    printf("Taskgroup result: %d\n", task_result);
}

/* Function 5: Tests nested clauses combination */
static void test_combined_clauses(void)
{
    int combined_sum = 0;
    
    /* Combined parallel and for clauses */
    #pragma omp target parallel for private(global_sum) \
                map(tofrom: combined_sum) reduction(+:combined_sum)
    for (int i = 0; i < N; i++) {
        combined_sum += global_array[i] % 10;
    }
    
    #pragma omp atomic
    global_sum += combined_sum;
    
    printf("Combined clauses sum: %d\n", combined_sum);
}

/* Main function orchestrates all tests */
int main(void)
{
    int i;
    
    /* Initialize global array */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Test 1: Target parallel for (for clause) */
    test_target_parallel_for();
    
    /* Test 2: Target parallel (parallel clause) */
    test_target_parallel();
    
    /* Test 3: Target sections (sections clause) */
    test_target_sections();
    
    /* Test 4: Target with taskgroup (taskgroup clause) */
    test_target_with_taskgroup();
    
    /* Test 5: Combined clauses */
    test_combined_clauses();
    
    printf("Final global sum: %d\n", global_sum);
    printf("Expected sum: %d\n", (N*(N-1)/2) % 100 * (N/100) + 45);
    
    return 0;
}
