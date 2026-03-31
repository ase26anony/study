/* test_omp_clauses.c
 * 
 * This test program is designed to trigger coverage of specific
 * OpenMP clause keywords in GCC's tree pretty-printer.
 * The uncovered lines in tree-pretty-print.cc handle the cases for:
 *   OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, 
 *   OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP.
 *
 * Compile with: gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-gimple -c test_omp_clauses.c
 * Additional flags for more coverage: -fdump-tree-omplower -fdump-tree-all -Wopenmp-parsing
 */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Use volatile to prevent optimization and ensure clauses are processed */
static volatile int g_volatile_bound = 100;
static volatile int g_volatile_counter = 0;

/* External function call to prevent dead code elimination */
extern double sin(double x);

/* Each test function is marked noinline and cold to ensure separate processing */
__attribute__((noinline, cold))
void test_for_clause(void) {
    int i;
    double arr[100];
    
    /* OMP_CLAUSE_FOR: Use in a combined construct with explicit 'for' clause */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:g_volatile_bound]) \
        num_teams(2) thread_limit(64)
    for (i = 0; i < g_volatile_bound; i++) {
        /* Use volatile index and external function to prevent optimization */
        int idx = g_volatile_counter + i;
        arr[i] = sin(idx * 0.1) + (idx % 10);
    }
    
    /* Use result to prevent removal */
    g_volatile_counter += (int)arr[g_volatile_bound / 2];
}

__attribute__((noinline, cold))
void test_parallel_clause(void) {
    int local_sum = 0;
    
    /* OMP_CLAUSE_PARALLEL: Use 'parallel' clause with target construct */
    #pragma omp target parallel map(tofrom: local_sum) \
        device(0) if(g_volatile_bound > 50)
    {
        int tid = omp_get_thread_num();
        /* Potential data race - may trigger diagnostic */
        local_sum += tid * (g_volatile_counter + 1);
        
        /* External call prevents optimization */
        double val = sin(tid * 0.5);
        if (val > 0.8) {
            local_sum += 1;
        }
    }
    
    g_volatile_counter += local_sum;
}

__attribute__((noinline, cold))
void test_sections_clause(void) {
    int section_results[3] = {0, 0, 0};
    
    /* OMP_CLAUSE_SECTIONS: Use 'sections' clause in combined construct */
    #pragma omp target teams distribute parallel for sections \
        map(tofrom: section_results) \
        num_teams(1) num_threads(4)
    for (int i = 0; i < 1; i++) {  /* Dummy loop to enable 'for' part */
        #pragma omp section
        {
            section_results[0] = omp_get_thread_num() * 10;
            double x = sin(section_results[0] * 0.01);
            if (x < 0) section_results[0] += 1;
        }
        
        #pragma omp section
        {
            section_results[1] = omp_get_thread_num() * 20;
            /* Use volatile variable to prevent optimization */
            section_results[1] += g_volatile_bound;
        }
        
        #pragma omp section
        {
            section_results[2] = omp_get_thread_num() * 30;
            /* Potential uninitialized value warning */
            int temp;
            #pragma omp parallel
            {
                temp = omp_get_num_threads();
            }
            section_results[2] += temp;
        }
    }
    
    for (int i = 0; i < 3; i++) {
        g_volatile_counter += section_results[i];
    }
}

__attribute__((noinline, cold))
void test_taskgroup_clause(void) {
    int sum = 0;
    
    /* OMP_CLAUSE_TASKGROUP: Use as clause with taskloop construct */
    #pragma omp taskloop taskgroup \
        grainsize(10) num_tasks(5) \
        reduction(+:sum)
    for (int i = 0; i < g_volatile_bound; i++) {
        /* Data dependency that might trigger analysis */
        sum += i * (g_volatile_counter + 1);
        
        /* External function call */
        double val = sin(i * 0.3);
        if (val > 0.5) {
            sum += 1;
        }
    }
    
    /* Also test standalone taskgroup construct */
    #pragma omp taskgroup
    {
        #pragma omp task
        {
            /* Potential race condition with g_volatile_counter */
            int local = g_volatile_counter;
            g_volatile_counter = local + sum;
        }
        
        #pragma omp task
        {
            /* Use result to prevent optimization */
            double x = sin(sum * 0.01);
            if (x < -0.5) {
                g_volatile_counter -= 1;
            }
        }
    }
}

int main(void) {
    int final_result = 0;
    
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Call all test functions to ensure all clauses are processed */
    test_for_clause();
    test_parallel_clause();
    test_sections_clause();
    test_taskgroup_clause();
    
    /* Use results to prevent dead code elimination */
    final_result = g_volatile_counter % 1000;
    
    printf("Final result: %d\n", final_result);
    printf("Test completed. Check compiler dumps for pretty-printer output.\n");
    
    return final_result == 0 ? 0 : 1;
}
