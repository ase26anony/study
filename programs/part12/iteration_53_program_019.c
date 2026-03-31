/* test_openmp_coverage.c
 * Generates OpenMP constructs with specific clauses to cover
 * OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP
 * in tree-pretty-print.cc lines 1434-1445
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function 1: Tests target teams distribute parallel for (for clause) */
static void test_for_clause(void) {
    int local_sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    /* OMP_CLAUSE_FOR will be generated here */
    #pragma omp target teams distribute parallel for map(tofrom: local_sum) reduction(+:local_sum)
    for (int i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    global_sum += local_sum;
    printf("For clause test sum: %d\n", local_sum);
}

/* Function 2: Tests target parallel (parallel clause) */
static void test_parallel_clause(void) {
    int parallel_sum = 0;
    int i;
    
    /* OMP_CLAUSE_PARALLEL will be generated here */
    #pragma omp target parallel reduction(+:parallel_sum) private(i)
    {
        #pragma omp for
        for (i = 0; i < N; i++) {
            parallel_sum += global_array[i] * 2;
        }
    }
    
    global_sum += parallel_sum;
    printf("Parallel clause test sum: %d\n", parallel_sum);
}

/* Function 3: Tests target parallel for (both parallel and for clauses) */
static void test_parallel_for_combined(void) {
    int combined_sum = 0;
    
    /* Both OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR will be generated here */
    #pragma omp target parallel for reduction(+:combined_sum)
    for (int i = 0; i < N; i++) {
        combined_sum += global_array[i] * 3;
    }
    
    global_sum += combined_sum;
    printf("Parallel+For combined test sum: %d\n", combined_sum);
}

/* Function 4: Tests target sections (sections clause) */
static void test_sections_clause(void) {
    int section_sum1 = 0, section_sum2 = 0;
    
    /* OMP_CLAUSE_SECTIONS will be generated here */
    #pragma omp target teams
    #pragma omp sections reduction(+:section_sum1, section_sum2)
    {
        #pragma omp section
        {
            for (int i = 0; i < N/2; i++) {
                section_sum1 += global_array[i];
            }
        }
        
        #pragma omp section
        {
            for (int i = N/2; i < N; i++) {
                section_sum2 += global_array[i];
            }
        }
    }
    
    global_sum += section_sum1 + section_sum2;
    printf("Sections clause test sum: %d + %d = %d\n", 
           section_sum1, section_sum2, section_sum1 + section_sum2);
}

/* Function 5: Tests taskgroup clause inside target region */
static void test_taskgroup_clause(void) {
    int task_sum = 0;
    
    /* OMP_CLAUSE_TASKGROUP will be generated here */
    #pragma omp target parallel
    {
        #pragma omp taskgroup
        {
            #pragma omp task reduction(+:task_sum)
            {
                for (int i = 0; i < N; i++) {
                    task_sum += global_array[i] / 2;
                }
            }
            
            #pragma omp task reduction(+:task_sum)
            {
                for (int i = 0; i < N; i++) {
                    task_sum += global_array[i] / 3;
                }
            }
            
            #pragma omp taskwait
        }
    }
    
    global_sum += task_sum;
    printf("Taskgroup clause test sum: %d\n", task_sum);
}

/* Function 6: Complex nested construct with multiple clauses */
static void test_nested_constructs(void) {
    int nested_sum = 0;
    
    /* Nested construct with sections inside parallel region */
    #pragma omp target parallel
    {
        int local_private = 0;
        
        #pragma omp sections private(local_private)
        {
            #pragma omp section
            {
                local_private = 0;
                #pragma omp for
                for (int i = 0; i < N/2; i++) {
                    local_private += global_array[i];
                }
                #pragma omp atomic
                nested_sum += local_private;
            }
            
            #pragma omp section
            {
                local_private = 0;
                #pragma omp for
                for (int i = N/2; i < N; i++) {
                    local_private += global_array[i];
                }
                #pragma omp atomic
                nested_sum += local_private;
            }
        }
        
        /* Taskgroup inside the same parallel region */
        #pragma omp taskgroup
        {
            #pragma omp task
            {
                #pragma omp atomic
                nested_sum += 1;
            }
        }
    }
    
    global_sum += nested_sum;
    printf("Nested constructs test sum: %d\n", nested_sum);
}

int main(void) {
    printf("=== OpenMP Clause Coverage Test ===\n");
    
    /* Initialize global array */
    for (int i = 0; i < N; i++) {
        global_array[i] = (i * 7) % 97;  /* Some non-trivial pattern */
    }
    
    /* Test all clause types */
    test_for_clause();           /* OMP_CLAUSE_FOR */
    test_parallel_clause();      /* OMP_CLAUSE_PARALLEL */
    test_parallel_for_combined();/* OMP_CLAUSE_PARALLEL + OMP_CLAUSE_FOR */
    test_sections_clause();      /* OMP_CLAUSE_SECTIONS */
    test_taskgroup_clause();     /* OMP_CLAUSE_TASKGROUP */
    test_nested_constructs();    /* Multiple clauses in nested context */
    
    printf("\nTotal global sum: %d\n", global_sum);
    printf("=== Test Complete ===\n");
    
    return 0;
}
