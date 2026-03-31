/* test_openmp_coverage.c
 * Generates OpenMP constructs with for, parallel, sections, and taskgroup clauses
 * to trigger pretty-printing of OMP_CLAUSE_* nodes in tree-pretty-print.cc
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function to test target parallel for clause */
static void test_target_parallel_for(void)
{
    int i;
    int local_sum = 0;
    
    /* This generates OMP_CLAUSE_FOR and OMP_CLAUSE_PARALLEL in combination */
    #pragma omp target teams distribute parallel for \
                map(tofrom: local_sum) private(i) reduction(+:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function to test target parallel clause */
static void test_target_parallel(void)
{
    int i;
    int temp = 0;
    
    /* This generates OMP_CLAUSE_PARALLEL */
    #pragma omp target parallel private(i) shared(temp) \
                map(tofrom: temp)
    {
        #pragma omp for reduction(+:temp)
        for (i = 0; i < N/2; i++) {
            temp += global_array[i];
        }
        
        /* Nested taskgroup inside parallel region - generates OMP_CLAUSE_TASKGROUP */
        #pragma omp taskgroup
        {
            #pragma omp task
            {
                int j;
                for (j = 0; j < 10; j++) {
                    /* Dummy task work */
                    temp += j;
                }
            }
        }
    }
    
    #pragma omp atomic
    global_sum += temp;
}

/* Function to test target sections clause */
static void test_target_sections(void)
{
    int section1_sum = 0, section2_sum = 0;
    
    /* This generates OMP_CLAUSE_SECTIONS */
    #pragma omp target teams map(tofrom: section1_sum, section2_sum)
    {
        #pragma omp sections private(i)
        {
            #pragma omp section
            {
                int i;
                for (i = 0; i < N/2; i++) {
                    section1_sum += global_array[i];
                }
            }
            
            #pragma omp section
            {
                int i;
                for (i = N/2; i < N; i++) {
                    section2_sum += global_array[i];
                }
            }
        }
    }
    
    #pragma omp atomic
    global_sum += section1_sum;
    #pragma omp atomic
    global_sum += section2_sum;
}

/* Function with combined clauses */
static void test_combined_clauses(void)
{
    int i;
    int combined_sum = 0;
    
    /* Combined parallel for - generates both OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR */
    #pragma omp target parallel for \
                private(i) reduction(+:combined_sum) \
                map(tofrom: combined_sum)
    for (i = 0; i < N; i++) {
        combined_sum += global_array[i] * 2;
    }
    
    #pragma omp atomic
    global_sum += combined_sum;
}

/* Main function with various OpenMP constructs */
int main(void)
{
    int i;
    
    /* Initialize array */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    printf("Starting OpenMP coverage test...\n");
    
    /* Test 1: Target parallel for */
    test_target_parallel_for();
    printf("After target parallel for: global_sum = %d\n", global_sum);
    
    /* Test 2: Target parallel with nested taskgroup */
    test_target_parallel();
    printf("After target parallel with taskgroup: global_sum = %d\n", global_sum);
    
    /* Test 3: Target sections */
    test_target_sections();
    printf("After target sections: global_sum = %d\n", global_sum);
    
    /* Test 4: Combined clauses */
    test_combined_clauses();
    printf("After combined clauses: global_sum = %d\n", global_sum);
    
    /* Additional direct constructs in main */
    {
        int main_local = 0;
        
        /* Direct target parallel for in main */
        #pragma omp target parallel for \
                    private(i) reduction(+:main_local) \
                    map(tofrom: main_local)
        for (i = 0; i < N; i++) {
            main_local += global_array[i] / 2;
        }
        
        global_sum += main_local;
        
        /* Direct taskgroup in main */
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp taskgroup
                {
                    #pragma omp task
                    {
                        int dummy = 0;
                        for (i = 0; i < 100; i++) {
                            dummy += i;
                        }
                        global_sum += dummy;
                    }
                }
            }
        }
    }
    
    printf("Final result: %d\n", global_sum);
    printf("Expected result with N=%d: approximately %d\n", 
           N, (N/2)*(99) + (N/2)*(99) + N*198 + N*99/2 + 4950);
    
    return 0;
}
