/* test_openmp_clauses.c
 * This program is designed to trigger the pretty-printing logic for
 * specific OpenMP clauses in tree-pretty-print.cc (lines 1434-1445).
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test_openmp_clauses.c -o test
 * Or with: gcc -O2 -fopenmp -fdump-tree-all test_openmp_clauses.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000

/* File-scope variables for testing data environment */
static int global_array[N];
static int global_sum = 0;

/* Function 1: Tests 'for' and 'parallel' clauses in combination */
static void test_for_and_parallel(void) {
    int i;
    int local_sum = 0;
    
    /* Combined parallel and for clauses in target region */
    #pragma omp target teams distribute parallel for \
                map(tofrom: local_sum) private(i) reduction(+:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    /* Separate parallel clause */
    #pragma omp target parallel private(i) shared(global_array) \
                reduction(+:global_sum)
    {
        #pragma omp for
        for (i = 0; i < N; i++) {
            global_sum += global_array[i];
        }
    }
    
    printf("Local sum from combined parallel+for: %d\n", local_sum);
}

/* Function 2: Tests 'sections' clause */
static void test_sections(void) {
    int section_a_result = 0, section_b_result = 0;
    int i;
    
    /* Sections clause inside target teams construct */
    #pragma omp target teams
    #pragma omp sections private(i) reduction(+:section_a_result, section_b_result)
    {
        #pragma omp section
        {
            for (i = 0; i < N/2; i++) {
                section_a_result += global_array[i];
            }
        }
        
        #pragma omp section
        {
            for (i = N/2; i < N; i++) {
                section_b_result += global_array[i];
            }
        }
    }
    
    printf("Section results: A=%d, B=%d\n", section_a_result, section_b_result);
}

/* Function 3: Tests 'taskgroup' clause nested inside parallel region */
static void test_taskgroup(void) {
    int task_sum = 0;
    
    /* Taskgroup inside target parallel region */
    #pragma omp target parallel
    {
        #pragma omp taskgroup
        {
            #pragma omp task shared(task_sum)
            {
                int i;
                int temp = 0;
                for (i = 0; i < N; i++) {
                    temp += global_array[i];
                }
                #pragma omp atomic
                task_sum += temp;
            }
            
            /* Additional tasks can be added here */
            #pragma omp task shared(task_sum)
            {
                #pragma omp atomic
                task_sum += 1;  /* trivial task */
            }
        }
    }
    
    printf("Taskgroup result: %d\n", task_sum);
}

/* Function 4: Tests all clauses in nested contexts */
static void test_nested_constructs(void) {
    int nested_sum = 0;
    
    /* Nested: parallel region containing for loop */
    #pragma omp target parallel
    {
        int i;
        
        /* This creates OMP_CLAUSE_FOR node */
        #pragma omp for reduction(+:nested_sum) private(i)
        for (i = 0; i < N; i++) {
            nested_sum += global_array[i];
        }
        
        /* Nested sections inside parallel region */
        #pragma omp sections
        {
            #pragma omp section
            {
                nested_sum += 1;
            }
            #pragma omp section
            {
                nested_sum += 2;
            }
        }
        
        /* Taskgroup inside the same parallel region */
        #pragma omp taskgroup
        {
            #pragma omp task shared(nested_sum)
            {
                #pragma omp atomic
                nested_sum += 10;
            }
        }
    }
    
    printf("Nested constructs sum: %d\n", nested_sum);
}

int main(void) {
    int i;
    
    /* Initialize array with predictable values */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    printf("Testing OpenMP clauses for pretty-printer coverage...\n");
    
    /* Test 1: for and parallel clauses */
    test_for_and_parallel();
    
    /* Test 2: sections clause */
    test_sections();
    
    /* Test 3: taskgroup clause */
    test_taskgroup();
    
    /* Test 4: nested constructs with multiple clauses */
    test_nested_constructs();
    
    /* Final verification */
    printf("Global sum: %d\n", global_sum);
    printf("All tests completed.\n");
    
    return 0;
}
