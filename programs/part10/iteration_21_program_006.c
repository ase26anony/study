/* test_omp_clauses.c - Coverage for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
   OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP pretty-printing */

#include <omp.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

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
    
    /* Use for clause in combined construct - will generate OMP_CLAUSE_FOR */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: sum) num_teams(2) thread_limit(64)
    for (i = 0; i < bound; i++) {
        /* Use math function to prevent optimization */
        sum += sin(i * 0.01);
    }
    
    g_volatile_result += sum;
}

/* Function 2: Test OMP_CLAUSE_PARALLEL */
NOINLINE_COLD
void test_parallel_clause(void) {
    double local_sum = 0.0;
    volatile int iter = g_volatile_bound / 2;
    
    /* Use parallel clause in target construct - will generate OMP_CLAUSE_PARALLEL */
    #pragma omp target parallel map(tofrom: local_sum) \
                if(target: iter > 50) device(0)
    {
        int tid = omp_get_thread_num();
        /* Introduce potential data race for diagnostic generation */
        local_sum += tid * 0.5;
    }
    
    g_volatile_result += local_sum;
}

/* Function 3: Test OMP_CLAUSE_SECTIONS */
NOINLINE_COLD
void test_sections_clause(void) {
    double section_sum[3] = {0.0, 0.0, 0.0};
    volatile int use_sections = 1;
    
    /* Use sections clause in combined construct - will generate OMP_CLAUSE_SECTIONS */
    #pragma omp target teams distribute parallel for sections \
                map(tofrom: section_sum) num_teams(1)
    {
        #pragma omp section
        {
            section_sum[0] = cos(0.5);
        }
        #pragma omp section
        {
            section_sum[1] = sin(1.0);
        }
        #pragma omp section
        {
            section_sum[2] = tan(0.25);
        }
    }
    
    for (int i = 0; i < 3; i++) {
        g_volatile_result += section_sum[i];
    }
}

/* Function 4: Test OMP_CLAUSE_TASKGROUP */
NOINLINE_COLD
void test_taskgroup_clause(void) {
    double task_sum = 0.0;
    volatile int task_count = 10;
    
    /* Use taskgroup clause - will generate OMP_CLAUSE_TASKGROUP */
    #pragma omp taskgroup task_reduction(+:task_sum)
    {
        #pragma omp taskloop grainsize(1) nogroup \
                    shared(task_sum) private(task_count)
        for (int i = 0; i < task_count; i++) {
            /* Use rand() to prevent optimization - introduces side effect */
            task_sum += rand() % 100 * 0.01;
        }
    }
    
    g_volatile_result += task_sum;
}

/* Additional function with error to trigger diagnostic pretty-printing */
NOINLINE_COLD
void trigger_diagnostic(void) {
    int x = 0;
    volatile int *ptr = &x;
    
    /* This may trigger a warning about shared variable in taskgroup */
    #pragma omp taskgroup
    {
        #pragma omp task shared(ptr)
        {
            (*ptr)++;
        }
    }
    
    /* Use all four clause types in one complex construct for maximum coverage */
    #pragma omp target teams distribute parallel for simd \
                parallel sections taskgroup \
                map(tofrom: x) num_teams(2)
    {
        x++;
    }
}

int main(void) {
    double final_result = 0.0;
    
    /* Seed random for reproducibility */
    srand(42);
    
    /* Call all test functions */
    test_for_clause();
    test_parallel_clause();
    test_sections_clause();
    test_taskgroup_clause();
    
    /* Trigger diagnostic path */
    trigger_diagnostic();
    
    /* Ensure computation isn't optimized away */
    final_result = g_volatile_result;
    
    /* Print result to ensure execution */
    printf("Result: %f\n", final_result);
    
    return 0;
}
