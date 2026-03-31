/* test_omp_clauses.c
 * 
 * This test program is designed to trigger coverage of specific OpenMP clause
 * keywords in GCC's tree pretty-printer (tree-pretty-print.cc).
 * The uncovered lines handle the cases for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP.
 *
 * Compile with: gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-gimple -c test_omp_clauses.c
 * Additional flags for more coverage: -fdump-tree-omplower -fdump-tree-all -Wopenmp-parsing
 */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 100

/* Use volatile to prevent optimization and ensure clauses are processed */
volatile int vol_bound = N;

/* External function to prevent dead code elimination */
extern double sin(double x);

/* Each test function is marked noinline and cold to ensure separate processing */
__attribute__((noinline, cold))
void test_for_clause(void) {
    int i;
    double arr[N];
    
    /* OMP_CLAUSE_FOR: Use in a combined construct with explicit 'for' clause */
    #pragma omp target teams distribute parallel for simd map(tofrom: arr[0:N])
    for (i = 0; i < vol_bound; i++) {
        arr[i] = sin(i * 0.1);
    }
    
    /* Use result to prevent elimination */
    volatile double sum = 0.0;
    for (i = 0; i < N; i++) {
        sum += arr[i];
    }
}

__attribute__((noinline, cold))
void test_parallel_clause(void) {
    int x = 0;
    
    /* OMP_CLAUSE_PARALLEL: Use 'parallel' clause with target construct */
    #pragma omp target parallel map(tofrom: x)
    {
        x += omp_get_thread_num() + 1;
    }
    
    /* Introduce potential data race warning */
    int shared_var = 0;
    #pragma omp target parallel map(tofrom: shared_var)
    {
        shared_var++;  /* This may trigger a diagnostic about shared variable */
    }
}

__attribute__((noinline, cold))
void test_sections_clause(void) {
    int a = 0, b = 0;
    
    /* OMP_CLAUSE_SECTIONS: Use in a combined construct with 'sections' clause */
    #pragma omp target teams distribute parallel for sections map(tofrom: a, b)
    {
        #pragma omp section
        {
            a = 1;
        }
        #pragma omp section
        {
            b = 2;
        }
    }
    
    /* Use results */
    volatile int result = a + b;
}

__attribute__((noinline, cold))
void test_taskgroup_clause(void) {
    int sum = 0;
    
    /* OMP_CLAUSE_TASKGROUP: Use taskgroup clause with taskloop */
    #pragma omp taskloop taskgroup
    for (int i = 0; i < vol_bound; i++) {
        #pragma omp task
        {
            sum += i;
        }
    }
    
    /* Also test standalone taskgroup */
    #pragma omp taskgroup
    {
        #pragma omp task
        {
            volatile int x = 42;
        }
    }
    
    /* Use __builtin_printf to potentially trigger tree printing during optimization */
    volatile int print_trigger = sum;
    if (print_trigger > 1000) {
        __builtin_printf("Taskgroup result: %d\n", sum);
    }
}

int main(void) {
    /* Initialize with volatile to prevent constant propagation */
    volatile int seed = 42;
    srand(seed);
    
    /* Call all test functions to ensure all constructs are processed */
    test_for_clause();
    test_parallel_clause();
    test_sections_clause();
    test_taskgroup_clause();
    
    /* Compute and print a result to ensure execution */
    int final_result = 0;
    #pragma omp parallel for reduction(+:final_result)
    for (int i = 0; i < N; i++) {
        final_result += rand() % 10;
    }
    
    printf("Final result: %d\n", final_result);
    return 0;
}
