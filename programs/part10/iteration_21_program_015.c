/* test_omp_clauses.c - Coverage for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
   OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP in tree-pretty-print.cc */

#include <omp.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/* Use volatile to prevent optimization and ensure clauses are processed */
volatile int g_volatile_bound = 100;
volatile int g_trigger = 1;

/* Prevent inlining to ensure each construct is processed separately */
__attribute__((noinline, cold))
void test_for_clause(void) {
    int i;
    double arr[100];
    
    /* OMP_CLAUSE_FOR: for clause in combined construct */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:100]) if(g_trigger > 0)
    for (i = 0; i < g_volatile_bound; i++) {
        arr[i] = sin(i * 0.1) * cos(i * 0.05);
    }
    
    /* Use result to prevent dead code elimination */
    volatile double sum = 0.0;
    for (i = 0; i < 10; i++) {
        sum += arr[i];
    }
}

__attribute__((noinline, cold))
void test_parallel_clause(void) {
    int x = 0;
    
    /* OMP_CLAUSE_PARALLEL: parallel clause in target construct */
    #pragma omp target parallel map(tofrom: x) if(g_trigger > 0) \
        num_threads(2) default(none) shared(g_volatile_bound)
    {
        int tid = omp_get_thread_num();
        /* Introduce potential data race for diagnostic generation */
        #pragma omp atomic
        x += tid + g_volatile_bound;
    }
    
    volatile int check = x;
}

__attribute__((noinline, cold))
void test_sections_clause(void) {
    int a = 0, b = 0, c = 0;
    
    /* OMP_CLAUSE_SECTIONS: sections clause in combined construct */
    #pragma omp target teams distribute parallel for sections \
        map(tofrom: a, b, c) if(g_trigger > 0) \
        num_teams(2) thread_limit(4)
    {
        #pragma omp section
        {
            a = omp_get_team_num() + 1;
        }
        #pragma omp section
        {
            b = omp_get_num_teams() * 2;
        }
        #pragma omp section
        {
            c = omp_get_thread_num() * 3;
        }
    }
    
    volatile int total = a + b + c;
}

__attribute__((noinline, cold))
void test_taskgroup_clause(void) {
    int sum = 0;
    
    /* OMP_CLAUSE_TASKGROUP: taskgroup clause with taskloop */
    #pragma omp taskgroup task_reduction(+:sum) \
        allocate(omp_default_mem_alloc: sum) if(g_trigger > 0)
    {
        #pragma omp taskloop grainsize(10) nogroup \
            in_reduction(+:sum) shared(g_volatile_bound)
        for (int i = 0; i < g_volatile_bound; i++) {
            /* Use math function to prevent optimization */
            sum += (int)(fabs(sin(i * 0.01)) * 100);
        }
    }
    
    /* Nested taskgroup to increase coverage probability */
    #pragma omp taskgroup
    {
        #pragma omp task
        {
            volatile int temp = sum;
        }
    }
    
    volatile int final_sum = sum;
}

/* Additional function with error to trigger diagnostic printing */
__attribute__((noinline, cold))
void test_with_potential_warning(void) {
    int problematic = 0;
    
    /* This may trigger warnings about implicit declaration or similar */
    #pragma omp target parallel if(undeclared_function())  /* Intentional error */
    {
        problematic = 1;
    }
    
    volatile int p = problematic;
}

int main(void) {
    int result = 0;
    
    /* Call all test functions */
    test_for_clause();
    test_parallel_clause();
    test_sections_clause();
    test_taskgroup_clause();
    
    /* Optional: Uncomment to trigger diagnostic paths */
    /* test_with_potential_warning(); */
    
    /* Use __builtin_printf to potentially trigger tree printing during optimization */
    result = g_volatile_bound + g_trigger;
    __builtin_printf("Result: %d\n", result);
    
    return 0;
}
