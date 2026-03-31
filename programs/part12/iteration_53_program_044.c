/* test_openmp_clauses.c
 * Generates OpenMP constructs with target clauses to trigger
 * OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS,
 * and OMP_CLAUSE_TASKGROUP pretty-printing logic.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function to test target parallel for with for clause */
static void test_for_clause(void) {
    int i;
    int local_sum = 0;
    
    /* OMP_CLAUSE_FOR: target teams distribute parallel for */
    #pragma omp target teams distribute parallel for \
        map(tofrom: local_sum) private(i) reduction(+:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function to test target parallel with parallel clause */
static void test_parallel_clause(void) {
    int i;
    int local_sum = 0;
    
    /* OMP_CLAUSE_PARALLEL: target parallel */
    #pragma omp target parallel private(i) reduction(+:local_sum) \
        map(tofrom: local_sum)
    {
        #pragma omp for
        for (i = 0; i < N/2; i++) {
            local_sum += global_array[i];
        }
    }
    
    /* Combined parallel and for clauses in single pragma */
    #pragma omp target parallel for private(i) reduction(+:local_sum) \
        map(tofrom: local_sum)
    for (i = N/2; i < N; i++) {
        local_sum += global_array[i];
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function to test target sections with sections clause */
static void test_sections_clause(void) {
    int sum1 = 0, sum2 = 0;
    
    /* OMP_CLAUSE_SECTIONS: target sections inside target teams */
    #pragma omp target teams
    {
        #pragma omp target sections reduction(+:sum1, sum2)
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

/* Function to test taskgroup clause inside target region */
static void test_taskgroup_clause(void) {
    int task_result = 0;
    
    /* OMP_CLAUSE_TASKGROUP: taskgroup inside target parallel */
    #pragma omp target parallel map(tofrom: task_result)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task shared(task_result)
                {
                    int temp = 0;
                    for (int i = 0; i < 100; i++) {
                        temp += global_array[i % N];
                    }
                    #pragma omp atomic
                    task_result += temp;
                }
                
                #pragma omp task shared(task_result)
                {
                    int temp = 0;
                    for (int i = 0; i < 100; i++) {
                        temp += global_array[(i + 50) % N];
                    }
                    #pragma omp atomic
                    task_result += temp;
                }
            }
        }
    }
    
    #pragma omp atomic
    global_sum += task_result;
}

/* Main function orchestrating all tests */
int main(void) {
    int i;
    
    /* Initialize array with predictable values */
    #pragma omp parallel for private(i)
    for (i = 0; i < N; i++) {
        global_array[i] = (i % 10) + 1;
    }
    
    /* Reset global sum */
    global_sum = 0;
    
    /* Test all clause types in sequence */
    test_for_clause();        /* Triggers OMP_CLAUSE_FOR */
    test_parallel_clause();   /* Triggers OMP_CLAUSE_PARALLEL */
    test_sections_clause();   /* Triggers OMP_CLAUSE_SECTIONS */
    test_taskgroup_clause();  /* Triggers OMP_CLAUSE_TASKGROUP */
    
    /* Additional nested combination in main */
    int final_check = 0;
    
    /* Nested: sections inside teams with parallel for */
    #pragma omp target teams
    {
        #pragma omp target sections private(i) reduction(+:final_check)
        {
            #pragma omp section
            {
                #pragma omp parallel for private(i) reduction(+:final_check)
                for (i = 0; i < N/4; i++) {
                    final_check += global_array[i];
                }
            }
            
            #pragma omp section
            {
                #pragma omp parallel for private(i) reduction(+:final_check)
                for (i = N/4; i < N/2; i++) {
                    final_check += global_array[i];
                }
            }
        }
    }
    
    /* Verify results */
    int expected = 0;
    for (i = 0; i < N; i++) {
        expected += global_array[i];
    }
    expected = expected * 4 + final_check;  /* 4 tests + final_check */
    
    printf("Global sum: %d\n", global_sum);
    printf("Final check: %d\n", final_check);
    printf("Expected total: %d\n", expected);
    printf("Test %s\n", (global_sum + final_check == expected) ? "PASSED" : "FAILED");
    
    return 0;
}
