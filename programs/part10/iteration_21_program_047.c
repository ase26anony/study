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
#include <math.h>
#include <stdlib.h>

#define ARRAY_SIZE 100

/* Use volatile to prevent optimization and ensure clauses are processed */
static volatile int vol_bound = ARRAY_SIZE;
static volatile int vol_counter = 0;

/* Non-inlined, cold functions to ensure each construct is processed separately */
__attribute__((noinline, cold))
void test_for_clause(void) {
    int arr[ARRAY_SIZE];
    
    /* OMP_CLAUSE_FOR: Use in a combined construct with explicit 'for' clause */
    #pragma omp target teams distribute parallel for simd map(tofrom: arr[0:ARRAY_SIZE])
    for (int i = 0; i < vol_bound; i++) {
        /* Use math function to prevent dead code elimination */
        arr[i] = (int)sin(i * 0.1) * 100;
    }
    
    /* Use result to prevent optimization */
    vol_counter += arr[vol_bound % ARRAY_SIZE];
}

__attribute__((noinline, cold))
void test_parallel_clause(void) {
    int x = 0;
    
    /* OMP_CLAUSE_PARALLEL: Use 'parallel' clause with target construct */
    #pragma omp target parallel map(tofrom: x) if(vol_bound > 50)
    {
        /* Potential data race to trigger diagnostic */
        x += omp_get_thread_num();
    }
    
    vol_counter += x;
}

__attribute__((noinline, cold))
void test_sections_clause(void) {
    int a = 0, b = 0;
    
    /* OMP_CLAUSE_SECTIONS: Use 'sections' clause in a combined construct */
    #pragma omp target teams distribute parallel for sections map(tofrom: a, b) num_teams(2)
    {
        #pragma omp section
        {
            a = rand() % 100;
        }
        #pragma omp section
        {
            b = rand() % 100;
        }
    }
    
    vol_counter += a + b;
}

__attribute__((noinline, cold))
void test_taskgroup_clause(void) {
    int sum = 0;
    
    /* OMP_CLAUSE_TASKGROUP: Use 'taskgroup' clause with taskloop */
    #pragma omp taskloop taskgroup
    for (int i = 0; i < vol_bound; i++) {
        /* Shared variable access - may trigger diagnostic */
        #pragma omp atomic
        sum += i;
    }
    
    /* Also test standalone taskgroup construct */
    #pragma omp taskgroup
    {
        #pragma omp task
        {
            vol_counter += 1;
        }
    }
    
    vol_counter += sum;
}

/* Additional function with error to trigger diagnostic printing */
__attribute__((noinline, cold))
void test_with_warning(void) {
    int problematic = 0;
    
    /* This may trigger a warning about implicit barrier */
    #pragma omp target parallel
    {
        problematic = omp_get_thread_num();
        #pragma omp barrier  /* Explicit barrier - may cause diagnostic */
    }
    
    /* Use builtin that compiler might analyze */
    __builtin_printf("Value: %d\n", problematic);
}

int main(void) {
    int total = 0;
    
    /* Seed random for reproducibility */
    srand(42);
    
    /* Call all test functions */
    test_for_clause();
    test_parallel_clause();
    test_sections_clause();
    test_taskgroup_clause();
    test_with_warning();
    
    /* Compute and print result to ensure execution */
    total = vol_counter;
    printf("Result: %d\n", total);
    
    return 0;
}
