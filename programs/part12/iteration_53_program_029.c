/* test_openmp_clauses.c
 * Designed to trigger OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, 
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP pretty-printing
 * in tree-pretty-print.cc lines 1434-1445
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
    
    /* This should generate OMP_CLAUSE_FOR and OMP_CLAUSE_PARALLEL */
    #pragma omp target teams distribute parallel for \
        map(tofrom: local_sum) map(to: global_array) \
        reduction(+:local_sum) private(i)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function to test target parallel clause */
static void test_target_parallel(void) {
    int i;
    int local_sum = 0;
    
    /* This should generate OMP_CLAUSE_PARALLEL */
    #pragma omp target parallel map(tofrom: local_sum) \
        map(to: global_array) reduction(+:local_sum) private(i)
    {
        #pragma omp for
        for (i = 0; i < N; i++) {
            local_sum += global_array[i] * 2;
        }
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function to test target sections clause */
static void test_target_sections(void) {
    int sum1 = 0, sum2 = 0;
    int i;
    
    /* This should generate OMP_CLAUSE_SECTIONS */
    #pragma omp target teams map(tofrom: sum1, sum2) \
        map(to: global_array)
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
    
    /* This should generate OMP_CLAUSE_TASKGROUP */
    #pragma omp target parallel map(tofrom: task_result) \
        map(to: global_array)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task shared(task_result)
                {
                    int temp = 0;
                    for (int i = 0; i < 10; i++) {
                        temp += global_array[i];
                    }
                    #pragma omp atomic
                    task_result += temp;
                }
                
                /* Additional task to ensure taskgroup is meaningful */
                #pragma omp task shared(task_result)
                {
                    int temp = 0;
                    for (int i = 10; i < 20; i++) {
                        temp += global_array[i];
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

/* Combined test with multiple clauses in one pragma */
static void test_combined_clauses(void) {
    int i;
    int combined_sum = 0;
    
    /* Combined parallel and for clauses in one pragma */
    #pragma omp target parallel for map(tofrom: combined_sum) \
        map(to: global_array) reduction(+:combined_sum) private(i)
    for (i = 0; i < N; i++) {
        combined_sum += global_array[i] * 3;
    }
    
    #pragma omp atomic
    global_sum += combined_sum;
}

int main(void) {
    int i;
    
    /* Initialize array with predictable values */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    printf("Testing OpenMP clauses for pretty-printer coverage...\n");
    
    /* Test each clause individually */
    test_target_parallel_for();      /* Triggers OMP_CLAUSE_FOR */
    printf("After target parallel for: global_sum = %d\n", global_sum);
    
    test_target_parallel();          /* Triggers OMP_CLAUSE_PARALLEL */
    printf("After target parallel: global_sum = %d\n", global_sum);
    
    test_target_sections();          /* Triggers OMP_CLAUSE_SECTIONS */
    printf("After target sections: global_sum = %d\n", global_sum);
    
    test_taskgroup();                /* Triggers OMP_CLAUSE_TASKGROUP */
    printf("After taskgroup: global_sum = %d\n", global_sum);
    
    test_combined_clauses();         /* Triggers both PARALLEL and FOR */
    printf("After combined clauses: global_sum = %d\n", global_sum);
    
    /* Expected value verification */
    int expected = 0;
    for (i = 0; i < N; i++) {
        expected += global_array[i];                     /* test_target_parallel_for */
        expected += global_array[i] * 2;                 /* test_target_parallel */
        expected += global_array[i];                     /* test_target_sections */
        if (i < 20) expected += global_array[i];         /* test_taskgroup */
        expected += global_array[i] * 3;                 /* test_combined_clauses */
    }
    
    printf("Expected sum: %d, Actual sum: %d\n", expected, global_sum);
    printf("Test %s\n", (global_sum == expected) ? "PASSED" : "FAILED");
    
    return (global_sum == expected) ? 0 : 1;
}
