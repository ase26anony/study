/* test-omp-clauses.c
 * This program is designed to trigger the OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP pretty-printing logic
 * in tree-pretty-print.cc (lines 1434-1445).
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test.c
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function to test target parallel for with 'for' clause */
static void test_target_parallel_for(void)
{
    int i;
    int local_sum = 0;
    
    /* This generates OMP_CLAUSE_FOR */
    #pragma omp target teams distribute parallel for map(tofrom: local_sum) private(i) reduction(+:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function to test target parallel with 'parallel' clause */
static void test_target_parallel(void)
{
    int i;
    int local_arr[N];
    
    /* This generates OMP_CLAUSE_PARALLEL */
    #pragma omp target parallel private(i)
    {
        #pragma omp for
        for (i = 0; i < N; i++) {
            local_arr[i] = i;
        }
    }
    
    /* Use the array to prevent dead code elimination */
    int check = 0;
    for (i = 0; i < N; i++) {
        check += local_arr[i];
    }
    if (check != (N-1)*N/2) {
        printf("Error in target parallel\n");
    }
}

/* Function to test target sections with 'sections' clause */
static void test_target_sections(void)
{
    int section_a = 0, section_b = 0;
    
    /* This generates OMP_CLAUSE_SECTIONS inside target teams */
    #pragma omp target teams
    {
        #pragma omp sections private(section_a, section_b)
        {
            #pragma omp section
            {
                section_a = 1;
                #pragma omp atomic
                global_sum += section_a;
            }
            #pragma omp section
            {
                section_b = 2;
                #pragma omp atomic
                global_sum += section_b;
            }
        }
    }
}

/* Function to test taskgroup with 'taskgroup' clause */
static void test_target_taskgroup(void)
{
    int task_result = 0;
    
    /* This generates OMP_CLAUSE_TASKGROUP inside target parallel */
    #pragma omp target parallel
    {
        #pragma omp taskgroup
        {
            #pragma omp task shared(task_result)
            {
                task_result = 42;
            }
            #pragma omp taskwait
        }
        
        #pragma omp atomic
        global_sum += task_result;
    }
}

/* Function combining parallel and for clauses in single pragma */
static void test_combined_parallel_for(void)
{
    int i;
    int combined_sum = 0;
    
    /* This generates both OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR */
    #pragma omp target parallel for private(i) reduction(+:combined_sum)
    for (i = 0; i < N; i++) {
        combined_sum += global_array[i] * 2;
    }
    
    #pragma omp atomic
    global_sum += combined_sum;
}

/* Main function orchestrating all tests */
int main(void)
{
    int i;
    
    /* Initialize global array */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Test 1: target teams distribute parallel for (for clause) */
    test_target_parallel_for();
    
    /* Test 2: target parallel (parallel clause) */
    test_target_parallel();
    
    /* Test 3: target sections (sections clause) */
    test_target_sections();
    
    /* Test 4: target parallel with taskgroup (taskgroup clause) */
    test_target_taskgroup();
    
    /* Test 5: Combined parallel for (both parallel and for clauses) */
    test_combined_parallel_for();
    
    /* Additional nested test: sections inside target teams */
    {
        int x = 0, y = 0;
        #pragma omp target teams
        {
            #pragma omp sections private(x, y)
            {
                #pragma omp section
                { x = 10; }
                #pragma omp section  
                { y = 20; }
            }
        }
        global_sum += x + y;
    }
    
    /* Additional nested test: taskgroup inside target parallel */
    {
        int z = 0;
        #pragma omp target parallel
        {
            #pragma omp taskgroup
            {
                #pragma omp task shared(z)
                { z = 30; }
            }
        }
        global_sum += z;
    }
    
    printf("Final sum: %d\n", global_sum);
    printf("Expected sum: %d\n", 3 * (N/2 * 99) + 1 + 2 + 42 + 2 * (N/2 * 99) + 30 + 30);
    
    return 0;
}
