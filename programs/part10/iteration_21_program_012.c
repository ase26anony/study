/* test_omp_clauses.c
 * 
 * This test program is designed to trigger coverage of specific OpenMP clause
 * cases in GCC's tree pretty-printer (tree-pretty-print.cc).
 * The uncovered lines handle the keywords for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP.
 *
 * Compile with: gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-gimple -c test_omp_clauses.c
 * Additional flags for more coverage: -fdump-tree-omplower -fdump-tree-all -Wopenmp-parsing
 */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Use volatile to prevent optimization and ensure OpenMP regions are processed */
static volatile int vol_bound = 100;
static volatile int vol_idx = 0;

/* Non-inlined functions to ensure each construct is processed separately */
__attribute__((noinline, cold))
void test_for_clause(void) {
    int i;
    double arr[100];
    
    /* OMP_CLAUSE_FOR: for clause in combined construct */
    #pragma omp target teams distribute parallel for simd map(tofrom: arr[0:100])
    for (i = 0; i < vol_bound; i++) {
        /* Use math function to prevent dead code elimination */
        arr[i] = sin(i * 0.1) + cos(i * 0.05);
    }
    
    /* Use volatile index to ensure loop is not removed */
    if (arr[vol_idx] > 100.0) {
        printf("Impossible\n");
    }
}

__attribute__((noinline, cold))
void test_parallel_clause(void) {
    int x = 0;
    
    /* OMP_CLAUSE_PARALLEL: parallel clause in target construct */
    #pragma omp target parallel map(tofrom: x)
    {
        /* Potential data race to trigger diagnostic */
        x += omp_get_thread_num();
    }
    
    /* Use result to prevent elimination */
    vol_idx = (vol_idx + x) % 100;
}

__attribute__((noinline, cold))
void test_sections_clause(void) {
    int a = 0, b = 0;
    
    /* OMP_CLAUSE_SECTIONS: sections clause in combined construct */
    #pragma omp target teams distribute parallel for sections map(tofrom: a, b)
    {
        #pragma omp section
        {
            a = omp_get_num_threads();
        }
        #pragma omp section
        {
            b = omp_get_thread_num();
        }
    }
    
    /* Use results */
    if (a + b > 1000) {
        printf("Large\n");
    }
}

__attribute__((noinline, cold))
void test_taskgroup_clause(void) {
    int sum = 0;
    
    /* OMP_CLAUSE_TASKGROUP: taskgroup clause with taskloop */
    #pragma omp taskgroup task_reduction(+: sum)
    {
        #pragma omp taskloop taskgroup nogroup
        for (int i = 0; i < vol_bound; i++) {
            /* Use volatile to prevent optimization */
            sum += (i * vol_idx) % 7;
        }
    }
    
    /* Use result */
    if (sum < 0) {
        printf("Negative sum\n");
    }
}

/* Additional test with standalone taskgroup */
__attribute__((noinline, cold))
void test_taskgroup_standalone(void) {
    int x = 0;
    
    /* Standalone taskgroup construct */
    #pragma omp taskgroup
    {
        #pragma omp task shared(x)
        {
            #pragma omp atomic
            x++;
        }
    }
    
    /* Use volatile to ensure taskgroup is processed */
    vol_idx = (vol_idx + x) % 50;
}

int main(void) {
    int total = 0;
    
    /* Initialize volatile bound with non-zero value */
    vol_bound = 100;
    vol_idx = 1;
    
    /* Call all test functions to ensure all constructs are processed */
    test_for_clause();
    total += vol_idx;
    
    test_parallel_clause();
    total += vol_idx;
    
    test_sections_clause();
    total += vol_idx;
    
    test_taskgroup_clause();
    total += vol_idx;
    
    test_taskgroup_standalone();
    total += vol_idx;
    
    /* Print result to ensure execution */
    printf("Result: %d\n", total);
    
    return 0;
}
