/* test_omp_clauses.c - Coverage for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, 
                         OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP */

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
    volatile int bound = g_volatile_bound / 2;
    
    /* Use target with parallel clause */
    #pragma omp target parallel map(tofrom: local_sum) \
                num_threads(4) if(bound > 50)
    {
        int tid = omp_get_thread_num();
        /* Introduce potential data race for diagnostic generation */
        #pragma omp atomic
        local_sum += tid * 0.5;
        
        /* Additional computation to ensure region isn't empty */
        for (int j = 0; j < 10; j++) {
            local_sum += cos(tid + j);
        }
    }
    
    g_volatile_result += local_sum;
}

/* Function 3: Test OMP_CLAUSE_SECTIONS */
NOINLINE_COLD
void test_sections_clause(void) {
    double section_sum[3] = {0.0, 0.0, 0.0};
    volatile int bound = g_volatile_bound;
    
    /* Use target teams with sections clause */
    #pragma omp target teams distribute parallel for sections \
                map(tofrom: section_sum) num_teams(2)
    {
        #pragma omp section
        {
            for (int i = 0; i < bound/3; i++) {
                section_sum[0] += sqrt(i + 1.0);
            }
        }
        
        #pragma omp section
        {
            for (int i = bound/3; i < 2*bound/3; i++) {
                section_sum[1] += log(i + 1.0);
            }
        }
        
        #pragma omp section
        {
            for (int i = 2*bound/3; i < bound; i++) {
                section_sum[2] += exp(i * 0.001);
            }
        }
    }
    
    for (int k = 0; k < 3; k++) {
        g_volatile_result += section_sum[k];
    }
}

/* Function 4: Test OMP_CLAUSE_TASKGROUP */
NOINLINE_COLD
void test_taskgroup_clause(void) {
    double task_sum = 0.0;
    volatile int bound = g_volatile_bound;
    
    /* Use taskgroup clause with taskloop */
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp taskgroup task_reduction(+:task_sum)
        {
            #pragma omp taskloop grainsize(10) in_reduction(+:task_sum) \
                        nogroup num_tasks(4)
            for (int i = 0; i < bound; i++) {
                /* Use volatile to prevent optimization */
                volatile double val = i * 0.1;
                task_sum += val * val;
            }
        }
    }
    
    g_volatile_result += task_sum;
}

/* Additional function to trigger diagnostics with incorrect usage */
NOINLINE_COLD
void trigger_diagnostic(void) {
    int x = 0;
    volatile int flag = 1;
    
    /* This may trigger warnings about implicit barrier */
    #pragma omp parallel if(flag)
    {
        #pragma omp taskgroup
        {
            #pragma omp task shared(x)
            {
                x = 1;  /* Potential data race if not properly synchronized */
            }
            /* Missing taskwait here might trigger diagnostic */
        }
    }
    
    /* Use x to prevent dead code elimination */
    g_volatile_result += x;
}

int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Call all test functions */
    test_for_clause();
    test_parallel_clause();
    test_sections_clause();
    test_taskgroup_clause();
    trigger_diagnostic();
    
    printf("Result: %f\n", (double)g_volatile_result);
    printf("Test completed.\n");
    
    return 0;
}
