/* test_omp_clauses.c
 * 
 * This test program is designed to trigger coverage of specific
 * OpenMP clause keywords in GCC's tree pretty-printer.
 * The uncovered lines in tree-pretty-print.cc handle the cases for:
 *   OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, 
 *   OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP
 *
 * Compile with: gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-gimple -c test_omp_clauses.c
 * Additional flags for more coverage: -fdump-tree-omplower -fdump-tree-all -Wopenmp-parsing
 */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ARRAY_SIZE 128

/* Prevent optimization and ensure each construct is processed separately */
__attribute__((noinline, cold))
void test_for_clause(volatile int n) {
    int i;
    float data[ARRAY_SIZE];
    
    /* OMP_CLAUSE_FOR: Use in a combined construct with explicit 'for' clause */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: data[0:ARRAY_SIZE]) if(n > 0)
    for (i = 0; i < ARRAY_SIZE; i++) {
        /* Use math function to prevent dead code elimination */
        data[i] = sinf(i * 0.1f) + cosf(i * 0.05f);
    }
    
    /* Use result to prevent optimization */
    volatile float sum = 0.0f;
    for (i = 0; i < ARRAY_SIZE; i++) {
        sum += data[i];
    }
}

__attribute__((noinline, cold))
void test_parallel_clause(volatile int n) {
    int data = 0;
    
    /* OMP_CLAUSE_PARALLEL: Use 'parallel' clause with target construct */
    #pragma omp target parallel map(tofrom: data) if(n > 0) \
        num_threads(2) default(none) shared(n)
    {
        int tid = omp_get_thread_num();
        /* Potential data race - may trigger diagnostic */
        data += tid + n;
        
        /* Use math function */
        volatile float x = sinf(tid * 0.5f);
    }
    
    /* Use result */
    volatile int result = data;
}

__attribute__((noinline, cold))
void test_sections_clause(volatile int n) {
    int a = 0, b = 0, c = 0;
    
    /* OMP_CLAUSE_SECTIONS: Use 'sections' clause in combined construct */
    #pragma omp target teams distribute parallel for sections \
        map(tofrom: a, b, c) num_teams(2) if(n > 0)
    {
        #pragma omp section
        {
            a = n * 2;
            volatile float x = cosf(a * 0.1f);
        }
        
        #pragma omp section
        {
            b = n * 3;
            volatile float y = sinf(b * 0.1f);
        }
        
        #pragma omp section
        {
            c = n * 4;
            volatile float z = tanf(c * 0.01f);
        }
    }
    
    /* Use results */
    volatile int sum = a + b + c;
}

__attribute__((noinline, cold))
void test_taskgroup_clause(volatile int n) {
    int sum = 0;
    
    /* OMP_CLAUSE_TASKGROUP: Use 'taskgroup' clause with taskloop */
    #pragma omp taskgroup task_reduction(+:sum)
    {
        #pragma omp taskloop taskgroup nogroup \
            grainsize(4) num_tasks(8) if(n > 0)
        for (int i = 0; i < ARRAY_SIZE; i++) {
            /* Potential data race - may trigger diagnostic */
            sum += i * n;
            
            /* Use math function */
            volatile float x = sinf(i * 0.1f);
        }
    }
    
    /* Also test standalone taskgroup construct */
    #pragma omp taskgroup
    {
        #pragma omp task shared(sum)
        {
            sum += 100;
            volatile float y = cosf(sum * 0.01f);
        }
    }
    
    /* Use result */
    volatile int result = sum;
}

int main(void) {
    volatile int n = 10; /* Use volatile to prevent constant propagation */
    int total = 0;
    
    /* Call all test functions to ensure all constructs are processed */
    test_for_clause(n);
    total += 1;
    
    test_parallel_clause(n);
    total += 2;
    
    test_sections_clause(n);
    total += 3;
    
    test_taskgroup_clause(n);
    total += 4;
    
    /* Print result to ensure execution */
    printf("Total marker: %d\n", total);
    
    return 0;
}
