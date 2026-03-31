/* test_omp_clauses.c
 * 
 * This test program is designed to trigger the uncovered lines in
 * tree-pretty-print.cc (dump_omp_clause function) for the following
 * OpenMP clause types: FOR, PARALLEL, SECTIONS, and TASKGROUP.
 * 
 * Compile with: gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-gimple -c test_omp_clauses.c
 * Additional flags for more coverage: -fdump-tree-omplower -fdump-tree-all
 */

#include <omp.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization and ensure clauses are processed */
static volatile int vol_bound = 100;
static volatile int vol_flag = 1;

/* Prevent inlining to ensure each construct is processed separately */
__attribute__((noinline, cold))
void test_for_clause(void) {
    int i;
    double arr[100];
    
    /* OMP_CLAUSE_FOR: Use in combined construct with 'for' clause */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: arr[0:100]) if(vol_flag) \
                num_teams(2) thread_limit(64)
    for (i = 0; i < vol_bound; i++) {
        arr[i] = sin(i * 0.1) + cos(i * 0.05);
    }
    
    /* Use result to prevent dead code elimination */
    if (arr[0] > 1000) printf("unreachable\n");
}

__attribute__((noinline, cold))
void test_parallel_clause(void) {
    int x = 0;
    
    /* OMP_CLAUSE_PARALLEL: Use 'parallel' clause with target construct */
    #pragma omp target parallel if(vol_flag) map(tofrom: x) \
                num_threads(4) default(shared)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        x += tid + 1;
    }
    
    if (x > 1000) printf("unreachable\n");
}

__attribute__((noinline, cold))
void test_sections_clause(void) {
    int a = 0, b = 0, c = 0;
    
    /* OMP_CLAUSE_SECTIONS: Use 'sections' clause in combined construct */
    #pragma omp target teams distribute parallel for sections \
                map(tofrom: a, b, c) num_teams(2) \
                reduction(+:a, b, c)
    {
        #pragma omp section
        { a = 1; }
        #pragma omp section
        { b = 2; }
        #pragma omp section
        { c = 3; }
    }
    
    if (a + b + c > 1000) printf("unreachable\n");
}

__attribute__((noinline, cold))
void test_taskgroup_clause(void) {
    int sum = 0;
    
    /* OMP_CLAUSE_TASKGROUP: Use 'taskgroup' clause with taskloop */
    #pragma omp taskloop taskgroup reduction(+:sum) \
                num_tasks(10) grainsize(5) \
                if(vol_flag)
    for (int i = 0; i < vol_bound; i++) {
        sum += i;
    }
    
    /* Also test standalone taskgroup with nested task */
    #pragma omp taskgroup
    {
        #pragma omp task shared(sum) if(vol_flag)
        {
            #pragma omp atomic
            sum += 42;
        }
    }
    
    if (sum > 100000) printf("unreachable\n");
}

/* Additional function to trigger potential warnings/diagnostics */
__attribute__((noinline, cold))
void trigger_diagnostic(void) {
    int shared_var = 0;
    
    /* This may trigger data race warnings */
    #pragma omp taskgroup
    {
        #pragma omp task shared(shared_var)
        {
            shared_var++;  /* Potential race condition */
        }
    }
    
    /* Use result */
    if (shared_var > 1000) printf("unreachable\n");
}

int main(void) {
    int total = 0;
    
    /* Initialize random seed for variability */
    srand(42);
    
    /* Call all test functions */
    test_for_clause();
    test_parallel_clause();
    test_sections_clause();
    test_taskgroup_clause();
    
    /* Trigger diagnostic path */
    trigger_diagnostic();
    
    /* Simple computation to ensure execution */
    #pragma omp parallel for reduction(+:total)
    for (int i = 0; i < 100; i++) {
        total += i * (rand() % 5);
    }
    
    printf("Result: %d\n", total);
    return total > 0 ? 0 : 1;
}
