/* test_omp_clauses.c - Coverage for OMP_CLAUSE_FOR, PARALLEL, SECTIONS, TASKGROUP */
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Prevent optimization and ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))
#define VOLATILE volatile

/* Global volatile to prevent dead code elimination */
VOLATILE int g_volatile_bound = 100;
VOLATILE double g_volatile_result = 0.0;

/* Function 1: Test OMP_CLAUSE_FOR */
NOINLINE void test_for_clause(void) {
    int i;
    double sum = 0.0;
    VOLATILE int bound = g_volatile_bound;
    
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
NOINLINE void test_parallel_clause(void) {
    double local_sum = 0.0;
    VOLATILE int iter = g_volatile_bound / 2;
    
    /* Use parallel clause in target construct - will generate OMP_CLAUSE_PARALLEL */
    #pragma omp target parallel map(tofrom: local_sum) \
                num_threads(4) if(iter > 50)
    {
        int tid = omp_get_thread_num();
        /* Complex enough to not be optimized away */
        local_sum += cos(tid * 0.1) * exp(-tid * 0.01);
    }
    
    g_volatile_result += local_sum;
}

/* Function 3: Test OMP_CLAUSE_SECTIONS */
NOINLINE void test_sections_clause(void) {
    double section_sum = 0.0;
    VOLATILE int use_sections = 1;
    
    /* Use sections clause in combined construct - will generate OMP_CLAUSE_SECTIONS */
    #pragma omp target teams distribute parallel for sections \
                map(tofrom: section_sum) reduction(+:section_sum) \
                if(use_sections)
    {
        #pragma omp section
        {
            section_sum += 1.0;
        }
        #pragma omp section
        {
            section_sum += 2.0;
        }
        #pragma omp section
        {
            section_sum += 3.0;
        }
    }
    
    g_volatile_result += section_sum;
}

/* Function 4: Test OMP_CLAUSE_TASKGROUP */
NOINLINE void test_taskgroup_clause(void) {
    double task_sum = 0.0;
    VOLATILE int task_count = g_volatile_bound;
    
    /* Use taskgroup clause - will generate OMP_CLAUSE_TASKGROUP */
    #pragma omp taskgroup task_reduction(+:task_sum)
    {
        #pragma omp taskloop taskgroup nogroup \
                    grainsize(4) num_tasks(8) \
                    shared(task_count)
        for (int i = 0; i < task_count; i++) {
            /* Potential data race to trigger diagnostic */
            task_sum += log(i + 1.0);
        }
    }
    
    g_volatile_result += task_sum;
}

/* Additional function with error to trigger diagnostic printing */
NOINLINE void trigger_diagnostic(void) {
    int x = 0;
    VOLATILE int *ptr = &x;
    
    /* This may trigger a diagnostic about data sharing */
    #pragma omp taskgroup
    {
        #pragma omp task shared(ptr)
        {
            (*ptr)++;  /* Potential race condition */
        }
        #pragma omp taskwait
    }
    
    /* Use builtin that might trigger tree printing during optimization */
    __builtin_printf("Diagnostic trigger: %d\n", x);
}

int main(void) {
    /* Initialize with non-trivial value */
    g_volatile_bound = 1000 + (int)(sin(1.0) * 100);
    
    printf("Testing OpenMP clause coverage...\n");
    
    /* Call all test functions */
    test_for_clause();
    test_parallel_clause();
    test_sections_clause();
    test_taskgroup_clause();
    
    /* Trigger diagnostic path */
    trigger_diagnostic();
    
    printf("Final result: %f\n", g_volatile_result);
    printf("Test completed.\n");
    
    return 0;
}
