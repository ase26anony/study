/* test_omp_clauses.c - Generate OpenMP clauses for pretty-printer coverage */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function to test 'for' and 'parallel' clauses */
static void test_for_parallel_clauses(void) {
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

/* Function to test 'sections' clause */
static void test_sections_clause(void) {
    int section_a = 0, section_b = 0;
    
    /* Sections clause inside target teams construct */
    #pragma omp target teams
    #pragma omp sections private(section_a, section_b)
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
static void test_taskgroup_clause(void) {
    int task_result = 0;
    
    /* Taskgroup inside target parallel region */
    #pragma omp target parallel
    {
        #pragma omp taskgroup
        {
            #pragma omp task shared(task_result)
            {
                int temp = 0;
                for (int i = 0; i < N; i += 4) {
                    temp += global_array[i];
                }
                #pragma omp atomic
                task_result += temp;
            }
            
            #pragma omp task shared(task_result)
            {
                int temp = 0;
                for (int i = 1; i < N; i += 4) {
                    temp += global_array[i];
                }
                #pragma omp atomic
                task_result += temp;
            }
        }
    }
    
    #pragma omp atomic
    global_sum += task_result;
}

/* Function with nested clauses */
static void test_nested_clauses(void) {
    int i, nested_sum = 0;
    
    /* Target with parallel for - both clauses present */
    #pragma omp target parallel for private(i) reduction(+:nested_sum) \
                num_threads(4)
    for (i = 0; i < N; i++) {
        nested_sum += global_array[i] * 3;
    }
    
    /* Target teams with distribute parallel for */
    #pragma omp target teams distribute parallel for \
                private(i) reduction(+:nested_sum) collapse(1)
    for (i = 0; i < N; i++) {
        nested_sum -= global_array[i];
    }
    
    #pragma omp atomic
    global_sum += nested_sum;
}

int main(void) {
    /* Initialize array with predictable values */
    for (int i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    /* Reset global sum */
    global_sum = 0;
    
    /* Test all clause types */
    test_for_parallel_clauses();      /* Tests 'for' and 'parallel' clauses */
    test_sections_clause();           /* Tests 'sections' clause */
    test_taskgroup_clause();          /* Tests 'taskgroup' clause */
    test_nested_clauses();            /* Tests nested/combined clauses */
    
    /* Additional direct tests in main */
    int main_sum = 0;
    
    /* Direct test: target parallel for */
    #pragma omp target parallel for reduction(+:main_sum) private(int k)
    for (int k = 0; k < N; k++) {
        main_sum += global_array[k];
    }
    
    /* Direct test: target sections */
    int sec1 = 0, sec2 = 0;
    #pragma omp target sections private(sec1, sec2)
    {
        #pragma omp section
        { sec1 = 1; }
        #pragma omp section  
        { sec2 = 2; }
    }
    main_sum += sec1 + sec2;
    
    printf("Final sum: %d\n", global_sum + main_sum);
    
    /* Verify computation */
    int verify_sum = 0;
    for (int i = 0; i < N; i++) {
        verify_sum += global_array[i];
    }
    printf("Verification sum: %d\n", verify_sum);
    
    return 0;
}
