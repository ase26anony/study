/* test_omp_clauses.c - Coverage for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
   OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP in tree-pretty-print.cc */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Prevent optimization and ensure each construct is processed separately */
#define NOINLINE_COLD __attribute__((noinline, cold))

/* Global volatile to prevent dead code elimination */
volatile int g_volatile_bound = 100;
volatile double g_volatile_result = 0.0;

/* 1. Test OMP_CLAUSE_FOR clause */
NOINLINE_COLD
void test_for_clause(void) {
    int i;
    double sum = 0.0;
    int bound = g_volatile_bound;
    
    /* Use for clause in combined construct - will generate OMP_CLAUSE_FOR */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: sum) num_teams(2) thread_limit(64)
    for (i = 0; i < bound; i++) {
        /* Use math function to prevent optimization */
        sum += sin(i * 0.01);
    }
    
    g_volatile_result += sum;
}

/* 2. Test OMP_CLAUSE_PARALLEL clause */
NOINLINE_COLD
void test_parallel_clause(void) {
    double local_sum = 0.0;
    int bound = g_volatile_bound;
    
    /* Use parallel clause with target - will generate OMP_CLAUSE_PARALLEL */
    #pragma omp target parallel map(tofrom: local_sum) \
                if(target: g_volatile_bound > 50)
    {
        int tid = omp_get_thread_num();
        /* Introduce potential data race for diagnostic generation */
        local_sum += tid * 0.1;
    }
    
    g_volatile_result += local_sum;
}

/* 3. Test OMP_CLAUSE_SECTIONS clause */
NOINLINE_COLD
void test_sections_clause(void) {
    double section_sum = 0.0;
    int bound = g_volatile_bound;
    
    /* Use sections clause in combined construct - will generate OMP_CLAUSE_SECTIONS */
    #pragma omp target teams distribute parallel for sections \
                map(tofrom: section_sum) num_teams(2)
    for (int i = 0; i < 1; i++) {  /* Dummy loop to enable distribute */
        #pragma omp section
        {
            section_sum += 1.0;
        }
        #pragma omp section
        {
            section_sum += 2.0;
        }
    }
    
    g_volatile_result += section_sum;
}

/* 4. Test OMP_CLAUSE_TASKGROUP clause */
NOINLINE_COLD
void test_taskgroup_clause(void) {
    double task_sum = 0.0;
    int bound = g_volatile_bound;
    
    /* Use taskgroup clause - will generate OMP_CLAUSE_TASKGROUP */
    #pragma omp taskgroup task_reduction(+:task_sum)
    {
        #pragma omp task in_reduction(+:task_sum)
        {
            task_sum += 3.0;
        }
        #pragma omp task in_reduction(+:task_sum)
        {
            task_sum += 4.0;
        }
    }
    
    /* Alternative: taskloop with taskgroup clause */
    #pragma omp taskloop taskgroup nogroup grainsize(10)
    for (int i = 0; i < bound; i++) {
        task_sum += i * 0.01;
    }
    
    g_volatile_result += task_sum;
}

/* Helper to ensure constructs aren't optimized away */
NOINLINE_COLD
double compute_volatile(void) {
    volatile double v = g_volatile_result;
    return v + sin(v);
}

int main(void) {
    double final_result = 0.0;
    
    printf("Testing OpenMP clause coverage...\n");
    
    /* Call all test functions to ensure all constructs are processed */
    test_for_clause();
    test_parallel_clause();
    test_sections_clause();
    test_taskgroup_clause();
    
    /* Ensure results are used */
    final_result = compute_volatile();
    
    printf("Final result: %f\n", final_result);
    
    return (final_result > 0.0) ? 0 : 1;
}
