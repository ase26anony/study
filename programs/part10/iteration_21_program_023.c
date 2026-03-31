/* test_omp_clauses.c - Coverage for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
   OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP in tree-pretty-print.cc */

#include <omp.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/* Prevent optimization and ensure each construct is processed separately */
#define NOINLINE_COLD __attribute__((noinline, cold))

/* Global volatile variables to prevent dead code elimination */
volatile int g_volatile_bound = 100;
volatile double g_volatile_result = 0.0;
volatile int g_volatile_counter = 0;

/* Function 1: Test OMP_CLAUSE_FOR */
NOINLINE_COLD
void test_for_clause(void) {
    int i;
    double sum = 0.0;
    int bound = g_volatile_bound;
    
    /* Use target teams distribute parallel for simd with explicit 'for' clause */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: sum) if(target: bound > 50) \
                num_teams(2) thread_limit(64)
    for (i = 0; i < bound; i++) {
        /* Use math function to prevent optimization */
        sum += sin(i * 0.01) * cos(i * 0.01);
    }
    
    g_volatile_result += sum;
    g_volatile_counter++;
}

/* Function 2: Test OMP_CLAUSE_PARALLEL */
NOINLINE_COLD
void test_parallel_clause(void) {
    int i;
    double local_sum = 0.0;
    int bound = g_volatile_bound / 2;
    
    /* Use target with parallel clause */
    #pragma omp target parallel map(tofrom: local_sum) \
                if(parallel: bound > 25) num_threads(4) \
                default(none) shared(bound)
    {
        int tid = omp_get_thread_num();
        /* Introduce potential data race for diagnostic generation */
        local_sum += tid * 0.1;
        
        #pragma omp for reduction(+:local_sum) nowait
        for (i = 0; i < bound; i++) {
            local_sum += sqrt(i + 1) * 0.01;
        }
    }
    
    g_volatile_result += local_sum;
    g_volatile_counter++;
}

/* Function 3: Test OMP_CLAUSE_SECTIONS */
NOINLINE_COLD
void test_sections_clause(void) {
    double section_sum = 0.0;
    int bound = g_volatile_bound;
    
    /* Use target teams with sections clause in combined construct */
    #pragma omp target teams distribute parallel for sections \
                map(tofrom: section_sum) \
                num_teams(2) num_threads(4)
    for (int i = 0; i < 1; i++) {  /* Dummy loop for distribute */
        #pragma omp section
        {
            section_sum += log(1.0 + bound) * 0.5;
        }
        #pragma omp section
        {
            section_sum += exp(0.01 * bound) * 0.3;
        }
    }
    
    g_volatile_result += section_sum;
    g_volatile_counter++;
}

/* Function 4: Test OMP_CLAUSE_TASKGROUP */
NOINLINE_COLD
void test_taskgroup_clause(void) {
    double task_sum = 0.0;
    int bound = g_volatile_bound;
    
    /* Use taskloop with taskgroup clause - may trigger diagnostics */
    #pragma omp taskgroup task_reduction(+:task_sum)
    {
        #pragma omp taskloop grainsize(10) nogroup \
                    in_reduction(+:task_sum) \
                    shared(bound) private(bound)  /* Intentional error for diagnostic */
        for (int i = 0; i < bound; i++) {
            task_sum += (i % 10) * 0.01;
        }
    }
    
    /* Also test standalone taskgroup construct */
    #pragma omp taskgroup
    {
        #pragma omp task shared(task_sum)
        {
            task_sum += 1.0;
        }
        #pragma omp taskwait
    }
    
    g_volatile_result += task_sum;
    g_volatile_counter++;
}

/* Additional function with combined construct for extra coverage */
NOINLINE_COLD
void test_combined_clauses(void) {
    int i;
    double combined_sum = 0.0;
    int bound = g_volatile_bound;
    
    /* Complex combined construct that may generate multiple clauses */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: combined_sum) \
                if(target: bound > 0) if(parallel: omp_in_parallel()) \
                collapse(1) ordered(1)
    for (i = 0; i < bound; i++) {
        combined_sum += (i % 7) * 0.001;
    }
    
    g_volatile_result += combined_sum;
    g_volatile_counter++;
}

int main(void) {
    double final_result = 0.0;
    
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Call all test functions */
    test_for_clause();
    test_parallel_clause();
    test_sections_clause();
    test_taskgroup_clause();
    test_combined_clauses();
    
    /* Ensure computations aren't optimized away */
    final_result = g_volatile_result + g_volatile_counter;
    
    printf("Final result: %f\n", final_result);
    printf("Test completed. Check compiler dumps for coverage.\n");
    
    return 0;
}
