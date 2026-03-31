/* test_omp_clauses.c
 * This test program is designed to trigger coverage of specific OpenMP clause
 * pretty-printing code in GCC's tree-pretty-print.cc.
 * Compile with: gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-gimple -c test_omp_clauses.c
 * Additional flags: -fdump-tree-omplower -Wopenmp-parsing
 */

#include <omp.h>
#include <math.h>
#include <stdlib.h>

/* Prevent optimization and ensure each construct is processed separately */
#define NOINLINE_COLD __attribute__((noinline, cold))

/* Global volatile variable to prevent dead code elimination */
volatile int g_volatile_bound = 100;
volatile int g_volatile_result = 0;

/* Function 1: Test OMP_CLAUSE_FOR
 * Use in a combined construct that explicitly includes a 'for' clause */
NOINLINE_COLD
void test_for_clause(void) {
    int i;
    double arr[100];
    
    /* Initialize array with volatile bound */
    for (i = 0; i < g_volatile_bound; i++) {
        arr[i] = sin(i * 0.1);
    }
    
    /* Combined construct with explicit 'for' clause */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:g_volatile_bound]) \
        num_teams(2) thread_limit(64)
    for (i = 0; i < g_volatile_bound; i++) {
        arr[i] += cos(arr[i]) * 0.5;
    }
    
    /* Use result to prevent optimization */
    g_volatile_result += (int)arr[g_volatile_bound/2];
}

/* Function 2: Test OMP_CLAUSE_PARALLEL
 * Use 'parallel' clause in a target construct */
NOINLINE_COLD
void test_parallel_clause(void) {
    int i;
    double sum = 0.0;
    volatile int local_bound = g_volatile_bound;
    
    /* Target construct with 'parallel' clause */
    #pragma omp target parallel map(tofrom: sum) \
        reduction(+:sum) if(local_bound > 50)
    {
        int tid = omp_get_thread_num();
        sum += tid * 0.01;
        
        /* Additional computation to make region non-trivial */
        for (i = 0; i < 10; i++) {
            sum += sin(tid + i) * 0.001;
        }
    }
    
    g_volatile_result += (int)(sum * 1000);
}

/* Function 3: Test OMP_CLAUSE_SECTIONS
 * Use 'sections' clause in a combined construct */
NOINLINE_COLD
void test_sections_clause(void) {
    int a = 0, b = 0, c = 0;
    volatile int trigger = g_volatile_bound;
    
    /* Combined construct with 'sections' clause */
    #pragma omp target teams distribute parallel for sections \
        map(tofrom: a, b, c) \
        num_teams(1) \
        shared(trigger)
    {
        #pragma omp section
        {
            a = trigger * 2;
            /* Potential data race hint for diagnostic */
            trigger = trigger + 1;  /* Shared variable modification */
        }
        #pragma omp section
        {
            b = trigger / 2;
        }
        #pragma omp section
        {
            c = trigger % 7;
        }
    }
    
    g_volatile_result += a + b + c;
}

/* Function 4: Test OMP_CLAUSE_TASKGROUP
 * Use 'taskgroup' clause with taskloop */
NOINLINE_COLD
void test_taskgroup_clause(void) {
    int i;
    long total = 0;
    volatile int bound = g_volatile_bound;
    
    /* Taskloop with taskgroup clause */
    #pragma omp taskloop taskgroup \
        reduction(+:total) \
        grainsize(10) \
        num_tasks(4) \
        shared(bound)
    for (i = 0; i < bound; i++) {
        /* Use rand() to prevent optimization - note: not thread-safe,
           but acceptable for coverage testing */
        total += rand() % 100;
    }
    
    /* Explicit taskgroup construct */
    #pragma omp taskgroup
    {
        #pragma omp task shared(total, bound)
        {
            total += bound * 3;
        }
        #pragma omp task shared(total)
        {
            total -= 42;
        }
    }
    
    g_volatile_result += (int)(total % 1000);
}

/* Main function that calls all test functions */
int main(void) {
    int final_result = 0;
    
    /* Initialize random seed for reproducibility */
    srand(42);
    
    /* Call each test function */
    test_for_clause();
    test_parallel_clause();
    test_sections_clause();
    test_taskgroup_clause();
    
    /* Aggregate results to ensure execution */
    final_result = g_volatile_result;
    
    /* Print result to prevent optimization and verify execution */
    printf("Final result: %d\n", final_result);
    
    return final_result % 256;
}
