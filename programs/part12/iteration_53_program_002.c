/* test-omp-clauses.c
 * This program is designed to trigger the pretty-printer logic for
 * specific OpenMP clauses: for, parallel, sections, and taskgroup.
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test-omp-clauses.c
 * The generated AST should contain OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP nodes.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function to test 'for' and 'parallel' clauses in combination */
static void test_for_and_parallel(void) {
    int i;
    int local_sum = 0;
    
    /* This pragma should generate both OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR nodes */
    #pragma omp target teams distribute parallel for simd \
                reduction(+:local_sum) private(i) map(tofrom:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    /* This pragma should generate OMP_CLAUSE_PARALLEL node */
    #pragma omp target parallel private(i) reduction(+:local_sum) \
                map(tofrom:local_sum)
    {
        #pragma omp for
        for (i = 0; i < N; i++) {
            local_sum += global_array[i];
        }
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function to test 'sections' clause */
static void test_sections(void) {
    int section_a = 0, section_b = 0;
    
    /* This pragma should generate OMP_CLAUSE_SECTIONS node */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom:section_a, section_b)
    for (int i = 0; i < N/2; i++) {
        section_a += global_array[i];
    }
    
    /* Alternative: sections inside target teams */
    #pragma omp target teams
    {
        #pragma omp sections private(int i) reduction(+:section_b)
        {
            #pragma omp section
            for (int i = N/2; i < N; i++) {
                section_b += global_array[i];
            }
            #pragma omp section
            for (int i = 0; i < N/2; i++) {
                section_b += global_array[i] * 2;
            }
        }
    }
    
    #pragma omp atomic
    global_sum += section_a + section_b;
}

/* Function to test 'taskgroup' clause */
static void test_taskgroup(void) {
    int task_sum = 0;
    
    /* taskgroup must be inside a parallel region */
    #pragma omp target parallel map(tofrom:task_sum)
    {
        /* This pragma should generate OMP_CLAUSE_TASKGROUP node */
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
            
            #pragma omp task shared(task_sum)
            {
                int temp = 0;
                for (int i = 0; i < N; i++) {
                    temp += global_array[i] * 2;
                }
                #pragma omp atomic
                task_sum += temp;
            }
        }
    }
    
    #pragma omp atomic
    global_sum += task_sum;
}

/* Main function with mixed constructs */
int main(void) {
    int i;
    
    /* Initialize array */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    /* Test 1: for + parallel clauses */
    test_for_and_parallel();
    
    /* Test 2: sections clause */
    test_sections();
    
    /* Test 3: taskgroup clause */
    test_taskgroup();
    
    /* Additional combined construct in main */
    int main_sum = 0;
    
    /* Combined parallel for in target region */
    #pragma omp target parallel for reduction(+:main_sum) private(i) \
                map(tofrom:main_sum)
    for (i = 0; i < N; i++) {
        main_sum += global_array[i];
    }
    
    global_sum += main_sum;
    
    printf("Final sum: %d\n", global_sum);
    printf("Expected sum: %d\n", (N * 99 / 2) * 8); /* Rough approximation */
    
    return 0;
}
