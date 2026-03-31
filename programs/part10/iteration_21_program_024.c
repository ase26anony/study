/* test_omp_clauses.c - Coverage for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, 
                         OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Prevent optimization and ensure each construct is processed separately */
#define NOINLINE_COLD __attribute__((noinline, cold))

/* Global volatile to prevent dead code elimination */
volatile int g_volatile_bound = 100;
volatile int g_trigger = 1;

/* Array to store results, marked volatile to prevent optimization */
volatile int results[4] = {0};

/* Function 1: Test OMP_CLAUSE_FOR */
NOINLINE_COLD
void test_for_clause(void) {
    int i;
    int local_sum = 0;
    
    /* Use target teams distribute parallel for simd with explicit for clause */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: local_sum) if(g_trigger) \
        num_teams(2) thread_limit(64) \
        reduction(+:local_sum)
    for (i = 0; i < g_volatile_bound; i++) {
        /* Use math function to prevent optimization */
        local_sum += (int)sin(i * 0.01) + 1;
    }
    
    results[0] = local_sum;
}

/* Function 2: Test OMP_CLAUSE_PARALLEL */
NOINLINE_COLD
void test_parallel_clause(void) {
    int local_val = 0;
    
    /* Use target with parallel clause - will generate OMP_CLAUSE_PARALLEL */
    #pragma omp target parallel map(tofrom: local_val) \
        if(g_trigger) num_threads(4) \
        default(shared) private(local_val)
    {
        int tid = omp_get_thread_num();
        /* Potential data race - may trigger diagnostic */
        local_val += tid + (int)cos(tid * 0.1);
    }
    
    results[1] = local_val;
}

/* Function 3: Test OMP_CLAUSE_SECTIONS */
NOINLINE_COLD
void test_sections_clause(void) {
    int section_a = 0, section_b = 0;
    
    /* Combined construct with sections clause */
    #pragma omp target teams distribute parallel for sections \
        map(tofrom: section_a, section_b) \
        num_teams(2) \
        reduction(+:section_a, section_b)
    {
        #pragma omp section
        {
            for (int i = 0; i < g_volatile_bound/2; i++) {
                section_a += (int)tan(i * 0.001);
            }
        }
        
        #pragma omp section
        {
            for (int i = 0; i < g_volatile_bound/2; i++) {
                section_b += (int)sin(i * 0.001) * 2;
            }
        }
    }
    
    results[2] = section_a + section_b;
}

/* Function 4: Test OMP_CLAUSE_TASKGROUP */
NOINLINE_COLD
void test_taskgroup_clause(void) {
    int task_sum = 0;
    
    /* Use taskloop with taskgroup clause */
    #pragma omp taskloop taskgroup \
        reduction(+:task_sum) \
        grainsize(10) \
        num_tasks(20) \
        if(g_trigger)
    for (int i = 0; i < g_volatile_bound; i++) {
        /* Introduce potential data sharing issue for diagnostic */
        task_sum += i * (int)cos(i * 0.01);
    }
    
    /* Also test standalone taskgroup construct */
    #pragma omp taskgroup
    {
        #pragma omp task
        {
            task_sum += 42;
        }
        #pragma omp task
        {
            task_sum += 23;
        }
    }
    
    results[3] = task_sum;
}

/* Main function that calls all test functions */
int main(void) {
    int final_result = 0;
    
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Call each test function */
    test_for_clause();
    test_parallel_clause();
    test_sections_clause();
    test_taskgroup_clause();
    
    /* Aggregate results to ensure execution */
    for (int i = 0; i < 4; i++) {
        final_result += results[i];
    }
    
    printf("Final result: %d\n", final_result);
    printf("Test completed. Check compiler dumps for coverage.\n");
    
    return final_result != 0 ? 0 : 1;
}
