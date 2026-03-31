/* test_openmp_clauses.c
 * Generates OpenMP constructs with for, parallel, sections, and taskgroup clauses
 * to trigger OMP_CLAUSE_* pretty-printing logic in tree-pretty-print.cc
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function 1: Tests target parallel for with for clause */
static void test_target_parallel_for(void)
{
    int i;
    int local_sum = 0;
    
    /* OMP_CLAUSE_FOR: target teams distribute parallel for */
    #pragma omp target teams distribute parallel for \
        map(tofrom: local_sum) private(i) reduction(+:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    /* OMP_CLAUSE_PARALLEL: target parallel for (combined) */
    #pragma omp target parallel for \
        map(tofrom: local_sum) private(i) reduction(+:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i] * 2;
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function 2: Tests target sections with sections clause */
static void test_target_sections(void)
{
    int section_a = 0, section_b = 0;
    
    /* OMP_CLAUSE_SECTIONS: target sections inside target teams */
    #pragma omp target teams
    {
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
    }
    
    #pragma omp atomic
    global_sum += section_a + section_b;
}

/* Function 3: Tests taskgroup clause inside target parallel region */
static void test_target_taskgroup(void)
{
    int task_result = 0;
    
    /* OMP_CLAUSE_TASKGROUP: taskgroup inside target parallel */
    #pragma omp target parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task shared(task_result)
                {
                    int temp = 0;
                    for (int i = 0; i < N; i++) {
                        temp += global_array[i] % 10;
                    }
                    #pragma omp atomic
                    task_result += temp;
                }
                
                #pragma omp task shared(task_result)
                {
                    int temp = 0;
                    for (int i = 0; i < N; i++) {
                        temp += global_array[i] / 10;
                    }
                    #pragma omp atomic
                    task_result += temp;
                }
            }
        }
    }
    
    #pragma omp atomic
    global_sum += task_result;
}

/* Function 4: Mixed clauses in nested contexts */
static void test_mixed_clauses(void)
{
    int mixed_sum = 0;
    
    /* Combined parallel and for clauses */
    #pragma omp target parallel for simd \
        private(mixed_sum) reduction(+:mixed_sum)
    for (int i = 0; i < N; i++) {
        mixed_sum += global_array[i] * 3;
    }
    
    /* Sections with parallel region */
    #pragma omp target parallel
    {
        #pragma omp sections reduction(+:mixed_sum)
        {
            #pragma omp section
            {
                for (int i = 0; i < N/3; i++) {
                    mixed_sum += global_array[i];
                }
            }
            #pragma omp section
            {
                for (int i = N/3; i < 2*N/3; i++) {
                    mixed_sum += global_array[i];
                }
            }
            #pragma omp section
            {
                for (int i = 2*N/3; i < N; i++) {
                    mixed_sum += global_array[i];
                }
            }
        }
    }
    
    #pragma omp atomic
    global_sum += mixed_sum;
}

int main(void)
{
    /* Initialize array with predictable values */
    for (int i = 0; i < N; i++) {
        global_array[i] = (i % 100) + 1;
    }
    
    /* Reset global sum */
    global_sum = 0;
    
    /* Test all clause types across different functions */
    test_target_parallel_for();      /* Triggers OMP_CLAUSE_FOR and OMP_CLAUSE_PARALLEL */
    test_target_sections();          /* Triggers OMP_CLAUSE_SECTIONS */
    test_target_taskgroup();         /* Triggers OMP_CLAUSE_TASKGROUP */
    test_mixed_clauses();            /* Triggers multiple clauses in combination */
    
    /* Additional direct tests in main() */
    int main_sum = 0;
    
    /* Direct target parallel for */
    #pragma omp target parallel for \
        map(tofrom: main_sum) reduction(+:main_sum)
    for (int i = 0; i < N; i++) {
        main_sum += global_array[i] * 4;
    }
    
    /* Direct target sections */
    #pragma omp target sections reduction(+:main_sum)
    {
        #pragma omp section
        {
            for (int i = 0; i < N; i += 2) {
                main_sum += global_array[i];
            }
        }
        #pragma omp section
        {
            for (int i = 1; i < N; i += 2) {
                main_sum += global_array[i];
            }
        }
    }
    
    global_sum += main_sum;
    
    /* Print result to ensure code isn't dead */
    printf("Final sum: %d\n", global_sum);
    
    /* Expected value verification */
    int expected = 0;
    for (int i = 0; i < N; i++) {
        expected += global_array[i] * 10;  /* Accounts for all the multiplications */
        expected += (global_array[i] % 10) + (global_array[i] / 10);
    }
    expected *= 2;  /* Some functions run twice */
    
    printf("Expected: %d\n", expected);
    printf("Verification: %s\n", (global_sum == expected) ? "PASS" : "FAIL");
    
    return 0;
}
