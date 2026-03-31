/* test-omp-pretty-print.c
 * This program is designed to trigger the pretty-printing logic for
 * specific OpenMP clauses in tree-pretty-print.cc (lines 1434-1445).
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test.c
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function 1: Tests 'for' and 'parallel' clauses in combination */
static void test_for_and_parallel(void) {
    int i;
    int local_sum = 0;
    
    /* Combined parallel and for clause in target region */
    #pragma omp target teams distribute parallel for \
                map(tofrom: local_sum) private(i) reduction(+:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    /* Separate parallel clause in target region */
    #pragma omp target parallel private(i) reduction(+:local_sum)
    {
        #pragma omp for
        for (i = 0; i < N/2; i++) {
            local_sum += global_array[i] * 2;
        }
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function 2: Tests 'sections' clause inside target teams */
static void test_sections_in_target(void) {
    int section_a = 0, section_b = 0;
    int i;
    
    /* Sections clause inside target teams construct */
    #pragma omp target teams
    #pragma omp sections private(i) reduction(+:section_a, section_b)
    {
        #pragma omp section
        for (i = 0; i < N/2; i++) {
            section_a += global_array[i];
        }
        
        #pragma omp section
        for (i = N/2; i < N; i++) {
            section_b += global_array[i];
        }
    }
    
    #pragma omp atomic
    global_sum += section_a + section_b;
}

/* Function 3: Tests 'taskgroup' clause inside target parallel region */
static void test_taskgroup_in_target(void) {
    int task_sum = 0;
    int i;
    
    /* Taskgroup clause inside target parallel region */
    #pragma omp target parallel private(i)
    {
        #pragma omp taskgroup task_reduction(+:task_sum)
        {
            #pragma omp task in_reduction(+:task_sum) private(i)
            for (i = 0; i < N/4; i++) {
                task_sum += global_array[i];
            }
            
            #pragma omp task in_reduction(+:task_sum) private(i)
            for (i = N/4; i < N/2; i++) {
                task_sum += global_array[i];
            }
        }
        
        /* Additional parallel for to ensure parallel clause is visited */
        #pragma omp for reduction(+:task_sum) private(i)
        for (i = N/2; i < N; i++) {
            task_sum += global_array[i];
        }
    }
    
    #pragma omp atomic
    global_sum += task_sum;
}

/* Function 4: Tests all clauses in nested contexts */
static void test_nested_constructs(void) {
    int nested_sum = 0;
    int i;
    
    /* Nested: target -> parallel -> for */
    #pragma omp target parallel
    {
        #pragma omp for private(i) reduction(+:nested_sum)
        for (i = 0; i < N; i++) {
            nested_sum += global_array[i];
        }
        
        /* Nested taskgroup inside parallel region */
        #pragma omp taskgroup task_reduction(+:nested_sum)
        {
            #pragma omp task in_reduction(+:nested_sum)
            {
                nested_sum += 1;
            }
        }
    }
    
    /* Another sections construct */
    #pragma omp target teams
    #pragma omp sections reduction(+:nested_sum)
    {
        #pragma omp section
        {
            nested_sum += 10;
        }
        #pragma omp section
        {
            nested_sum += 20;
        }
    }
    
    #pragma omp atomic
    global_sum += nested_sum;
}

int main(void) {
    int i;
    
    /* Initialize global array */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    /* Reset global sum */
    global_sum = 0;
    
    /* Execute all test functions to generate various OpenMP clauses */
    test_for_and_parallel();      /* Generates for, parallel clauses */
    test_sections_in_target();    /* Generates sections clause */
    test_taskgroup_in_target();   /* Generates taskgroup, parallel clauses */
    test_nested_constructs();     /* Generates all clauses in nested contexts */
    
    /* Final verification output */
    printf("Final sum: %d\n", global_sum);
    
    /* Expected value check */
    int expected = 0;
    for (i = 0; i < N; i++) {
        expected += global_array[i];
    }
    /* Account for all the additional operations in test functions */
    expected = expected * 4 + 31;  /* 4 calls * sum + additions in nested_constructs */
    
    if (global_sum == expected) {
        printf("Result verification PASSED\n");
    } else {
        printf("Result verification FAILED: expected %d, got %d\n", expected, global_sum);
    }
    
    return 0;
}
