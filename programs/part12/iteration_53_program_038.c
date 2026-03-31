/* test_openmp_clauses.c
 * Generates OpenMP constructs with for, parallel, sections, and taskgroup clauses
 * to trigger OMP_CLAUSE_* pretty-printing in tree-pretty-print.cc
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function to test target parallel for clause */
static void test_target_parallel_for(void) {
    int i;
    int local_sum = 0;
    
    /* This generates OMP_CLAUSE_FOR and OMP_CLAUSE_PARALLEL */
    #pragma omp target teams distribute parallel for \
                map(tofrom: local_sum) private(i) reduction(+:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function to test target parallel clause */
static void test_target_parallel(void) {
    int i;
    int temp_sum = 0;
    
    /* This generates OMP_CLAUSE_PARALLEL */
    #pragma omp target parallel private(i) reduction(+:temp_sum) \
                map(tofrom: temp_sum)
    {
        #pragma omp for
        for (i = 0; i < N/2; i++) {
            temp_sum += global_array[i];
        }
    }
    
    #pragma omp atomic
    global_sum += temp_sum;
}

/* Function to test target sections clause */
static void test_target_sections(void) {
    int sum1 = 0, sum2 = 0;
    int i;
    
    /* This generates OMP_CLAUSE_SECTIONS */
    #pragma omp target teams map(tofrom: sum1, sum2)
    {
        #pragma omp sections private(i)
        {
            #pragma omp section
            for (i = 0; i < N/2; i++) {
                sum1 += global_array[i];
            }
            
            #pragma omp section
            for (i = N/2; i < N; i++) {
                sum2 += global_array[i];
            }
        }
    }
    
    #pragma omp atomic
    global_sum += sum1 + sum2;
}

/* Function to test taskgroup clause */
static void test_taskgroup(void) {
    int task_result = 0;
    
    /* This generates OMP_CLAUSE_TASKGROUP */
    #pragma omp target parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task shared(task_result)
                {
                    int local_val = 0;
                    #pragma omp parallel for reduction(+:local_val)
                    for (int i = 0; i < 100; i++) {
                        local_val += i;
                    }
                    #pragma omp atomic
                    task_result += local_val;
                }
                
                #pragma omp task shared(task_result)
                {
                    int local_val = 0;
                    #pragma omp parallel for reduction(+:local_val)
                    for (int i = 100; i < 200; i++) {
                        local_val += i;
                    }
                    #pragma omp atomic
                    task_result += local_val;
                }
            }
        }
    }
    
    #pragma omp atomic
    global_sum += task_result;
}

/* Function combining multiple clauses */
static void test_combined_clauses(void) {
    int combined_sum = 0;
    int i;
    
    /* Combined parallel and for clauses */
    #pragma omp target parallel for private(i) reduction(+:combined_sum) \
                map(tofrom: combined_sum)
    for (i = 0; i < N; i++) {
        combined_sum += global_array[i] * 2;
    }
    
    #pragma omp atomic
    global_sum += combined_sum;
}

int main(void) {
    int i;
    
    /* Initialize array */
    #pragma omp parallel for private(i)
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    printf("Testing OpenMP clause pretty-printing coverage...\n");
    
    /* Test each clause individually */
    test_target_parallel_for();      /* Generates for and parallel clauses */
    printf("After target_parallel_for: global_sum = %d\n", global_sum);
    
    test_target_parallel();          /* Generates parallel clause */
    printf("After target_parallel: global_sum = %d\n", global_sum);
    
    test_target_sections();          /* Generates sections clause */
    printf("After target_sections: global_sum = %d\n", global_sum);
    
    test_taskgroup();                /* Generates taskgroup clause */
    printf("After taskgroup: global_sum = %d\n", global_sum);
    
    test_combined_clauses();         /* Generates combined parallel for */
    printf("After combined_clauses: global_sum = %d\n", global_sum);
    
    /* Additional nested test with sections inside teams */
    {
        int nested_sum = 0;
        #pragma omp target teams map(tofrom: nested_sum)
        {
            #pragma omp sections
            {
                #pragma omp section
                {
                    #pragma omp parallel for reduction(+:nested_sum)
                    for (int j = 0; j < N/4; j++) {
                        nested_sum += global_array[j];
                    }
                }
                #pragma omp section
                {
                    #pragma omp parallel for reduction(+:nested_sum)
                    for (int j = N/4; j < N/2; j++) {
                        nested_sum += global_array[j];
                    }
                }
            }
        }
        #pragma omp atomic
        global_sum += nested_sum;
    }
    
    printf("Final result: %d\n", global_sum);
    printf("Expected result with N=%d: %d\n", N, 
           (N/100)*4950*2 + 4950 + 14950);  /* Sum of arithmetic series */
    
    return 0;
}
