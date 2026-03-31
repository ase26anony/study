/* test_omp_clauses.c - Coverage for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
   OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP in tree-pretty-print.cc */

#include <omp.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, cold))
#define VOLATILE volatile

/* Global volatile variable to prevent dead code elimination */
VOLATILE int g_volatile_bound = 100;
VOLATILE double g_volatile_result = 0.0;

/* Function 1: Test OMP_CLAUSE_FOR */
NOINLINE void test_for_clause(void) {
    int i;
    double sum = 0.0;
    VOLATILE int bound = g_volatile_bound;
    
    /* Use for clause in a combined construct */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: sum) num_teams(2) thread_limit(64)
    for (i = 0; i < bound; i++) {
        sum += sin(i * 0.1);  /* Non-trivial computation */
    }
    
    g_volatile_result += sum;
}

/* Function 2: Test OMP_CLAUSE_PARALLEL */
NOINLINE void test_parallel_clause(void) {
    double local_sum = 0.0;
    VOLATILE int bound = g_volatile_bound / 2;
    
    /* Use parallel clause with target construct */
    #pragma omp target parallel map(tofrom: local_sum) \
                num_threads(4) if(bound > 50)
    {
        int tid = omp_get_thread_num();
        local_sum += cos(tid * 0.5);  /* Prevent optimization */
    }
    
    g_volatile_result += local_sum;
}

/* Function 3: Test OMP_CLAUSE_SECTIONS */
NOINLINE void test_sections_clause(void) {
    double section_sum[3] = {0.0, 0.0, 0.0};
    VOLATILE int bound = g_volatile_bound;
    
    /* Use sections clause in a combined construct */
    #pragma omp target teams distribute parallel for sections \
                map(tofrom: section_sum) num_teams(3)
    for (int i = 0; i < 3; i++) {
        #pragma omp section
        {
            for (int j = 0; j < bound; j++) {
                section_sum[0] += sqrt(j + 1);
            }
        }
        #pragma omp section
        {
            for (int j = 0; j < bound; j++) {
                section_sum[1] += log(j + 2);
            }
        }
        #pragma omp section
        {
            for (int j = 0; j < bound; j++) {
                section_sum[2] += exp(j * 0.01);
            }
        }
    }
    
    g_volatile_result += section_sum[0] + section_sum[1] + section_sum[2];
}

/* Function 4: Test OMP_CLAUSE_TASKGROUP */
NOINLINE void test_taskgroup_clause(void) {
    double task_sum = 0.0;
    VOLATILE int bound = g_volatile_bound;
    
    /* Create potential data race to trigger diagnostic */
    VOLATILE int shared_counter = 0;
    
    /* Use taskgroup clause with taskloop */
    #pragma omp taskloop taskgroup shared(shared_counter) \
                grainsize(10) num_tasks(5)
    for (int i = 0; i < bound; i++) {
        #pragma omp task
        {
            /* Potential data race - may trigger diagnostic */
            shared_counter++;
            task_sum += sin(i * 0.2) * cos(i * 0.1);
        }
    }
    
    /* Also use standalone taskgroup */
    #pragma omp taskgroup
    {
        #pragma omp task
        {
            task_sum += 1.0;
        }
        #pragma omp task
        {
            task_sum += 2.0;
        }
    }
    
    g_volatile_result += task_sum + shared_counter;
}

/* Main function that calls all test functions */
int main(void) {
    double final_result = 0.0;
    
    /* Initialize random seed for variability */
    srand(42);
    
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Call all test functions */
    test_for_clause();
    test_parallel_clause();
    test_sections_clause();
    test_taskgroup_clause();
    
    /* Use __builtin_printf to potentially trigger tree printing */
    __builtin_printf("Result: %f\n", g_volatile_result);
    
    /* Force compiler to consider all code */
    if (g_volatile_result > 1000.0) {
        printf("Unexpected large result\n");
    }
    
    return 0;
}
