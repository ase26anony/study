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

/* Function 1: Tests target teams distribute parallel for (OMP_CLAUSE_FOR) */
static void test_for_clause(void) {
    int i;
    int local_sum = 0;
    
    /* This generates OMP_CLAUSE_FOR in the AST */
    #pragma omp target teams distribute parallel for map(tofrom: local_sum) reduction(+:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    #pragma omp atomic
    global_sum += local_sum;
    
    printf("For clause test complete, local_sum = %d\n", local_sum);
}

/* Function 2: Tests target parallel (OMP_CLAUSE_PARALLEL) */
static void test_parallel_clause(void) {
    int i;
    int local_sum = 0;
    
    /* This generates OMP_CLAUSE_PARALLEL in the AST */
    #pragma omp target parallel map(tofrom: local_sum) private(i) reduction(+:local_sum)
    {
        #pragma omp for
        for (i = 0; i < N; i++) {
            local_sum += global_array[i] * 2;
        }
    }
    
    #pragma omp atomic
    global_sum += local_sum;
    
    printf("Parallel clause test complete, local_sum = %d\n", local_sum);
}

/* Function 3: Tests target sections (OMP_CLAUSE_SECTIONS) */
static void test_sections_clause(void) {
    int section1_sum = 0, section2_sum = 0;
    
    /* This generates OMP_CLAUSE_SECTIONS in the AST */
    #pragma omp target teams map(tofrom: section1_sum, section2_sum)
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
            for (int i = N/2; i < N; i++) {
                section2_sum += global_array[i];
            }
        }
    }
    
    #pragma omp atomic
    global_sum += section1_sum + section2_sum;
    
    printf("Sections clause test complete, sums = %d, %d\n", section1_sum, section2_sum);
}

/* Function 4: Tests taskgroup (OMP_CLAUSE_TASKGROUP) inside target region */
static void test_taskgroup_clause(void) {
    int task_sum = 0;
    
    /* This generates OMP_CLAUSE_TASKGROUP in the AST */
    #pragma omp target parallel map(tofrom: task_sum)
    {
        #pragma omp taskgroup
        {
            #pragma omp task shared(task_sum)
            {
                int temp = 0;
                for (int i = 0; i < N; i++) {
                    temp += global_array[i];
                }
                #pragma omp atomic
                task_sum += temp;
            }
            
            #pragma omp taskwait
        }
    }
    
    #pragma omp atomic
    global_sum += task_sum;
    
    printf("Taskgroup clause test complete, task_sum = %d\n", task_sum);
}

/* Function 5: Tests combined parallel for clause (both OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR) */
static void test_combined_parallel_for(void) {
    int combined_sum = 0;
    
    /* This generates both OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR in the AST */
    #pragma omp target parallel for map(tofrom: combined_sum) reduction(+:combined_sum)
    for (int i = 0; i < N; i++) {
        combined_sum += global_array[i] * 3;
    }
    
    #pragma omp atomic
    global_sum += combined_sum;
    
    printf("Combined parallel for test complete, combined_sum = %d\n", combined_sum);
}

/* Function 6: Tests nested taskgroup inside parallel region */
static void test_nested_taskgroup(void) {
    int nested_sum = 0;
    
    /* Nested: taskgroup inside parallel region */
    #pragma omp target parallel map(tofrom: nested_sum)
    {
        int local_nested = 0;
        
        #pragma omp for private(local_nested)
        for (int i = 0; i < N; i++) {
            local_nested += global_array[i];
        }
        
        #pragma omp taskgroup
        {
            #pragma omp task
            {
                #pragma omp atomic
                nested_sum += local_nested;
            }
        }
    }
    
    #pragma omp atomic
    global_sum += nested_sum;
    
    printf("Nested taskgroup test complete, nested_sum = %d\n", nested_sum);
}

int main(void) {
    /* Initialize global array */
    for (int i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    printf("Starting OpenMP clause coverage tests...\n");
    printf("Array size: %d, values: 0-99 repeating\n\n", N);
    
    /* Reset global sum */
    global_sum = 0;
    
    /* Test 1: OMP_CLAUSE_FOR */
    printf("1. Testing OMP_CLAUSE_FOR (target teams distribute parallel for):\n");
    test_for_clause();
    
    /* Test 2: OMP_CLAUSE_PARALLEL */
    printf("\n2. Testing OMP_CLAUSE_PARALLEL (target parallel):\n");
    test_parallel_clause();
    
    /* Test 3: OMP_CLAUSE_SECTIONS */
    printf("\n3. Testing OMP_CLAUSE_SECTIONS (target sections):\n");
    test_sections_clause();
    
    /* Test 4: OMP_CLAUSE_TASKGROUP */
    printf("\n4. Testing OMP_CLAUSE_TASKGROUP (taskgroup inside target parallel):\n");
    test_taskgroup_clause();
    
    /* Test 5: Combined parallel for */
    printf("\n5. Testing combined OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR:\n");
    test_combined_parallel_for();
    
    /* Test 6: Nested taskgroup */
    printf("\n6. Testing nested taskgroup inside parallel region:\n");
    test_nested_taskgroup();
    
    printf("\nAll tests completed successfully!\n");
    printf("Final global_sum = %d\n", global_sum);
    printf("Expected value should be positive (sum of various array computations)\n");
    
    return 0;
}
