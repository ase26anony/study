/* test_omp_clauses.c - Generate OpenMP constructs for pretty-printer coverage */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
#define M 100

/* File-scope variables for testing */
static int global_array[N];
static int global_sum = 0;

/* Function 1: Tests 'for' and 'parallel' clauses in combination */
static void test_for_and_parallel(void) {
    int i;
    int local_sum = 0;
    
    /* Combined parallel and for clause in target region */
    #pragma omp target teams distribute parallel for \
        map(tofrom: local_sum) map(to: global_array) \
        reduction(+:local_sum) private(i)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    /* Separate parallel clause in target region */
    #pragma omp target parallel map(tofrom: local_sum) \
        private(i) num_threads(4)
    {
        int thread_sum = 0;
        #pragma omp for
        for (i = 0; i < N/2; i++) {
            thread_sum += global_array[i];
        }
        #pragma omp atomic
        local_sum += thread_sum;
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function 2: Tests 'sections' clause */
static void test_sections(void) {
    int section_a = 0, section_b = 0;
    int i;
    
    /* Sections clause inside target teams construct */
    #pragma omp target teams map(tofrom: section_a, section_b) \
        map(to: global_array) num_teams(2)
    #pragma omp sections
    {
        #pragma omp section
        {
            for (i = 0; i < N/2; i++) {
                section_a += global_array[i];
            }
        }
        
        #pragma omp section
        {
            for (i = N/2; i < N; i++) {
                section_b += global_array[i];
            }
        }
    }
    
    #pragma omp atomic
    global_sum += section_a + section_b;
}

/* Function 3: Tests 'taskgroup' clause */
static void test_taskgroup(void) {
    int task_sum = 0;
    int i;
    
    /* Taskgroup inside target parallel region */
    #pragma omp target parallel map(tofrom: task_sum) \
        map(to: global_array) private(i)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task private(i) shared(task_sum)
                {
                    int local_task_sum = 0;
                    for (i = 0; i < N/4; i++) {
                        local_task_sum += global_array[i];
                    }
                    #pragma omp atomic
                    task_sum += local_task_sum;
                }
                
                #pragma omp task private(i) shared(task_sum)
                {
                    int local_task_sum = 0;
                    for (i = N/4; i < N/2; i++) {
                        local_task_sum += global_array[i];
                    }
                    #pragma omp atomic
                    task_sum += local_task_sum;
                }
            } /* end taskgroup */
        } /* end single */
    } /* end target parallel */
    
    #pragma omp atomic
    global_sum += task_sum;
}

/* Function 4: Tests all clauses in nested contexts */
static void test_nested_clauses(void) {
    int nested_sum = 0;
    int i, j;
    
    /* Nested: parallel with for inside sections */
    #pragma omp target teams map(tofrom: nested_sum) \
        map(to: global_array)
    {
        #pragma omp parallel private(i, j)
        {
            #pragma omp sections
            {
                #pragma omp section
                {
                    #pragma omp for reduction(+:nested_sum)
                    for (i = 0; i < N/2; i++) {
                        nested_sum += global_array[i];
                    }
                }
                
                #pragma omp section
                {
                    #pragma omp for reduction(+:nested_sum)
                    for (i = N/2; i < N; i++) {
                        nested_sum += global_array[i];
                    }
                }
            }
        }
    }
    
    #pragma omp atomic
    global_sum += nested_sum;
}

int main(void) {
    int i;
    
    /* Initialize array with predictable values */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Test 1: for and parallel clauses */
    printf("Testing 'for' and 'parallel' clauses...\n");
    test_for_and_parallel();
    
    /* Test 2: sections clause */
    printf("Testing 'sections' clause...\n");
    test_sections();
    
    /* Test 3: taskgroup clause */
    printf("Testing 'taskgroup' clause...\n");
    test_taskgroup();
    
    /* Test 4: nested clauses */
    printf("Testing nested clauses...\n");
    test_nested_clauses();
    
    /* Verify computation */
    int expected_sum = 0;
    for (i = 0; i < N; i++) {
        expected_sum += global_array[i];
    }
    expected_sum *= 4; /* Each test adds to global_sum */
    
    printf("Final global_sum = %d\n", global_sum);
    printf("Expected sum = %d\n", expected_sum);
    printf("Test %s\n", (global_sum == expected_sum) ? "PASSED" : "FAILED");
    
    return 0;
}
