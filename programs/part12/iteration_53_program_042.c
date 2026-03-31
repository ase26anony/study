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

/* Function 1: Tests target parallel for with for clause */
static void test_target_parallel_for(void) {
    int i;
    int local_sum = 0;
    
    /* This generates OMP_CLAUSE_FOR and OMP_CLAUSE_PARALLEL */
    #pragma omp target teams distribute parallel for \
                reduction(+:local_sum) map(tofrom:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    #pragma omp atomic
    global_sum += local_sum;
    
    /* Combined parallel and for in single pragma */
    int arr[N];
    #pragma omp target parallel for simd \
                private(i) map(tofrom:arr[0:N])
    for (i = 0; i < N; i++) {
        arr[i] = i * 2;
    }
}

/* Function 2: Tests target parallel with parallel clause */
static void test_target_parallel(void) {
    int i;
    int thread_sum = 0;
    
    /* This generates OMP_CLAUSE_PARALLEL */
    #pragma omp target parallel reduction(+:thread_sum) \
                num_threads(4) map(tofrom:thread_sum)
    {
        #pragma omp for
        for (i = 0; i < N/2; i++) {
            thread_sum += global_array[i];
        }
    }
    
    #pragma omp atomic
    global_sum += thread_sum;
    
    /* Nested taskgroup inside target parallel region */
    #pragma omp target parallel map(tofrom:global_sum)
    {
        int local_var = 0;
        
        /* This generates OMP_CLAUSE_TASKGROUP */
        #pragma omp taskgroup
        {
            #pragma omp task shared(local_var)
            {
                local_var = 1;
            }
            #pragma omp taskwait
        }
        
        #pragma omp atomic
        global_sum += local_var;
    }
}

/* Function 3: Tests target sections with sections clause */
static void test_target_sections(void) {
    int section_a = 0, section_b = 0;
    
    /* This generates OMP_CLAUSE_SECTIONS */
    #pragma omp target teams sections \
                reduction(+:section_a, section_b) \
                map(tofrom:section_a, section_b)
    {
        #pragma omp section
        {
            for (int i = 0; i < N/2; i++) {
                section_a += global_array[i];
            }
        }
        
        #pragma omp section
        {
            for (int j = N/2; j < N; j++) {
                section_b += global_array[j];
            }
        }
    }
    
    #pragma omp atomic
    global_sum += section_a;
    #pragma omp atomic
    global_sum += section_b;
}

/* Function 4: Tests taskgroup clause in various contexts */
static void test_taskgroup(void) {
    int task_result = 0;
    
    /* Taskgroup inside target region */
    #pragma omp target map(tofrom:task_result)
    {
        /* This generates OMP_CLAUSE_TASKGROUP */
        #pragma omp taskgroup
        {
            #pragma omp task shared(task_result)
            {
                task_result = 42;
            }
        }
    }
    
    #pragma omp atomic
    global_sum += task_result;
    
    /* Complex nesting with multiple clauses */
    #pragma omp target teams distribute parallel for \
                map(tofrom:global_array[0:N])
    for (int i = 0; i < N; i++) {
        #pragma omp taskgroup
        {
            #pragma omp task
            {
                global_array[i] += 1;
            }
        }
    }
}

/* Main function orchestrates all tests */
int main(void) {
    /* Initialize global array */
    for (int i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Test 1: Target parallel for with for clause */
    test_target_parallel_for();
    printf("Test 1 (target parallel for) completed. Global sum: %d\n", global_sum);
    
    /* Test 2: Target parallel with parallel clause */
    test_target_parallel();
    printf("Test 2 (target parallel) completed. Global sum: %d\n", global_sum);
    
    /* Test 3: Target sections with sections clause */
    test_target_sections();
    printf("Test 3 (target sections) completed. Global sum: %d\n", global_sum);
    
    /* Test 4: Taskgroup clause in various contexts */
    test_taskgroup();
    printf("Test 4 (taskgroup) completed. Global sum: %d\n", global_sum);
    
    /* Final verification computation */
    int verify_sum = 0;
    #pragma omp target teams distribute parallel for \
                reduction(+:verify_sum) map(to:global_array[0:N])
    for (int i = 0; i < N; i++) {
        verify_sum += global_array[i];
    }
    
    printf("Verification sum: %d\n", verify_sum);
    printf("All tests completed successfully!\n");
    
    return 0;
}
