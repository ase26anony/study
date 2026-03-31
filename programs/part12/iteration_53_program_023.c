/* test_openmp_clauses.c
 * Generates AST nodes for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP to exercise
 * the uncovered pretty-printer code in tree-pretty-print.cc
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
    
    /* This generates OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR */
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
    int local_sum = 0;
    
    /* This generates OMP_CLAUSE_PARALLEL */
    #pragma omp target parallel private(i) reduction(+:local_sum)
    {
        /* Combined parallel for - generates both clauses */
        #pragma omp for
        for (i = 0; i < N/2; i++) {
            local_sum += global_array[i];
        }
        
        /* Nested taskgroup inside parallel region - generates OMP_CLAUSE_TASKGROUP */
        #pragma omp taskgroup
        {
            #pragma omp task
            {
                int temp = 0;
                for (int j = 0; j < 10; j++) {
                    temp += j;
                }
                #pragma omp atomic
                local_sum += temp;
            }
        }
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function to test target sections clause */
static void test_target_sections(void)
{
    int sum1 = 0, sum2 = 0;
    
    /* This generates OMP_CLAUSE_SECTIONS */
    #pragma omp target teams distribute parallel for \
                map(tofrom: sum1, sum2) private(int i)
    for (int i = 0; i < N; i++) {
        if (i < N/2) sum1 += global_array[i];
        else sum2 += global_array[i];
    }
    
    /* Alternative sections construct */
    #pragma omp target teams
    {
        #pragma omp sections private(int i) reduction(+:sum1, sum2)
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

/* Function to test taskgroup clause in various contexts */
static void test_taskgroup(void)
{
    int task_sum = 0;
    
    /* Taskgroup inside target parallel region */
    #pragma omp target parallel
    {
        /* This generates OMP_CLAUSE_TASKGROUP */
        #pragma omp taskgroup
        {
            #pragma omp task private(int i) shared(task_sum)
            {
                int local = 0;
                for (int i = 0; i < 100; i++) {
                    local += i;
                }
                #pragma omp atomic
                task_sum += local;
            }
            
            #pragma omp task private(int i) shared(task_sum)
            {
                int local = 0;
                for (int i = 100; i < 200; i++) {
                    local += i;
                }
                #pragma omp atomic
                task_sum += local;
            }
        }
        
        /* Another taskgroup with wait */
        #pragma omp taskgroup
        {
            #pragma omp task
            {
                int temp = 5;
                #pragma omp atomic
                task_sum += temp;
            }
        }
    }
    
    #pragma omp atomic
    global_sum += task_sum;
}

/* Main function with combined constructs */
int main(void)
{
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    printf("Testing OpenMP clause pretty-printing coverage...\n");
    
    /* Test 1: Target parallel for with combined clauses */
    printf("Test 1: target parallel for\n");
    #pragma omp target parallel for private(int i) reduction(+:global_sum)
    for (int i = 0; i < N; i++) {
        global_sum += global_array[i];
    }
    
    /* Test 2: Nested taskgroup in target region */
    printf("Test 2: nested taskgroup\n");
    test_taskgroup();
    
    /* Test 3: Target sections */
    printf("Test 3: target sections\n");
    test_target_sections();
    
    /* Test 4: Complex combined construct */
    printf("Test 4: complex combined\n");
    #pragma omp target teams distribute parallel for \
                private(int i) reduction(+:global_sum) \
                num_teams(4) thread_limit(64)
    for (int i = 0; i < N; i++) {
        global_array[i] = (global_array[i] + 1) % 100;
        global_sum += global_array[i];
    }
    
    /* Test 5: Multiple clauses in sequence */
    printf("Test 5: sequence of clauses\n");
    test_target_parallel_for();
    test_target_parallel();
    
    /* Final verification */
    int expected_sum = 0;
    for (int i = 0; i < N; i++) {
        expected_sum += global_array[i];
    }
    
    printf("Global sum: %d (expected: %d)\n", global_sum, expected_sum);
    printf("All tests completed.\n");
    
    return 0;
}
