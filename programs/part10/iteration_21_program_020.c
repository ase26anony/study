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
volatile double g_volatile_result = 0.0;

/* Function 1: Test OMP_CLAUSE_FOR */
NOINLINE_COLD
void test_for_clause(void) {
    int i;
    double sum = 0.0;
    volatile int bound = g_volatile_bound;
    
    /* Use target teams distribute parallel for simd with explicit 'for' clause */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: sum) if(target: bound > 50) \
        num_teams(2) thread_limit(64) reduction(+:sum)
    for (i = 0; i < bound; i++) {
        /* Use math function to prevent optimization */
        sum += sin(i * 0.01);
    }
    
    g_volatile_result += sum;
    __builtin_printf("For clause test completed: %f\n", sum);
}

/* Function 2: Test OMP_CLAUSE_PARALLEL */
NOINLINE_COLD
void test_parallel_clause(void) {
    double local_sum = 0.0;
    volatile int iter = g_volatile_bound / 2;
    
    /* Use target parallel with explicit 'parallel' clause */
    #pragma omp target parallel map(tofrom: local_sum) \
        if(parallel: iter > 10) num_threads(4) \
        default(shared) private(iter)
    {
        int tid = omp_get_thread_num();
        /* Introduce potential data race for diagnostic generation */
        local_sum += tid * 0.5;
        
        /* Use volatile to prevent optimization */
        volatile int temp = tid;
        if (temp < 0) __builtin_unreachable();
    }
    
    g_volatile_result += local_sum;
    __builtin_printf("Parallel clause test completed: %f\n", local_sum);
}

/* Function 3: Test OMP_CLAUSE_SECTIONS */
NOINLINE_COLD
void test_sections_clause(void) {
    double section_sum[3] = {0.0, 0.0, 0.0};
    volatile int sections_bound = 3;
    
    /* Use target teams with sections clause */
    #pragma omp target teams distribute parallel for sections \
        map(tofrom: section_sum) num_teams(2) \
        reduction(+:section_sum)
    for (int s = 0; s < sections_bound; s++) {
        #pragma omp section
        {
            section_sum[0] += cos(0.1);
        }
        #pragma omp section
        {
            section_sum[1] += sin(0.2);
        }
        #pragma omp section
        {
            section_sum[2] += tan(0.3);
        }
    }
    
    double total = section_sum[0] + section_sum[1] + section_sum[2];
    g_volatile_result += total;
    __builtin_printf("Sections clause test completed: %f\n", total);
}

/* Function 4: Test OMP_CLAUSE_TASKGROUP */
NOINLINE_COLD
void test_taskgroup_clause(void) {
    double task_sum = 0.0;
    volatile int task_count = g_volatile_bound;
    
    /* Use taskloop with taskgroup clause */
    #pragma omp taskloop taskgroup reduction(+:task_sum) \
        grainsize(4) num_tasks(8) nogroup
    for (int i = 0; i < task_count; i++) {
        /* Introduce potential diagnostic: shared variable in taskgroup */
        #pragma omp task shared(task_sum) firstprivate(i)
        {
            double val = sqrt(i + 1.0);
            #pragma omp atomic
            task_sum += val;
        }
    }
    
    /* Also test standalone taskgroup construct */
    #pragma omp taskgroup
    {
        #pragma omp task
        {
            volatile int dummy = 1;
            task_sum += dummy * 0.1;
        }
    }
    
    g_volatile_result += task_sum;
    __builtin_printf("Taskgroup clause test completed: %f\n", task_sum);
}

/* Main function that calls all tests */
int main(void) {
    double final_result = 0.0;
    
    printf("Starting OpenMP clause coverage tests...\n");
    
    /* Call each test function */
    test_for_clause();
    test_parallel_clause();
    test_sections_clause();
    test_taskgroup_clause();
    
    /* Use the result to prevent optimization */
    final_result = g_volatile_result;
    
    /* Force compiler to consider the value */
    if (final_result < -1000000.0) {
        __builtin_unreachable();
    }
    
    printf("Final result: %f\n", final_result);
    printf("Compilation with -fdump-tree-* should trigger pretty-printer for clauses.\n");
    
    return (final_result > 0.0) ? 0 : 1;
}
