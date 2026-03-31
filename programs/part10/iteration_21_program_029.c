/* test_omp_clauses.c */
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 100

/* Prevent optimization and inlining */
volatile int vol_bound = N;
__attribute__((noinline, cold)) void use_result(int r) {
    printf("%d\n", r); /* Prevent dead code elimination */
}

/* Test OMP_CLAUSE_FOR */
__attribute__((noinline, cold))
int test_for_clause(void) {
    int sum = 0;
    int i;
    
    /* Use target teams distribute parallel for simd with explicit for clause */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: sum) reduction(+:sum) if(vol_bound > 50)
    for (i = 0; i < vol_bound; i++) {
        sum += i * 2;
    }
    
    return sum;
}

/* Test OMP_CLAUSE_PARALLEL */
__attribute__((noinline, cold))
int test_parallel_clause(void) {
    int result = 0;
    
    /* Use target with parallel clause */
    #pragma omp target parallel map(tofrom: result) \
        if(vol_bound > 0) num_threads(2)
    {
        int tid = omp_get_thread_num();
        result = tid * 100;
    }
    
    return result;
}

/* Test OMP_CLAUSE_SECTIONS */
__attribute__((noinline, cold))
int test_sections_clause(void) {
    int a = 0, b = 0;
    
    /* Use target teams with sections clause */
    #pragma omp target teams distribute parallel for sections \
        map(tofrom: a, b) num_teams(2)
    {
        #pragma omp section
        {
            a = vol_bound * 3;
        }
        #pragma omp section
        {
            b = vol_bound * 5;
        }
    }
    
    return a + b;
}

/* Test OMP_CLAUSE_TASKGROUP */
__attribute__((noinline, cold))
int test_taskgroup_clause(void) {
    int total = 0;
    
    /* Use taskloop with taskgroup clause */
    #pragma omp taskloop taskgroup reduction(+:total) \
        grainsize(10) num_tasks(4)
    for (int i = 0; i < vol_bound; i++) {
        total += i;
    }
    
    /* Also test standalone taskgroup */
    #pragma omp taskgroup
    {
        #pragma omp task
        {
            total += 100;
        }
    }
    
    return total;
}

/* Additional function to trigger diagnostics */
__attribute__((noinline, cold))
void trigger_diagnostic(void) {
    int x = 0;
    int y = 0;
    
    /* This may trigger data race warnings */
    #pragma omp parallel shared(x) private(y)
    {
        x++; /* Potential race condition for diagnostic */
        y = omp_get_thread_num();
    }
    
    /* Use builtin that might trigger tree printing during optimization */
    volatile int trigger = 1;
    if (trigger) {
        __builtin_printf("Trigger: %d\n", x + y);
    }
}

int main(void) {
    int total = 0;
    
    /* Call all test functions */
    total += test_for_clause();
    total += test_parallel_clause();
    total += test_sections_clause();
    total += test_taskgroup_clause();
    
    /* Trigger diagnostic path */
    trigger_diagnostic();
    
    /* Use result to prevent optimization */
    use_result(total);
    
    return 0;
}
