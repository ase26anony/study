/* test-omp-clauses.c
 * 
 * This program is designed to generate OpenMP AST nodes for the clauses:
 *   OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP
 * so that the GCC tree pretty-printer will visit them when using dump flags
 * like -fdump-tree-omplower, -fdump-tree-original, etc.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function to test 'for' and 'parallel' clauses combined */
static void test_for_and_parallel(void)
{
    int i;
    int local_sum = 0;
    
    /* This pragma should generate OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR nodes */
    #pragma omp target teams distribute parallel for \
                reduction(+:local_sum) map(tofrom:local_sum) private(i)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function to test 'sections' clause */
static void test_sections(void)
{
    int section_a = 0, section_b = 0;
    
    /* This pragma should generate OMP_CLAUSE_SECTIONS node */
    #pragma omp target teams distribute parallel sections \
                reduction(+:section_a, section_b)
    {
        #pragma omp section
        {
            for (int i = 0; i < N/2; i++) {
                section_a += global_array[i];
            }
        }
        
        #pragma omp section
        {
            for (int i = N/2; i < N; i++) {
                section_b += global_array[i];
            }
        }
    }
    
    #pragma omp atomic
    global_sum += section_a + section_b;
}

/* Function to test 'taskgroup' clause */
static void test_taskgroup(void)
{
    int task_sum = 0;
    
    /* Outer parallel region */
    #pragma omp target parallel
    {
        /* This pragma should generate OMP_CLAUSE_TASKGROUP node */
        #pragma omp taskgroup task_reduction(+:task_sum)
        {
            #pragma omp task in_reduction(+:task_sum)
            {
                int local = 0;
                for (int i = 0; i < N/4; i++) {
                    local += global_array[i];
                }
                #pragma omp atomic
                task_sum += local;
            }
            
            #pragma omp task in_reduction(+:task_sum)
            {
                int local = 0;
                for (int i = N/4; i < N/2; i++) {
                    local += global_array[i];
                }
                #pragma omp atomic
                task_sum += local;
            }
        }
    }
    
    #pragma omp atomic
    global_sum += task_sum;
}

/* Function combining multiple clauses in nested constructs */
static void test_combined_nested(void)
{
    int combined_sum = 0;
    
    /* Nested: teams with parallel for inside */
    #pragma omp target teams
    {
        int team_sum = 0;
        
        /* This should generate OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR */
        #pragma omp parallel for reduction(+:team_sum) private(int j)
        for (int j = 0; j < N; j++) {
            team_sum += global_array[j];
        }
        
        #pragma omp atomic
        combined_sum += team_sum;
    }
    
    /* Another level: sections inside teams */
    #pragma omp target teams
    {
        int section_sum = 0;
        
        /* This should generate OMP_CLAUSE_SECTIONS */
        #pragma omp parallel sections reduction(+:section_sum)
        {
            #pragma omp section
            {
                for (int k = 0; k < N/2; k++) {
                    section_sum += global_array[k];
                }
            }
            
            #pragma omp section
            {
                for (int k = N/2; k < N; k++) {
                    section_sum += global_array[k];
                }
            }
        }
        
        #pragma omp atomic
        combined_sum += section_sum;
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
    
    /* Test 1: for + parallel clauses */
    test_for_and_parallel();
    printf("After test_for_and_parallel: global_sum = %d\n", global_sum);
    
    /* Test 2: sections clause */
    test_sections();
    printf("After test_sections: global_sum = %d\n", global_sum);
    
    /* Test 3: taskgroup clause */
    test_taskgroup();
    printf("After test_taskgroup: global_sum = %d\n", global_sum);
    
    /* Test 4: combined nested constructs */
    test_combined_nested();
    printf("After test_combined_nested: global_sum = %d\n", global_sum);
    
    /* Verify result (should be 4 * sum of array) */
    int expected = 0;
    for (int i = 0; i < N; i++) {
        expected += global_array[i];
    }
    expected *= 4;
    
    printf("Expected sum: %d, Actual sum: %d\n", expected, global_sum);
    printf("Test %s\n", (global_sum == expected) ? "PASSED" : "FAILED");
    
    return 0;
}
