/* test_omp_clauses.c - Coverage for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, 
                         OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Prevent optimization and ensure each construct is processed separately */
#define NOINLINE_COLD __attribute__((noinline, cold))

/* Global volatile variables to prevent dead code elimination */
volatile int g_volatile_bound = 100;
volatile double g_volatile_result = 0.0;

/* Function 1: Test OMP_CLAUSE_FOR */
NOINLINE_COLD
void test_for_clause(void) {
    int i;
    double sum = 0.0;
    volatile int bound = g_volatile_bound;
    
    /* Use target teams distribute parallel for simd with explicit for clause */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: sum) if(target: bound > 50) \
                num_teams(2) thread_limit(64) \
                reduction(+:sum) \
                safelen(8) simdlen(4) \
                collapse(1) ordered(1) \
                lastprivate(i) linear(i:1) \
                aligned(sum:64) \
                private(i) shared(bound) \
                default(none) \
                schedule(static, 16) \
                /* The 'for' clause is implicit in 'parallel for simd' */
    for (i = 0; i < bound; i++) {
        /* Use math function to prevent optimization */
        sum += sin(i * 0.01) * cos(i * 0.01);
    }
    
    g_volatile_result += sum;
    __builtin_printf("For clause test: %f\n", sum);
}

/* Function 2: Test OMP_CLAUSE_PARALLEL */
NOINLINE_COLD
void test_parallel_clause(void) {
    volatile int bound = g_volatile_bound / 2;
    double local_sum = 0.0;
    
    /* Use target with parallel clause - will generate OMP_CLAUSE_PARALLEL */
    #pragma omp target parallel \
                map(tofrom: local_sum) \
                if(parallel: bound > 25) \
                num_threads(4) \
                default(shared) private(bound) \
                firstprivate(bound) \
                reduction(+:local_sum) \
                proc_bind(spread) \
                /* The 'parallel' clause is explicit here */
    {
        int tid = omp_get_thread_num();
        /* Introduce potential data race for diagnostic generation */
        #pragma omp atomic
        local_sum += tid * 0.5 + sin(tid);
    }
    
    g_volatile_result += local_sum;
    __builtin_printf("Parallel clause test: %f\n", local_sum);
}

/* Function 3: Test OMP_CLAUSE_SECTIONS */
NOINLINE_COLD
void test_sections_clause(void) {
    volatile int bound = g_volatile_bound;
    double section_sum = 0.0;
    
    /* Use target teams with sections clause */
    #pragma omp target teams distribute parallel for sections \
                map(tofrom: section_sum) \
                num_teams(2) \
                reduction(+:section_sum) \
                /* The 'sections' clause is explicit here */
    {
        #pragma omp section
        {
            for (int i = 0; i < bound/2; i++) {
                section_sum += sqrt(i + 1.0);
            }
        }
        
        #pragma omp section
        {
            for (int i = bound/2; i < bound; i++) {
                section_sum += log(i + 1.0);
            }
        }
    }
    
    g_volatile_result += section_sum;
    __builtin_printf("Sections clause test: %f\n", section_sum);
}

/* Function 4: Test OMP_CLAUSE_TASKGROUP */
NOINLINE_COLD
void test_taskgroup_clause(void) {
    volatile int bound = g_volatile_bound;
    double task_sum = 0.0;
    
    /* Use taskloop with taskgroup clause */
    #pragma omp taskloop taskgroup \
                grainsize(4) num_tasks(8) \
                nogroup \
                if(taskloop: bound > 10) \
                shared(bound, task_sum) \
                untied mergeable \
                /* The 'taskgroup' clause is explicit here */
    for (int i = 0; i < bound; i++) {
        #pragma omp atomic
        task_sum += (i % 3 == 0) ? 1.0 : -1.0;
    }
    
    /* Also test standalone taskgroup construct */
    #pragma omp taskgroup
    {
        #pragma omp task
        {
            task_sum *= 1.1;
        }
        #pragma omp task
        {
            task_sum += 5.0;
        }
    }
    
    g_volatile_result += task_sum;
    __builtin_printf("Taskgroup clause test: %f\n", task_sum);
}

/* Additional test with combined construct containing multiple clauses */
NOINLINE_COLD
void test_combined_construct(void) {
    volatile int bound = g_volatile_bound;
    double combined_sum = 0.0;
    
    /* Complex construct that may trigger multiple clause printings */
    #pragma omp target teams distribute parallel for simd \
                sections taskgroup \
                map(tofrom: combined_sum) \
                if(target: bound > 0) if(parallel: bound > 10) \
                num_teams(4) thread_limit(32) \
                reduction(+:combined_sum) \
                safelen(16) simdlen(8) \
                collapse(2) \
                lastprivate(bound) \
                /* Contains for, sections, and taskgroup clauses */
    for (int i = 0; i < bound; i++) {
        for (int j = 0; j < 5; j++) {
            combined_sum += (i * j) * 0.01;
        }
    }
    
    g_volatile_result += combined_sum;
}

int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Call all test functions to ensure all constructs are processed */
    test_for_clause();
    test_parallel_clause();
    test_sections_clause();
    test_taskgroup_clause();
    test_combined_construct();
    
    /* Use the result to prevent optimization */
    printf("Final accumulated result: %f\n", g_volatile_result);
    
    /* Force compiler to consider all paths by using result in condition */
    if (g_volatile_result > 1000.0) {
        printf("Unexpected large result\n");
    }
    
    return 0;
}
