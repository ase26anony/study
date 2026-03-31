/* test_omp_clauses.c - Coverage for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
   OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP in GCC's tree pretty-printer */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Prevent optimization and inlining to ensure constructs are processed */
#define NOINLINE_COLD __attribute__((noinline, cold))

/* Global volatile variables to prevent dead code elimination */
volatile int g_volatile_bound = 100;
volatile double g_volatile_result = 0.0;

/* Function 1: Test OMP_CLAUSE_FOR in combined construct */
NOINLINE_COLD
void test_for_clause(void) {
    int i;
    double sum = 0.0;
    volatile int bound = g_volatile_bound;
    
    /* Combined construct with explicit 'for' clause */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: sum) num_teams(2) thread_limit(64)
    for (i = 0; i < bound; i++) {
        /* Use math function to prevent optimization */
        sum += sin(i * 0.01);
    }
    
    g_volatile_result += sum;
}

/* Function 2: Test OMP_CLAUSE_PARALLEL in target construct */
NOINLINE_COLD
void test_parallel_clause(void) {
    double local_sum = 0.0;
    volatile int iter = g_volatile_bound / 2;
    
    /* Target construct with 'parallel' clause */
    #pragma omp target parallel map(tofrom: local_sum) \
        if(target: iter > 50) device(0)
    {
        int tid = omp_get_thread_num();
        /* Introduce potential data race for diagnostic generation */
        #pragma omp atomic
        local_sum += tid * 0.5;
    }
    
    g_volatile_result += local_sum;
}

/* Function 3: Test OMP_CLAUSE_SECTIONS in combined construct */
NOINLINE_COLD
void test_sections_clause(void) {
    double section_sum[3] = {0.0, 0.0, 0.0};
    volatile int use_sections = 1;
    
    /* Combined construct with 'sections' clause */
    #pragma omp target teams distribute parallel for sections \
        map(tofrom: section_sum) num_teams(1)
    {
        #pragma omp section
        {
            for (int j = 0; j < 10; j++) {
                section_sum[0] += cos(j * 0.1);
            }
        }
        
        #pragma omp section
        {
            for (int j = 0; j < 10; j++) {
                section_sum[1] += sin(j * 0.2);
            }
        }
        
        #pragma omp section
        {
            for (int j = 0; j < 10; j++) {
                section_sum[2] += tan(j * 0.05);
            }
        }
    }
    
    for (int k = 0; k < 3; k++) {
        g_volatile_result += section_sum[k];
    }
}

/* Function 4: Test OMP_CLAUSE_TASKGROUP in taskloop construct */
NOINLINE_COLD
void test_taskgroup_clause(void) {
    double task_sum = 0.0;
    volatile int task_count = g_volatile_bound;
    
    /* Taskloop with 'taskgroup' clause - may trigger diagnostic */
    #pragma omp taskgroup task_reduction(+:task_sum)
    {
        #pragma omp taskloop taskgroup nogroup \
            grainsize(4) num_tasks(8) reduction(+:task_sum)
        for (int i = 0; i < task_count; i++) {
            /* Use rand() to prevent optimization - data race potential */
            task_sum += (rand() % 100) * 0.01;
        }
    }
    
    g_volatile_result += task_sum;
}

/* Function 5: Additional test with nested taskgroup for extra coverage */
NOINLINE_COLD
void test_nested_taskgroup(void) {
    volatile int outer = 5;
    
    #pragma omp taskgroup
    {
        double inner_sum = 0.0;
        
        #pragma omp taskloop taskgroup simd safelen(4) \
            if(taskloop: outer > 0)
        for (int i = 0; i < outer * 10; i++) {
            inner_sum += sqrt(i + 1.0);
        }
        
        g_volatile_result += inner_sum;
    }
}

int main(void) {
    double final_result = 0.0;
    
    /* Initialize random seed */
    srand(42);
    
    printf("Testing OpenMP clause coverage for GCC pretty-printer...\n");
    
    /* Call all test functions to ensure all constructs are processed */
    test_for_clause();
    test_parallel_clause();
    test_sections_clause();
    test_taskgroup_clause();
    test_nested_taskgroup();
    
    /* Use __builtin_printf to potentially trigger tree printing during optimization */
    final_result = g_volatile_result;
    __builtin_printf("Final result: %f\n", final_result);
    
    /* Additional volatile store to prevent optimization */
    volatile double *dummy = (volatile double *)malloc(sizeof(double));
    if (dummy) {
        *dummy = final_result;
        free((void *)dummy);
    }
    
    return (final_result > 0.0) ? 0 : 1;
}
