/* test_omp_clauses.c - Coverage for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, 
   OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP pretty-printing */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Use volatile to prevent optimization and ensure clauses are processed */
volatile int g_volatile_bound = 100;
volatile double g_volatile_result = 0.0;

/* Prevent inlining to ensure each function's tree is processed separately */
__attribute__((noinline, cold))
void test_for_clause(void) {
    int i;
    double sum = 0.0;
    int bound = g_volatile_bound;
    
    /* OMP_CLAUSE_FOR: Use in combined construct with explicit 'for' clause */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: sum) num_teams(2) thread_limit(64)
    for (i = 0; i < bound; i++) {
        /* Use math function to prevent dead code elimination */
        sum += sin(i * 0.01);
    }
    
    g_volatile_result += sum;
}

__attribute__((noinline, cold))
void test_parallel_clause(void) {
    double local_sum = 0.0;
    int bound = g_volatile_bound;
    
    /* OMP_CLAUSE_PARALLEL: Use 'parallel' clause with target construct */
    #pragma omp target parallel map(tofrom: local_sum) \
        num_threads(4) if(target: bound > 50)
    {
        int tid = omp_get_thread_num();
        /* Introduce potential data race to trigger diagnostic */
        #pragma omp atomic
        local_sum += tid * 0.5;
    }
    
    g_volatile_result += local_sum;
}

__attribute__((noinline, cold))
void test_sections_clause(void) {
    double section_sum = 0.0;
    int bound = g_volatile_bound;
    
    /* OMP_CLAUSE_SECTIONS: Use 'sections' clause in combined construct */
    #pragma omp target teams distribute parallel for sections \
        map(tofrom: section_sum) num_teams(2)
    for (int i = 0; i < 1; i++) {  /* Dummy loop for 'for' part */
        #pragma omp section
        {
            section_sum += 1.0;
        }
        #pragma omp section
        {
            section_sum += 2.0;
        }
    }
    
    g_volatile_result += section_sum;
}

__attribute__((noinline, cold))
void test_taskgroup_clause(void) {
    double task_sum = 0.0;
    int bound = g_volatile_bound;
    
    /* OMP_CLAUSE_TASKGROUP: Use 'taskgroup' clause with taskloop */
    #pragma omp taskloop taskgroup \
        grainsize(10) num_tasks(5) shared(task_sum)
    for (int i = 0; i < bound; i++) {
        /* Potential data race - may trigger diagnostic */
        #pragma omp atomic
        task_sum += cos(i * 0.01);
    }
    
    /* Also test standalone taskgroup construct */
    #pragma omp taskgroup
    {
        #pragma omp task shared(task_sum)
        {
            #pragma omp atomic
            task_sum += 3.14;
        }
    }
    
    g_volatile_result += task_sum;
}

/* Additional function with nested constructs to ensure deep processing */
__attribute__((noinline, cold))
void test_combined_clauses(void) {
    double combined_sum = 0.0;
    int bound = g_volatile_bound;
    
    /* Mix multiple clause types in nested constructs */
    #pragma omp target teams distribute parallel for \
        map(tofrom: combined_sum) num_teams(3)
    for (int i = 0; i < bound; i++) {
        #pragma omp taskgroup
        {
            #pragma omp task shared(combined_sum)
            {
                combined_sum += sin(i * 0.1) * cos(i * 0.1);
            }
        }
    }
    
    g_volatile_result += combined_sum;
}

int main(void) {
    double final_result = 0.0;
    
    printf("Testing OpenMP clause pretty-printing coverage...\n");
    
    /* Call all test functions to ensure all constructs are processed */
    test_for_clause();
    test_parallel_clause();
    test_sections_clause();
    test_taskgroup_clause();
    test_combined_clauses();
    
    /* Use result to prevent dead code elimination */
    final_result = g_volatile_result;
    printf("Final result: %f\n", final_result);
    
    return (final_result > 0.0) ? 0 : 1;
}
